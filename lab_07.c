#include <reg51.h>
#include <stdlib.h>
#include <ABSACC.h>
#define uchar unsigned char
#define uint unsigned int

// 嵌入一个电子琴用来表示游戏结束 蹦蹦蹦 TODO
/*
#define mode 0x82;
xdata unsigned char Control _at_ 0x8003;
xdata unsigned char Speaker _at_ 0x8000;
*/

xdata unsigned char RowLow _at_ 0xb002;
xdata unsigned char RowHigh _at_ 0xb003;
xdata unsigned char ColLow _at_ 0xb000;
xdata unsigned char ColHigh _at_ 0xb001;
xdata unsigned char OUTBIT _at_ 0x8002;
xdata unsigned char OUTSEG _at_ 0x8004;
xdata unsigned char IN _at_ 0x8001;

code unsigned char KeyTable[] = {
    0x16, 0x15, 0x14, 0xff,
    0x13, 0x12, 0x11, 0x10,
    0x0d, 0x0c, 0x0b, 0x0a,
    0x0e, 0x03, 0x06, 0x09,
    0x0f, 0x02, 0x05, 0x08,
    0x00, 0x01, 0x04, 0x07};

//纪念一下曾经我失败的小蛇 // 老子不用链表了 老子用结构体就好了
struct Snack
{
    uchar col_low;
    uchar col_high;
    uchar row_low;
    uchar row_high;
    //struct Snack* next;
};

#define UP 0x08
#define DOWN 0x02
#define LEFT 0x04
#define RIGHT 0x06

uchar direction;    // 定义方向 默认等于LEFT
uchar snack_length; // 记录小蛇的长度
uint Tick;
uint C100us;
struct Snack snacks[20];    // 20 个小蛇
struct Snack Point;         // 随机的食物
uchar Point_flag = 1;       // 是否需要随机生成食物的标志
uint zhongzi = 10000;       // 随机种子
uchar is_end_game_flag = 0; // 是否结束游戏标志

void delay(uchar t)
{
    uchar i, j;

    for (i = t; i > 0; i--)
    {
        for (j = 0; j < 100; j++)
            ;
    }
}
unsigned char TestKey()
{
    OUTBIT = 0;
    return (~IN & 0x0f);
}

unsigned char GetKey()
{
    unsigned char Pos;
    unsigned char i;
    unsigned char k;

    i = 6;
    Pos = 0x20;
    do
    {
        OUTBIT = ~Pos;
        Pos >>= 1;
        k = ~IN & 0x0f;
    } while ((--i != 0) && (k == 0));

    if (k != 0)
    {
        i *= 4;
        if (k & 2)
            i += 1;
        else if (k & 4)
            i += 2;
        else if (k & 8)
            i += 3;

        OUTBIT = 0;
        do
            delay(10);
        while (TestKey());

        return (KeyTable[i]);
    }
    else
        return (0xff);
}

void create_rand_point(uchar zhongzi)
{
    uchar value;
    uchar p;
    srand(zhongzi);
    value = rand() % 2;
    p = rand() % 8;
    if (value == 1)
    {
        Point.col_low = 0x00;
        switch (p)
        {
        case 0:
        {
            Point.col_high = 0x01;
            break;
        }
        case 1:
        {
            Point.col_high = 0x02;
            break;
        }
        case 2:
        {
            Point.col_high = 0x04;
            break;
        }
        case 3:
        {
            Point.col_high = 0x80;
            break;
        }
        case 4:
        {
            Point.col_high = 0x10;
            break;
        }
        case 5:
        {
            Point.col_high = 0x20;
            break;
        }
        case 6:
        {
            Point.col_high = 0x40;
            break;
        }
        case 7:
        {
            Point.col_high = 0x80;
            break;
        }
        default:
            break;
        }
    }
    else
    {
        Point.col_high = 0x00;
        switch (p)
        {
        case 0:
        {
            Point.col_low = 0x01;
            break;
        }
        case 1:
        {
            Point.col_low = 0x02;
            break;
        }
        case 2:
        {
            Point.col_low = 0x04;
            break;
        }
        case 3:
        {
            Point.col_low = 0x80;
            break;
        }
        case 4:
        {
            Point.col_low = 0x10;
            break;
        }
        case 5:
        {
            Point.col_low = 0x20;
            break;
        }
        case 6:
        {
            Point.col_low = 0x40;
            break;
        }
        case 7:
        {
            Point.col_low = 0x80;
            break;
        }
        default:
            break;
        }
    }

    value = rand() % 2;
    p = rand() % 8;
    if (value == 1)
    {
        Point.row_low = 0x00;
        switch (p)
        {
        case 0:
        {
            Point.row_high = 0x01;
            break;
        }
        case 1:
        {
            Point.row_high = 0x02;
            break;
        }
        case 2:
        {
            Point.row_high = 0x04;
            break;
        }
        case 3:
        {
            Point.row_high = 0x80;
            break;
        }
        case 4:
        {
            Point.row_high = 0x10;
            break;
        }
        case 5:
        {
            Point.row_high = 0x20;
            break;
        }
        case 6:
        {
            Point.row_high = 0x40;
            break;
        }
        case 7:
        {
            Point.row_high = 0x80;
            break;
        }
        default:
            break;
        }
    }
    else
    {
        Point.row_high = 0x00;
        switch (p)
        {
        case 0:
        {
            Point.row_low = 0x01;
            break;
        }
        case 1:
        {
            Point.row_low = 0x02;
            break;
        }
        case 2:
        {
            Point.row_low = 0x04;
            break;
        }
        case 3:
        {
            Point.row_low = 0x80;
            break;
        }
        case 4:
        {
            Point.row_low = 0x10;
            break;
        }
        case 5:
        {
            Point.row_low = 0x20;
            break;
        }
        case 6:
        {
            Point.row_low = 0x40;
            break;
        }
        case 7:
        {
            Point.row_low = 0x80;
            break;
        }
        default:
            break;
        }
    }
}

