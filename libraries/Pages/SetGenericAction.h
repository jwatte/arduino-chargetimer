
#if !defined(SetGenericAction_h)
#define SetGenericAction_h

class GenericText : public Paint
{
public:
    virtual void action(unsigned char btn, Menu *m) = 0;
    virtual void enter() = 0;
    virtual void exit() = 0;
};

//  Time adjusting
class SetGenericAction : public Action
{
public:
    SetGenericAction(GenericText *text) :
        text_(text)
    {
    }
    GenericText *text_;
    void paint(char *buf)
    {
        strcpy_P(buf, ps_AdjustControls);
    }
    void action(unsigned char btn, Menu *m)
    {
        text_->action(btn, m);
    }
    void enter()
    {
        text_->enter();
    }
    void exit()
    {
        text_->exit();
    }
};

#endif  //  SetGenericAction_h
