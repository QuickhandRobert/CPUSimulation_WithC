@Music Player for PCBox
@Plays Simple Notes
@Constant Strings
CONST C50 "Enter the Filename to Open:"
CONST C51 "Enter the BPM:"
CONST C52 "Error: file doesn't exist"
CONST C60 "[>           ]"
CONST C61 "[=>          ]"
CONST C62 "[==>         ]"
CONST C63 "[===>        ]"
CONST C64 "[====>       ]"
CONST C65 "[=====>      ]"
CONST C66 "[======>     ]"
CONST C67 "[=======>    ]"
CONST C68 "[========>   ]"
CONST C69 "[=========>  ]"
CONST C70 "[==========> ]"
CONST C71 [===========>]
CONST C72 [============]
CONST C73 [============]
CONST C53 ".0 / "
CONST C54 .0
@--------------------------------------
REGSET S:150 60
REGSET S:151 13
@--------------------------------------
:Get_File_Input_Again
OUTPUT C50
RUNPROC Get_Input_To_A0_An @Get filename
FEX X A
NOT X
CRUN X OUTPUT C52
CRUN X OUTPUT S:100
CRUN X GOTO Get_File_Input_Again
FOPEN A @Open said file
OUTPUT C51
RUNPROC Get_Integer_Input_To_X
SETBPM X
CLS
RUNPROC Get_File_Len
REGCOPY S S:1
REGSET S 0
STARTSTREAM
REGSET U 0 @Iterator
:Next_Note
RNC U X
INC U
RNC U Z
INC U
ADD Z S S
RUNPROC Print_Progress
PLAYNOTE X Z
RUNPROC Search_For_EOF
CRUN Y RUNPROC Quit_MPlayer
GOTO Next_Note
@--------------------------------------
@Procedures:
PROC Get_Integer_Input_To_X:
REGSET X 0 @Output
REGSET Y 0 @Where input is stores and processed
REGSET Z 0 @Logic
INPUT Y
EQ Y S:156 Z
REGCOPY Z A
CRUN Z GOTO Get_Integer_Input_To_X_Repeat
SUB Y S:104 Y
ADD X Y X
:Get_Integer_Input_To_X_Repeat
INPUT Y
EQ Y S:103 U
CRUN U CRUN Z NEG X
CRUN U ENDPROC
SUB Y S:104 Y
MUL X S:100 X
ADD X Y X
GOTO Get_Integer_Input_To_X_Repeat
ENDPROC
PROC Quit_MPlayer:
STOPSTREAM
OUTPUT S:100
REGSET MAR 10
MEMLOAD
CLEAR
REGCOPY MDR PC
ENDPROC
PROC Print_Progress:
CLS
MUL S S:151 Y
DIV Y S:1 Y
ADD Y S:150 Y
OUTPUT C:Y
REGCOPY S Y
RUNPROC Print_Number_A
OUTPUT C53
REGCOPY S:1 Y
DEBUG
RUNPROC Print_Number_A
OUTPUT C54
ENDPROC
PROC Print_Number_A:
REGSET A 1 (@Iterator)
REGSET V 0 @(Used for logic)
@Convert number to string:
:Number_To_String_Continue
LO Y S:100 A:A
DIV Y S:100 Y
INC A
EQ Y S:103 V
NOT V
CRUN V GOTO Number_To_String_Continue
@Print in Reverse:
:Print_In_Reverse_Continue
DEC A
HIGHER A S:103 V
NOT V
CRUN V ENDPROC
DEBUG
ADD A:A S:104 A:A @(Convert to ASCII character)
OUTPUT A:A
GOTO Print_In_Reverse_Continue
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
PROC Get_File_Len:
REGSET U 0 @Iterator
REGSET S 0
INC U
:Next_Char
RNC U X
HIGHER X S:100 Y
CRUN Y ENDPROC
ADD X S S
INC U
INC U
CRUN Y ENDPROC
GOTO Next_Char
ENDPROC
PROC Search_For_EOF:
@Uses U, V, Y
REGSET Y 0
REGSET A:255 104
REGSET A:256 5
:Search_For_EOF_Repeat
INC U
INC Y
RNC U V
ADD Y A:255 Y
EQ V S:Y V
SUB Y A:255 Y
CRUN V GOTO Search_For_EOF_Repeat
REGCOPY Y A:255
EQ Y A:256 Y
SUB U A:255 U @Reset file position
ENDPROC
END