void rand_point(uchar i)
{
    uchar j;
label1:
    create_rand_point(i);
    for (j = 0; j < snack_length; ++j)
    {
        // 四个都一样 点在蛇的身子里面
        if ((snacks[j].col_low == Point.col_low) && (snacks[j].col_high == Point.col_high) && (snacks[j].row_low == Point.row_low) && (snacks[j].row_high == Point.row_high))
            goto label1;
    }
}
void display_snack()
{
    uchar i;
    for (i = 0; i < snack_length; ++i)
    {
        RowLow = 0x00;
        RowHigh = 0x00;
        ColLow = ~snacks[i].col_low;
        ColHigh = ~snacks[i].col_high;
        RowLow = snacks[i].row_low;
        RowHigh = snacks[i].row_high;
        delay(1);
    }
    // 显示食物
    RowLow = 0x00;
    RowHigh = 0x00;
    ColLow = ~Point.col_low;
    ColHigh = ~Point.col_high;
    RowLow = Point.row_low;
    RowHigh = Point.row_high;
    delay(1);
}
uchar the_status()
{
    uchar flag = 0;
    uchar i;
    struct Snack tmp;
    if (direction == LEFT)
    {
        if (snacks[0].col_low == 0x80)
            tmp.col_high = 0x01;
        else
            tmp.col_high = (snacks[0].col_high << 1);
        tmp.col_low = (snacks[0].col_low << 1);

        tmp.row_low = snacks[0].row_low;
        tmp.row_high = snacks[0].row_high;

        //snacks[0] = tmp;
    }
    if (direction == RIGHT)
    {
        // 向右边移动的算法
        if (snacks[0].col_high == 0x01)
            tmp.col_low = 0x80;
        else
            tmp.col_low = (snacks[0].col_low >> 1);
        tmp.col_high = (snacks[0].col_high >> 1);

        tmp.row_low = snacks[0].row_low;
        tmp.row_high = snacks[0].row_high;

        //snacks[0] = tmp;
    }

    // 往上边移动的算法
    if (direction == UP)
    {
        if (snacks[0].row_low == 0x80)
            tmp.row_high = 0x01;
        else
            tmp.row_high = (snacks[0].row_high << 1);
        tmp.row_low = (snacks[0].row_low << 1);

        tmp.col_low = snacks[0].col_low;
        tmp.col_high = snacks[0].col_high;

        //snacks[0] = tmp;
    }

    // 往下边移动的算法  应该是没问题
    if (direction == DOWN)
    {
        if (snacks[0].row_high == 0x01)
            tmp.row_low = 0x80;
        else
            tmp.row_low = (snacks[0].row_low >> 1);
        tmp.row_high = (snacks[0].row_high >> 1);

        tmp.col_low = snacks[0].col_low;
        tmp.col_high = snacks[0].col_high;

        //snacks[0] = tmp;
    }
    if ((tmp.col_low == Point.col_low) && (tmp.col_high == Point.col_high) && (tmp.row_low == Point.row_low) && (tmp.row_high == Point.row_high))
        flag = 1;

    for (i = 0; i < snack_length; ++i)
    {
        if ((tmp.col_low == snacks[i].col_low) && (tmp.col_high == snacks[i].col_high) && (tmp.row_low == snacks[i].row_low) && (tmp.row_high == snacks[i].row_high))
            flag = 2; // 撞到自己了 游戏结束
    }
    // 撞到边界了。
    if ((tmp.col_low == 0x00 && tmp.col_high == 0x00) || (tmp.row_low == 0x00 && tmp.row_high == 0x00))
        flag = 3;

    return flag;
}

