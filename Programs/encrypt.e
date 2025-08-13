@File Encryption, Written for PCBox
@Uses a XOR Algorithm, With a Pre-defined Key
@Constant Strings
CONST C50 "Enter input Filename:"
CONST C51 "Enter output Filename:"
CONST C52 "Enter encryption key:"
CONST C53 "Error: file doesn't exist"
CONST C54 "Error: file already exists"
@-----------------------------------------
REGSET S:150 8 @Size of a Byte
REGSET S:151 255
@-----------------------------------------
:Get_File_Input_Again
OUTPUT C50
RUNPROC Get_Input_To_A0_An @Get filename
FEX X A
NOT X
CRUN X OUTPUT C53
CRUN X OUTPUT S:100
CRUN X GOTO Get_File_Input_Again
FOPEN A @Open said file
OUTPUT C52
RUNPROC Get_Input_To_A0_An @Don't touch Y from here on, (Holds the key's length)
REGSET MAR 100 @Memory Starting Point
REGSET U 0 @Iterator
REGSET V 0 @Logic stuff
REGSET X 0 @Operational Stuff
REGSET S 0
:Next_Int
REGSET MDR 0
RNC U X
REGCOPY X S:1
LO U Y Z
XOR X A:Z X
BITOR MDR X MDR
EQ S:1 S:100 V
CRUN V RUNPROC Search_For_EOF
CRUN S MEMWRITE
CRUN S GOTO Write_File
SHIFTBACK MDR S:150 MDR
INC U
RNC U X
REGCOPY X S:1
LO U Y Z
XOR X A:Z X
BITOR MDR X MDR
EQ S:1 S:100 V
CRUN V RUNPROC Search_For_EOF
CRUN S MEMWRITE
CRUN S GOTO Write_File
SHIFTBACK MDR S:150 MDR
INC U
RNC U X
REGCOPY X S:1
LO U Y Z
XOR X A:Z X
BITOR MDR X MDR
EQ S:1 S:100 V
CRUN V RUNPROC Search_For_EOF
CRUN S MEMWRITE
CRUN S GOTO Write_File
SHIFTBACK MDR S:150 MDR
INC U
RNC U X
REGCOPY X S:1
LO U Y Z
XOR X A:Z X
BITOR MDR X MDR
EQ S:1 S:100 V
CRUN V RUNPROC Search_For_EOF
CRUN S MEMWRITE
CRUN S GOTO Write_File
MEMWRITE
INC MAR
GOTO Next_Int
:Write_File
:Get_File_Output_Again
OUTPUT C51
RUNPROC Get_Input_To_A0_An @Get filename
FEX X A
CRUN X OUTPUT C54
CRUN X OUTPUT S:100
CRUN X GOTO Get_File_Output_Again
MKF A
FOPEN A
@--------------------------------
REGCOPY U S:1 @Backup file length
REGSET U 0
REGSET MAR 100
:Write_Next_Char
DEBUG
MEMLOAD
BITAND MDR S:151 X
WNC U X
INC U
SHIFTFORWARD MDR S:150 MDR
BITAND MDR S:151 X
EQ X S:103 V
CRUN V RUNPROC Quit_Encrypt
WNC U X
INC U
SHIFTFORWARD MDR S:150 MDR
BITAND MDR S:151 X
EQ X S:103 V
CRUN V RUNPROC Quit_Encrypt
WNC U X
INC U
SHIFTFORWARD MDR S:150 MDR
BITAND MDR S:151 X
EQ X S:103 V
CRUN V RUNPROC Quit_Encrypt
WNC U X
INC U
SHIFTFORWARD MDR S:150 MDR
BITAND MDR S:151 X
EQ X S:103 V
CRUN V RUNPROC Quit_Encrypt
LOWER U S:1 V
INC MAR
CRUN V GOTO Write_Next_Char
RUNPROC Quit_Encrypt
@-----------------------------------------
@Procedures:
PROC Quit_Encrypt:
INC S:1
WNC S:1 S:100
INC S:1
WNC S:1 S:105
INC S:1
WNC S:1 S:106
INC S:1
WNC S:1 S:107
INC S:1
WNC S:1 S:100
REGSET MAR 10
MEMLOAD
CLEAR
REGCOPY MDR PC
ENDPROC
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
PROC Search_For_EOF:
@Uses U, V, Y
REGSET S 0
REGSET A:255 104
REGSET A:256 5
:Search_For_EOF_Repeat
INC U
INC S
RNC U V
ADD S A:255 S
EQ V S:S V
SUB S A:255 S
CRUN V GOTO Search_For_EOF_Repeat
REGCOPY Z A:255
EQ S A:256 S
SUB U A:255 U @Reset file position
ENDPROC
END