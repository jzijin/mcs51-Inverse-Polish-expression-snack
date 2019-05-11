#include <reg51.h>
#include <ABSACC.h>

#define out_seg XBYTE[0x8004]
#define out_bit XBYTE[0x8002]
#define IN XBYTE[0x8001] // the key input

#define COMMIT 0x13
#define QUIT 0x16
#define RESTART 0x14
#define STOP 0xff
#define RESET 0xff

#define CLOCK 1
#define STOP_WATCH 0

unsigned char mode; // is CLOCK mode or is STOP_WATCH mode
unsigned char code ZM[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};
unsigned char output[6];
unsigned char tmp[6];
unsigned char status; // global variable to achieve the stop/start function
unsigned char flag; // 判断是否进入秒表模式

// 用来记录表达式数组
unsigned char equation[20];
// 表达式数组当前的长度
unsigned char equation_length = 0;
// 记录转化后缀表达式的数组
unsigned char Polan_equation[20];
// 后缀表达式的长度
unsigned char Polan_equation_length = 0;
// 用数组模拟栈
unsigned int stack[10];
// 栈的长度 相当于栈顶指针
unsigned char stack_length = 0;

#define ADD 0x0a
#define SUB 0x0b
#define MUL 0x0c
#define DIV 0x0d

#define LEFT_BRA 0x0f

#define RIGHT_BRA 0x0e

// 入栈操作 存储数据 指针加1
void push(unsigned int value)
{
    stack[stack_length] = value;
    stack_length++;
}

// 获得栈顶数据
unsigned int top()
{
    return stack[stack_length - 1];
}

// 出栈操作 直接把长度减一即可
void pop()
{
    stack_length--;
}

// delay some time
void delay(unsigned int time)
{
    unsigned char i;
    while (time--)
        for (i = 0; i < 100; ++i)
            ;// 空操作
}

// the hex num to normal display number
void h2d(unsigned int x, unsigned char *output)
{
    unsigned char i = 0;
    for (; i <= 6; ++i)
    {
        output[i] = ZM[x % 10];
        x /= 10;
    }
}

// display the num
void Display()
{
    unsigned char i;
    unsigned int Pos = 0x01;

    for (i = 0; i < 6; ++i)
    {
        out_bit = 0x00;
        out_seg = output[i];
        out_bit = Pos;
        delay(0.5);
        Pos <<= 1;
    }

    out_bit = 0x00;
}

code unsigned char KeyTable[] = {
    0x16, 0x15, 0x14, 0xff,
    0x13, 0x12, 0x11, 0x10,
    0x0d, 0x0c, 0x0b, 0x0a,
    0x0e, 0x03, 0x06, 0x09,
    0x0f, 0x02, 0x05, 0x08,
    0x00, 0x01, 0x04, 0x07};

// start the key event
unsigned char TestKey()
{
    // test the key
    out_bit = 0;
    return (~IN & 0x0f); // the higher four bit is not used
}

unsigned char GetKey()
{
    // the function is to get key
    unsigned char col;
    unsigned char row;
    unsigned char Pos = 0x20; // the left LED equal the first col
    col = 6;
    do
    {
        out_bit = ~Pos;
        Pos >>= 1;
        row = ~IN & 0x0f;
    } while ((--col != 0) && (row == 0));

    // caculate the index of key table
    if (row != 0)
    {
        col *= 4;
        if (row & 2)
            col += 1;
        else if (row & 4)
            col += 2;
        else if (row & 8)
            col += 3;

        out_bit = 0;

        do
        {
            delay(10);
        } while (TestKey()); // wait the key to release

        return (KeyTable[col]);
    }
    else
        return (0xff);
}

void init() // reset and init
{
    output[0] = ZM[0x00];
    output[1] = ZM[0x00];
    output[2] = ZM[0x00];
    output[3] = ZM[0x00];
    output[4] = ZM[0x00];
    output[5] = ZM[0x00];
}

// 十六进制转十进制
void tooct()
{
    unsigned int num;
    unsigned char i = 0;
    // caculate
    num = tmp[0] + tmp[1] * 16 + tmp[2] * 16 * 16 + tmp[3] * 16 * 16 * 16;

    for (; i <= 6; ++i)
    {
        output[i] = ZM[num % 10];
        tmp[i] = num % 10;
        num /= 10;
    }
}

unsigned int Tick1, Tick2;
unsigned int T100us;

unsigned char Hour = 0, Minute = 0, Second = 0;
unsigned char watch_Hour = 0, watch_Minute = 0, watch_Second = 0;
unsigned int C100us1, C100us2;

