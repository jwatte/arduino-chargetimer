#include "inlines.h"

class SetDayOfWeekText : public Paint
{
public:
    SetDayOfWeekText() {}
    bool set_; // was set
    unsigned char dayOfWeek;
    void enter()
    {
        dayOfWeek = prefs->startWday;
        set_ = false;
    }
    void exit()
    {
        Serial.println("exit");
        if (set_)
        {
            set_ = false;
            prefs->startWday = dayOfWeek;
            prefs.save();
        }
    }
    void paint(char *buf)
    {
        strcpy_P(buf, ps_DayOfWeekPage);
        if (!colonBlink)
        {
            buf[12] = 0;
        }
        else
        {
            strcpy_P(buf + 12, &ps_DaysOfWeek[4 * dayOfWeek]);
        }
    }
    void action(unsigned char btn, Menu *m)
    {
        switch (btn)
        {
            case 3:
                set_ = true;
            case 0:
                m->gotoPage(m->curPage->parent);
                break;
            case 1:
                decrement(dayOfWeek, 0x07);
                break;
            case 2:
                increment(dayOfWeek, 0x07);
                break;
            default:
                BAD_BUTTON();
                break;
        }
    }
};
extern SetDayOfWeekText sdowText;

class SetDayOfWeekAction : public Action
{
public:
    void paint(char *buf)
    {
        strcpy_P(buf, ps_AdjustControls);
    }
    void action(unsigned char btn, Menu *m)
    {
        sdowText.action(btn, m);
    }
    void enter()
    {
        sdowText.enter();
    }
    void exit()
    {
        sdowText.exit();
    }
};