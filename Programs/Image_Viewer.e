@Simple Image Viewer, Written for PCBox
@Yes I Know This Isn't Optimal, Sorry
@Constans Strings
CONST C50 "Enter the Filename to Open:"
CONST C51 "Error: file doesn't exist"
@-----------------------------------------
@Frequently Used Values
REGSET S:150 3
@-----------------------------------------
:Get_File_Input_Again
OUTPUT C50
RUNPROC Get_Input_To_A0_An @Get filename
FEX X A
NOT X
CRUN X OUTPUT C51
CRUN X OUTPUT S:100
CRUN X GOTO Get_File_Input_Again
FOPEN A @Open said file
CLS
REGSET S:1 0 @R
REGSET S:2 0 @G
REGSET S:3 0 @B
REGSET U 0 @Incrementor
RNC U S @Width
INC U
RNC U V @Height
MUL V S V
INC U
INC U
INC V
:Next_Pixel
RNC U S:1
INC U
RNC U S:2
INC U
RNC U S:3
INC U
DRAWPIXEL S:1 S:2 S:3
DIV U S:150 Y
REGCOPY Y X
DEC X
DEBUG
LO X S X
NOT X
CRUN X OUTPUT S:100
LOWER Y V X
NOT X
CRUN X RUNPROC Quit_Img
GOTO Next_Pixel
@-----------------------------------------
@Procedures
PROC Get_Input_To_A0_An:
@Used Registers:
REGSET X 0 @Input
REGSET U 0 @Used for 0 value
REGSET V 0 @Comparison result
REGSET Y -1 @Itterator
:Get_Input_To_A0_An_Iterate
INC Y
INPUT X
EQ X S:103 U @Is input finished?
EQ X S:130 V @Or space is detected?
OR U V V
CRUN V GOTO Get_Input_To_A0_An_END @End reading stdin then
REGCOPY X A:Y
GOTO Get_Input_To_A0_An_Iterate
:Get_Input_To_A0_An_END
REGCOPY S:103 A:Y
ENDPROC
PROC Quit_Img:
REGSET MAR 10
MEMLOAD
REGCOPY MDR PC
ENDPROC
END
