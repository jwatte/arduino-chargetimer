#include "inlines.h"
#include "SetGenericAction.h"
#include <DateTime.h>

extern DateTime lastDateTime;

void twoDigit(char *dst, unsigned char bcd)
{
    dst[0] = '0' + (bcd >> 4);
    dst[1] = '0' + (bcd & 0xf);
}

//  Time menu
class SetDateText : public GenericText
{
public:
    SetDateText() {}
    
    bool set_; // was set
    unsigned char year;
    unsigned char month;
    unsigned char mday;
    unsigned char state;
    
    void enter()
    {
        year = lastDateTime.year;
        month = lastDateTime.month;
        mday = lastDateTime.mday;
        set_ = false;
        state = 0;
    }
    
    void exit()
    {
        if (set_)
        {
            set_ = false;
            lastDateTime.year = year;
            lastDateTime.month = month;
            lastDateTime.mday = mday;
            writeTime(lastDateTime);
        }
    }
    
    void paint(char *buf)
    {
        strcpy_P(buf, ps_DatePage);
        twoDigit(&buf[8], year);
        twoDigit(&buf[11], month);
        twoDigit(&buf[14], mday);
        if (!colonBlink)
        {
            char *ptr = &buf[8] + state * 3;
            ptr[0] = ptr[1] = ' ';
            if (state == 0)
            {
                buf[6] = buf[7] = ' ';
            }
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
            case 2:
                action2(btn, m);
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
                decrement(year, 0x99);
                break;
            case 2:
                increment(year, 0x99);
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
                decrement1(month, 0x12);
                break;
            case 2:
                increment1(month, 0x12);
                break;
            case 3:
                state = 2;
                break;
            default:
                BAD_BUTTON();
                break;
        }
    }
    
    void action2(unsigned char btn, Menu *m)
    {
        switch (btn)
        {
            case 0:
                state = 1;
                break;
            case 1:
                decrement1(mday, bcdDaysInMonth(year, month));
                break;
            case 2:
                increment1(mday, bcdDaysInMonth(year, month));
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

