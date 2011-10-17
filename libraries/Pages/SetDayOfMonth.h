#include "inlines.h"

class SetDayOfMonthText : public Paint
{
public:
    SetDayOfMonthText() {}
    bool set_; // was set
    unsigned char dayOfMonth;
    void enter()
    {
        dayOfMonth = prefs->startMday;
        set_ = false;
    }
    void exit()
    {
        Serial.println("exit");
        if (set_)
        {
            set_ = false;
            prefs->startMday = dayOfMonth;
            prefs.save();
        }
    }
    void paint(char *buf)
    {
        strcpy_P(buf, ps_DayOfMonthPage);
        if (!colonBlink)
        {
            buf[13] = 0;
        }
        else if (dayOfMonth == 0)
        {
            strncpy(&buf[13], "--", 2);
        }
        else
        {
            unsigned char d = dayOfMonth & 0xf;
            buf[14] = d + '0';
            d = dayOfMonth >> 4;
            if (d != 0)
            {
                buf[13] = d + '0';
            }
            else
            {
                buf[13] = ' ';
            }
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
                decrement(dayOfMonth, 0x31);
                break;
            case 2:
                increment(dayOfMonth, 0x31);
                break;
            default:
                BAD_BUTTON();
                break;
        }
    }
};
extern SetDayOfMonthText sdomText;

class SetDayOfMonthAction : public Action
{
public:
    void paint(char *buf)
    {
        strcpy_P(buf, ps_AdjustControls);
    }
    void action(unsigned char btn, Menu *m)
    {
        sdomText.action(btn, m);
    }
    void enter()
    {
        sdomText.enter();
    }
    void exit()
    {
        sdomText.exit();
    }
};
