#if !defined(Pages_inlines_h)
#define Pages_inlines_h

static void increment(unsigned char &ch, unsigned char top)
{
    if (9 == (ch & 0xf))
    {
        ch = ch + 7;
    }
    else
    {
        ch = ch + 1;
    }
    if (ch > top)
    {
        ch = 0;
    }
}

static void increment1(unsigned char &ch, unsigned char top)
{
    if (9 == (ch & 0xf))
    {
        ch = ch + 7;
    }
    else
    {
        ch = ch + 1;
    }
    if (ch > top)
    {
        ch = 1;
    }
}

static void decrement(unsigned char &ch, unsigned char top)
{
    if (!(ch & 0xf))
    {
        ch = ch - 7;
    }
    else
    {
        ch = ch - 1;
    }
    if (ch > top)
    {
        ch = top;
    }
}

static void decrement1(unsigned char &ch, unsigned char top)
{
    if (!(ch & 0xf))
    {
        ch = ch - 7;
    }
    else
    {
        ch = ch - 1;
    }
    if (ch > top || ch == 0)
    {
        ch = top;
    }
}


#endif
