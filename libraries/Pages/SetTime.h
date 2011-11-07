#include "inlines.h"
#include "SetGenericAction.h"
#include <DateTime.h>

extern DateTime lastDateTime;

//  Time menu
class SetTimeText : public GenericText
{
public:
    SetTimeText() {}
    bool set_; // was set
    unsigned char hours;
    unsigned char minutes;
    unsigned char state;
    void enter()
    {
        hours = lastDateTime.hour;
        minutes = lastDateTime.minute;
        set_ = false;
        state = 0;
    }
    void exit()
    {
        if (set_)
        {
            set_ = false;
            lastDateTime.hour = hours;
            lastDateTime.minute = minutes;
            lastDateTime.second = 0;
            writeTime(lastDateTime);
        }
    }
    void paint(char *buf)
    {
        strcpy_P(buf, ps_TimePage);
        fmtHrsMins((unsigned long)b2d(hours) * 3600 + (unsigned long)b2d(minutes) * 60, &buf[9]);
        if (!colonBlink)
        {
            char *ptr = buf + 9 + (state & 1) * 3;
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

