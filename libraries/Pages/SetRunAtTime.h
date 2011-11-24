#include "inlines.h"
#include "SetGenericAction.h"
#include <DateTime.h>

extern DateTime lastDateTime;

//  Time menu
class SetRunAtTimeText : public GenericText
{
public:
    SetRunAtTimeText() {}
    bool set_; // was set
    unsigned char hours;
    unsigned char minutes;
    unsigned char state;
    void enter()
    {
        hours = prefs->startHour;
        minutes = prefs->startMinute;
        set_ = false;
        state = 0;
    }
    void exit()
    {
        if (set_)
        {
            set_ = false;
            prefs->startHour = hours;
            prefs->startMinute = minutes;
            prefs.save();
        }
    }
    void paint(char *buf)
    {
        strcpy_P(buf, ps_RunTimePage);
        fmtHrsMins((unsigned long)b2d(hours) * 3600 + (unsigned long)b2d(minutes) * 60, &buf[10]);
        if (!colonBlink)
        {
            char *ptr = buf + 10 + (state & 1) * 3;
            ptr[0] = ptr[1] = ' ';
        }
    }
    void action(unsigned char btn, Menu *m)
    {
        switch (state)
        {
            case 0:
                action0(btn, m);
                break;
            case 1:
                action1(btn, m);
                break;
            default:
                BAD_STATE();
                break;
        }
    }
    void action0(unsigned char btn, Menu *m)
    {
        switch (btn)
        {
            case 0:
                m->gotoPage(m->curPage->parent);
                break;
            case 1:
                decrement(hours, 0x24);
                break;
            case 2:
                increment(hours, 0x24);
                break;
            case 3:
                state = 1;
                break;
            default:
                BAD_BUTTON();
                break;
        }
    }
    void action1(unsigned char btn, Menu *m)
    {
        switch (btn)
        {
            case 0:
                state = 0;
                break;
            case 1:
                decrement(minutes, 0x59);
                break;
            case 2:
                increment(minutes, 0x59);
                break;
            case 3:
                set_ = true;
                m->gotoPage(m->curPage->parent);
                break;
            default:
                BAD_BUTTON();
                break;
        }
    }
};