void T0Int() interrupt 1
{
    // clock always run
    // every time it call the function add one is not correct
    C100us1--;
    if (C100us1 == 0)
    {
        C100us1 = Tick1;
        Second++;
        if (Second == 60)
        {
            Second = 0;
            Minute++;
            if (Minute == 60)
            {
                Minute = 0;
                Hour++;
                if (Hour == 24)
                    Hour = 0;
            }
        }
    }
    if (flag)
    {
        C100us2--;
        if (C100us2 == 0)
        {
            C100us2 = Tick2;
            watch_Second++;
            if (watch_Second == 60)
            {
                watch_Second = 0;
                watch_Minute++;
                if (watch_Minute == 60)
                {
                    watch_Minute = 0;
                    watch_Hour++;
                    if (watch_Hour == 24)
                        watch_Hour = 0;
                }
            }
        }
    }
}

void time_init()
{
    TR0 = 1;
    Tick1 = 10000;
    C100us1 = 10000;
    Tick2 = 10000 / 60;
    C100us2 = 10000 / 60;
    TMOD = 0x02;
    TH0 = (256 - 100);
    TL0 = (256 - 100);
    IE = 0x82;
}

// 用于模式4 输入表达式 怎么移位
void polan_shift(unsigned char key)
{

    unsigned char i;
    equation[equation_length++] = key;

    if ((key >= 0x00 && key <= 0x09))
    {
        for (i = 5; i > 0; --i)
        {
            output[i] = output[i - 1];
        }
        output[0] = ZM[key];
        Display();
    }
}

// 普通移位 tmp 存储当前数据
void shift(unsigned char key)
{

    unsigned char i;

    for (i = 5; i > 0; --i)
    {
        tmp[i] = tmp[i - 1];
        output[i] = output[i - 1];
    }
    tmp[0] = key;
    output[0] = ZM[key];
    Display();
}

unsigned char test_and_getkey()
{
    if (TestKey())
    {
        return GetKey();
    }
    return 0x00; // not COMMIT any value is ok;
}

// mode 1 16 进制转十进制
unsigned char key_event()
{
    unsigned char key;
    unsigned char return_value = 0;
    init();
    while (1)
    {
        Display();
        if (TestKey())
        {
            key = GetKey();

            if (key == RESET) // reset
            {
                init();
                //continue;
            }

            if (key <= 0x0f)
            {
                shift(key);
            }

            // press the key exec to caculate the value from hex to oct
            if (key == COMMIT)
            {
                tooct();
            }
            if (key == QUIT)
            {
                init();
                return_value = 1;
                break;
            }
        }
    }
    return return_value; // 随便返回一个数都行，正常是走不到这行的，写这行只是为了避免警告。
}

void caculate_time()
{
    if (mode == CLOCK)
    {
        output[5] = ZM[Hour / 10];
        output[4] = ZM[Hour % 10] | 0x80;
        output[3] = ZM[Minute / 10];
        output[2] = ZM[Minute % 10] | 0x80;
        output[1] = ZM[Second / 10];
        output[0] = ZM[Second % 10];
    }
    else if (mode == STOP_WATCH)
    {
        //unsigned char watch_Hour, watch_Minute, watch_Second;
        if (Tick2 > 10000 / 120)
        {
            output[4] = ZM[watch_Hour % 10];
            output[2] = ZM[watch_Minute % 10];
        }
        else
        {
            output[4] = ZM[watch_Hour % 10] | 0x80;
            output[2] = ZM[watch_Minute % 10] | 0x80;
        }
        output[5] = ZM[watch_Hour / 10];

        output[3] = ZM[watch_Minute / 10];

        output[1] = ZM[watch_Second / 10];
        output[0] = ZM[watch_Second % 10];
    }
}

unsigned char clock()
{
    unsigned char key;
    unsigned char return_value = 0;
    TR0 = 1;

    if (status)
    {
        time_init();
        init();
    }
    while (1)
    {

        Display();
        caculate_time();
        key = test_and_getkey();
        if (key == STOP)
        {
            if (status == 1)
                status = 0;
            TR0 = 0;
            break;
        }
        if (key == RESTART)
        {
            status = 1;
            init(); // look cool but useless this step
            Hour = 0;
            Minute = 0;
            Second = 0;
            break;
        }
        if (key == QUIT)
        {
            init();
            return_value = 1;
            status = 0;
            return return_value;
        }
    }
    return return_value;
}

unsigned char stop_watch()
{
    unsigned char key;
    unsigned char return_value = 0;
    TR0 = 1;

    if (flag)
    {
        time_init();
        init();
    }
    while (1)
    {

        Display();
        caculate_time();
        key = test_and_getkey();
        if (key == STOP)
        {
            if (flag == 1)
                flag = 0;
            TR0 = 0;
            break;
        }
        if (key == RESTART)
        {
            flag = 1;
            init(); // look cool but useless this step
            watch_Hour = 0;
            watch_Minute = 0;
            watch_Second = 0;
            break;
        }
        if (key == QUIT)
        {
            init();
            return_value = 1;
            flag = 0;
            return return_value;
        }
    }
    return return_value;
}

