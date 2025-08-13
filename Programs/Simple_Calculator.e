@Simple Calculator (4 Main Operations) for PCBox
@----------------------------------
@Constant Strings:
CONST C50 "Enter the First Number(Enter q to Exit): "
CONST C51 "Enter the Second Number(Enter q to Exit): "
CONST C52 "Enter the Desired Operation(Enter q to Exit): "
CONST C51 "The Answer is: "
@----------------------------------
@Operations
REGSET S:150 +
REGSET S:151 -
REGSET S:152 /
REGSET S:153 *
@----------------------------------
:Next_Calculation
CLS
OUTPUT C50
RUNPROC Get_Integer_Input_To_X
REGCOPY X MDR
REGSET MDR 5
MEMWRITE
OUTPUT C51
RUNPROC Get_Integer_Input_To_X
REGCOPY X MDR
REGSET MDR 6
MEMWRITE
OUTPUT C52
INPUT X
EQ X S:150 Y
@----------------------------------
@Procedures
PROC Get_Integer_Input_To_X:
REGSET X 0 @Output
REGSET Y 0 @Where input is stores and processed
REGSET U 0 @Logic
:Get_Integer_Input_To_X_Repeat
MUL X S:100 X
INPUT Y
EQ Y S:103 U
CRUN U ENDPROC
SUB Y S:104 Y
ADD X Y X
GOTO Get_Integer_Input_To_X_Repeat
ENDPROC
PROC Quit_Calc:
REGSET MAR 10
MEMLOAD
REGCOPY MDR PC
ENDPROC
END