void end_game()
{

    while (1)
    {
        ColLow = ~0xff; //??????
        ColHigh = ~0xff;
        RowLow = 0xff; //??????
        RowHigh = 0xff;
        TR0 = 0; // 关闭定时器
                 //goto start;
    }
}

void move()
{
    // 现在写的是不吃到失误的情况
    uchar i;
    struct Snack tmp;
    // 判断一下现在的状态
    uchar status = the_status();

    if (status == 1)
    {
        Point_flag = 1;
        snack_length++;
    }

    else if (status == 2 || status == 3)
    {
        // 撞到自己了 或者撞到墙了 游戏已经结束了
        // call the endgame() signal
        is_end_game_flag = 1;
        end_game();
    }

    //direction = LEFT;
    for (i = snack_length - 1; i >= 1; --i)
    {
        snacks[i] = snacks[i - 1];
    }

    if (direction == LEFT)
    {
        // 先写死了向左边移动  应该是没问题
        if (snacks[0].col_low == 0x80)
            tmp.col_high = 0x01;
        else
            tmp.col_high = (snacks[0].col_high << 1);
        tmp.col_low = (snacks[0].col_low << 1);

        tmp.row_low = snacks[0].row_low;
        tmp.row_high = snacks[0].row_high;

        snacks[0] = tmp;
    }
    if (direction == RIGHT)
    {
        // 向右边移动的算法
        if (snacks[0].col_high == 0x01)
            tmp.col_low = 0x80;
        else
            tmp.col_low = (snacks[0].col_low >> 1);
        tmp.col_high = (snacks[0].col_high >> 1);

        tmp.row_low = snacks[0].row_low;
        tmp.row_high = snacks[0].row_high;

        snacks[0] = tmp;
    }

    // 往上边移动的算法
    if (direction == UP)
    {
        if (snacks[0].row_low == 0x80)
            tmp.row_high = 0x01;
        else
            tmp.row_high = (snacks[0].row_high << 1);
        tmp.row_low = (snacks[0].row_low << 1);

        tmp.col_low = snacks[0].col_low;
        tmp.col_high = snacks[0].col_high;

        snacks[0] = tmp;
    }

    // 往下边移动的算法  应该是没问题
    if (direction == DOWN)
    {
        if (snacks[0].row_high == 0x01)
            tmp.row_low = 0x80;
        else
            tmp.row_low = (snacks[0].row_low >> 1);
        tmp.row_high = (snacks[0].row_high >> 1);

        tmp.col_low = snacks[0].col_low;
        tmp.col_high = snacks[0].col_high;

        snacks[0] = tmp;
    }
}

void T0Int() interrupt 1
{
    //uchar key;
    // 中断函数 每次计算一下新的数组的值， 重新显示
    C100us--;

    if (C100us == 0)
    {
        if (snack_length >= 10 && snack_length <= 15)
            Tick = 2500;
        else if (snack_length > 15 && snack_length <= 20)
            Tick = 500;
        C100us = Tick;
        // 开始处理移动
        move();
        display_snack();
        if (Point_flag) // 如果现在需要产生食物 那么就应该产生
        {
            rand_point(zhongzi--);
            Point_flag = 0;
        }
    }
}
void time_init()
{

    TR0 = 1;
    Tick = 5000;
    C100us = 10000;
    TMOD = 0x02;
    TH0 = (256 - 100);
    TL0 = (256 - 100);
    IE = 0x82;
}
void init()
{
    struct Snack head;
    // 初始化小蛇的长度为4
    snack_length = 4;

    head.col_low = 0x80;
    head.col_high = 0x00;
    head.row_low = 0x00;
    head.row_high = 0x01;
    snacks[0] = head;

    head.col_low = 0x40;
    head.col_high = 0x00;
    head.row_low = 0x00;
    head.row_high = 0x01;
    snacks[1] = head;

    head.col_low = 0x20;
    head.col_high = 0x00;
    head.row_low = 0x00;
    head.row_high = 0x01;
    snacks[2] = head;

    head.col_low = 0x10;
    head.col_high = 0x00;
    head.row_low = 0x00;
    head.row_high = 0x01;
    snacks[3] = head;

    direction = LEFT; // 定义方向 默认等于LEFT
}

void main()
{
    uchar key;
    init();
    time_init();

    while (1)
    {

        //delay(100);
        if (TestKey())
        {
            key = GetKey();
            if (key == UP || key == DOWN || key == LEFT || key == RIGHT)
                direction = key;
        }

        display_snack();
        delay(2);
    }
}
