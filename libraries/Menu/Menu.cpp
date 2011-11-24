
#include "Menu.h"

#include <WProgram.h>
#include <LiquidCrystal.h>
#include <HardwareSerial.h>

char menuScratchpad[21];
static char menuOne[21];
static char menuTwo[21];

void deltaUpdate(LiquidCrystal &lcd, char const *src, char *dst, unsigned char x, unsigned char y)
{
    bool move = true;
    bool clear = false;
    bool changed = false;
    for (int i = 0; i < 16; ++i)
    {
        if (src[i] == 0)
        {
            clear = true;
        }
        char ch = clear ? ' ' : src[i];
        if (ch != dst[i])
        {
            changed = true;
            if (move)
            {
                lcd.setCursor(x + i, y);
            }
            move = false;
            lcd.write(ch);
            dst[i] = ch;
        }
        else
        {
            move = true;
        }
    }
    /*
    if (changed)
    {
        Serial.println(src);
    }
    */
}

Page::Page(Page *prt, Paint *p, Action *a) :
    paint(p),
    action(a),
    parent(prt),
    nextSibling(0),
    firstChild(0)
{
    parent->addChild(this);
}

Page::Page(Paint *p, Action *a) :
    paint(p),
    action(a),
    parent(0),
    nextSibling(0),
    firstChild(0)
{
}

void Page::addChild(Page *cld)
{
    Page **pp = &firstChild;
    while (*pp)
    {
        pp = &((*pp)->nextSibling);
    }
    *pp = cld;
}

void Page::enter()
{
    action->enter();
}

void Page::exit()
{
    action->exit();
}



Menu::Menu(LiquidCrystal &l, void (*mx)()) :
    Page(this, this),
    curPage(0),
    lcd(l),
    menuExit(mx),
    bounceMillis(0),
    lastBtns(0xf),
    curBtns(0xf),
    dirty(true)
{
}

void Menu::button(unsigned char btn, unsigned char state)
{
    unsigned char mask = 1 << btn;
    if (state != BTN_INACTIVE)
    {
        curBtns |= mask;
    }
    else
    {
        curBtns &= (0xf ^ mask);
    }
}

void Menu::step()
{
    if (curBtns != lastBtns)
    {
        if ((curBtns & lastBtns) == lastBtns)
        {
            //  button up only
            lastBtns = curBtns;
        }
        else
        {
            unsigned long ml = millis();
            if (ml > bounceMillis + 50)
            {
                //  got past the bounce threshold
                bounceMillis = ml;
                for (unsigned char b = 0; b != 4; ++b)
                {
                    if (!(curBtns & (1 << b)) && (lastBtns & (1 << b)))
                    {
                        curPage->action->action(b, this);
                        dirty = true;
                    }
                }
                lastBtns = curBtns;
            }
        }
    }
    if (curPage == 0 || curPage == this)
    {
        if (menuExit != 0)
        {
            (*this->menuExit)();
        }
        else
        {
            reset();
        }
    }
    if (dirty)
    {
        paint();
    }
}

void Menu::invalidate()
{
    dirty = true;
}

void Menu::paint()
{
    dirty = false;
    curPage->paint->paint(menuScratchpad);
    deltaUpdate(lcd, menuScratchpad, menuOne, 0, 0);
    curPage->action->paint(menuScratchpad);
    deltaUpdate(lcd, menuScratchpad, menuTwo, 0, 1);
}

void Menu::reset()
{
    memset(menuOne, 0, sizeof(menuOne));
    memset(menuTwo, 0, sizeof(menuTwo));
    lcd.clear();
    gotoPage(firstChild);
}

void Menu::paint(char *buf)
{
}

void Menu::action(unsigned char btn, Menu *m)
{
}

void Menu::gotoPage(Page *p)
{
    /*
    Serial.print("gotoPage("); Serial.print((int)p, DEC); Serial.println(")");
    */
    if (curPage)
    {
        curPage->exit();
    }
    if (p)
    {
        p->enter();
    }
    curPage = p;
    dirty = true;
    bounceMillis = millis();
}

void paintProgmem(LiquidCrystal &lcd, prog_char const *str, unsigned char x, unsigned char y)
{
    strncpy_P(menuScratchpad, str, sizeof(menuScratchpad)-1);
    menuScratchpad[sizeof(menuScratchpad)-1] = 0;
    lcd.setCursor(x, y);
    lcd.print(menuScratchpad);
}


PaintProgmemStr::PaintProgmemStr(prog_char const *pm) :
    str(pm)
{
}

void PaintProgmemStr::paint(char *buf)
{
    strncpy_P(buf, str, 20);
    buf[20] = 0;
}


PROGMEM prog_char canPrevNextOk[] = "Cnc Prev Next OK";

void CanPrevNextOkAction::paint(char *buf)
{
    strncpy_P(buf, canPrevNextOk, 20);
    buf[20] = 0;
}


static void upPage(Menu *m)
{
    m->gotoPage(m->curPage->parent);
}

static void prevPage(Menu *m)
{
    Page *pp = m->curPage->parent->firstChild;
    Page *ret = 0;
    while (pp)
    {
        ret = pp;
        if (pp->nextSibling == m->curPage)
        {
            break;
        }
        pp = pp->nextSibling;
    }
    m->gotoPage(ret);
}

static void nextPage(Menu *m)
{
    Page *ret = m->curPage->nextSibling;
    if (!ret)
    {
        ret = m->curPage->parent->firstChild;
    }
    m->gotoPage(ret);
}

static void accept(Menu *m)
{
    if (m->curPage->firstChild)
    {
        m->gotoPage(m->curPage->firstChild);
    }
}

void CanPrevNextOkAction::action(unsigned char btn, Menu *m)
{
    switch (btn)
    {
    case 0:
        upPage(m);
        break;
    case 1:
        prevPage(m);
        break;
    case 2:
        nextPage(m);
        break;
    case 3:
        accept(m);
        break;
    }
}


NavPage::NavPage(Page *parent, PROGMEM prog_char *str) :
    Page(parent, &str_, &act_),
    str_(str)
{
}

void Paint::enter()
{
}

void Paint::exit()
{
}
