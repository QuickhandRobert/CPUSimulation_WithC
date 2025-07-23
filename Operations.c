#include "headers.h"
#include <stdio.h>
/******************************************************
* Funcs: Logical & Arithmetic Operations              *
* Param: Integer Pointers to Corresponding Registers  *
*                                                     *
* Return: none                                        *
******************************************************/
void F_OR(int *r1, int *r2, int *r3)
{
    *r3 = *r1 || *r2;
}
void F_AND(int *r1, int *r2, int *r3)
{
    *r3 = *r1 && *r2;
}
void F_NOT(int *r1)
{
    *r1 = !*r1;
}
void F_XOR(int *r1, int *r2, int *r3)
{
    *r3 = *r1 ^ *r2;
}
void F_NAND(int *r1, int *r2, int *r3)
{
    *r3 = !(*r1 && *r2);
}
void F_NOR(int *r1, int *r2, int *r3)
{
    *r3 = !(*r1 || *r2);
}
void F_ADD(int *r1, int *r2, int *r3)
{
    *r3 = *r1 + *r2;
}
void F_SUB(int *r1, int *r2, int *r3)
{
    *r3 = *r1 - *r2;
}
void F_DIV(int *r1, int *r2, int *r3)
{
    *r3 = *r1 / *r2;
}
void F_MUL (int *r1, int *r2, int *r3)
{
    *r3 = *r1 * *r2;
}
void F_LO(int *r1, int *r2, int *r3)
{
    *r3 = *r1 % *r2;
}
void F_INC(int *r1)
{
    (*r1)++;
}
void F_DEC(int *r1)
{
    *r1--;
}
void F_NEG(int *r1)
{
    *r1 *= -1;
}
void F_EQ(int *r1, int *r2, int *r3)
{
    *r3 = *r1 == *r2;
}
