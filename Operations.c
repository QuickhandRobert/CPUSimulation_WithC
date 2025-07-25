#include "headers.h"
#include <stdio.h>
/******************************************************
* Funcs: Logical & Arithmetic Operations              *
* Param: Integer Pointers to Corresponding Registers  *
*                                                     *
* Return: none                                        *
******************************************************/
void F_OR(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 || *r2;
}
void F_AND(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 && *r2;
}
void F_NOT(long long *r1)
{
    *r1 = !*r1;
}
void F_XOR(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 ^ *r2;
}
void F_NAND(long long *r1, long long *r2, long long *r3)
{
    *r3 = !(*r1 && *r2);
}
void F_NOR(long long *r1, long long *r2, long long *r3)
{
    *r3 = !(*r1 || *r2);
}
void F_ADD(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 + *r2;
}
void F_SUB(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 - *r2;
}
void F_DIV(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 / *r2;
}
void F_MUL (long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 * *r2;
}
void F_LO(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 % *r2;
}
void F_INC(long long *r1)
{
    (*r1)++;
}
void F_DEC(long long *r1)
{
    *r1--;
}
void F_NEG(long long *r1)
{
    *r1 *= -1;
}
void F_EQ(long long *r1, long long *r2, long long *r3)
{
    *r3 = *r1 == *r2;
}