// 获得运算符的优先级别 '(' 优先级别最低 
unsigned char get_level(unsigned char c)
{
    if (c == ADD || c == SUB)
        return 1;
    if (c == MUL || c == DIV)
        return 2;
    return 0;
}

// 转化成后缀表达式
void to_Polan()
{
    unsigned char index = 0;
    unsigned char polan_index = 0;
    unsigned char c;
    unsigned int tmp;
    while (index < equation_length)
    {
        if (equation[index] >= 0 && equation[index] <= 9)
        {
            tmp = equation[index];
            Polan_equation[polan_index++] = tmp;
        }
        else if (equation[index] == LEFT_BRA)
            push(LEFT_BRA);
        else if (equation[index] == RIGHT_BRA)
        {
            while (1)
            {
                c = top();
                pop();
                if (c == LEFT_BRA)
                    break;
                Polan_equation[polan_index++] = c;
            }
        }
        else if (equation[index] == ADD || equation[index] == SUB || equation[index] == MUL || equation[index] == DIV)
        {
            while (1)
            {
                if (stack_length == 0 || top() == LEFT_BRA || get_level(top()) < get_level(equation[index]))
                {
                    push(equation[index]); 
                    break;
                }
                else
                {
                    c = top();
                    pop();
                    Polan_equation[polan_index++] = c;
                }
            }
        }
        index++;
    }

    while (1)
    {
        if (stack_length == 0)
            break;
        c = top();
        pop();
        Polan_equation[polan_index++] = c;
    }
    // 最后得到后缀表达式的长度
    Polan_equation_length = polan_index;
}

// 根据计算的到的后缀表达式计算
unsigned int Polan_calc()
{
    unsigned int add1;
    unsigned int add2;
    unsigned char index = 0;
    while (index < Polan_equation_length)
    {
        if (Polan_equation[index] >= 0x00 && Polan_equation[index] <= 0x09)
        {
            push(Polan_equation[index]);
        }
        else
        {
            add1 = top();
            pop();
            add2 = top();
            pop();
            switch (Polan_equation[index])
            {
            case ADD:
                push(add1 + add2);
                break;
            case SUB:
                push(add2 - add1);
                break;
            case MUL:
                push(add2 * add1);
                break;
            case DIV:
                push(add2 / add1);
                break;
            }
        }
        index++;
    }
    return top();
}

void display_polan(unsigned int value)
{
    unsigned char i = 0;
    for (; i <= 6; ++i)
    {
        output[i] = ZM[value % 10];
        value /= 10;
    }
    Display();
}

unsigned char calc_()
{
    unsigned char key;
    unsigned char return_value = 0;
    init();
    while (1)
    {
        Display();
        if (TestKey())
        {
            key = GetKey();

            if (key == RESET) // reset
            {
                init();
                equation_length = 0;
                stack_length = 0;
                Polan_equation_length = 0;
            }

            if (key <= 0x0f)
            {
                polan_shift(key);
            }

            if (key == COMMIT)
            {
                to_Polan();
                display_polan(Polan_calc());
            }
            if (key == QUIT)
            {
                init();
                return_value = 1;
                break;
            }
        }
    }
    return return_value;
}

void main()
{
    unsigned char key;
    init();
    status = 1;

    mode = CLOCK;
    while (1)
    {
        Display();
        if (clock())
        {
            break;
        }
    }

    while (1)
    {
        Display();

        if (TestKey())
        {
            key = GetKey();
            if (key < 0x0f)
            {
                shift(key);
            }
            switch (key)
            {
            case 0x01: // mode 1
            {

                while (1)
                {
                    Display();

                    if (test_and_getkey() == COMMIT)
                    {
                        if (key_event())
                        {
                            break;
                        }
                    }
                }
                break;
            }
                //}

            case 0x02: // mode 2
            {

                mode = CLOCK;
                while (1)
                {
                    Display();

                    if (test_and_getkey() == COMMIT) // enter the mode 2;
                    {
                        if (clock())
                        {
                            break;
                        }
                    }
                }
                break;
            }
            case 0x03: // mode 3
            {
                flag = 1;
                mode = STOP_WATCH;
                while (1)
                {
                    Display();

                    if (test_and_getkey() == COMMIT) // mode 3 enter
                    {
                        if (stop_watch())
                        {
                            break;
                        }
                    }
                }
                break;
            }
            case 0x04: 
            {
                while (1)
                {
                    Display();
                    if (test_and_getkey() == COMMIT) // mode 3 enter
                    {
                        if (calc_())
                        {
                            break;
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}
