
#if !defined(MENU_H)
#define MENU_H

#include <avr/pgmspace.h>

class LiquidCrystal;
class Menu;

extern char menuScratchpad[21];

void paintProgmem(LiquidCrystal &lcd, prog_char const *str, unsigned char x, unsigned char y);
void deltaUpdate(LiquidCrystal &lcd, char const *src, char *dst, unsigned char x, unsigned char y);


class Paint
{
public:
    virtual void paint(char *oBuf) = 0;
    virtual void enter();
    virtual void exit();
};

class Action : public Paint
{
public:
    virtual void action(unsigned char btn, Menu *m) = 0;
};

class Adjustable
{
public:
    virtual void adjust(char dir) = 0;
    virtual bool next() = 0;
    virtual void cancel() = 0;
};

class Page
{
public:
    Page(Page *prt, Paint *p, Action *a);
    void addChild(Page *cld);
    void exit();
    void enter();
    
    Paint *paint;
    Action *action;
    Page *nextSibling;
    Page *parent;
    Page *firstChild;

protected:
    Page(Paint *p, Action *a);
};

enum ButtonState
{
    BTN_INACTIVE = 0,
    BTN_ACTIVE = 1
};


class Menu : public Page, public Action
{
public:
    Menu(LiquidCrystal &lcd, void (*mx)() = 0);
    void button(unsigned char btn, unsigned char state);
    void step();
    void invalidate();

    void reset();
    void paint();
    void gotoPage(Page *p);
    
    Page *curPage;
    LiquidCrystal &lcd;
    void (*menuExit)();
    
    unsigned long bounceMillis;
    unsigned char lastBtns;
    unsigned char curBtns;
    bool dirty;

    virtual void paint(char *oBuf);
    virtual void action(unsigned char btn, Menu *m);
};


class PaintProgmemStr : public Paint
{
public:
    PaintProgmemStr(prog_char const *str);
    virtual void paint(char *oBuf);
    
    prog_char const *str;
};

class CanPrevNextOkAction : public Action
{
public:
    void paint(char *oBuf);
    void action(unsigned char btn, Menu *m);
};

class NavPage : public Page
{
public:
    NavPage(Page *parent, PROGMEM prog_char *str);
    PaintProgmemStr str_;
    CanPrevNextOkAction act_;
};


#endif  //  MENU_H
