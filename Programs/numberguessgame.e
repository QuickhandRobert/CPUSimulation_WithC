@Number Guessing Game Written for PCBox
@Constant Strings
CONST C50 "Welcome To the Number Guessing Game!"
CONST C51 "Go a Bit Higher!"
CONST C52 "Go a Bit Lower!"
CONST C53 "Enter a Number Between 0~1000:"
@Error Messages
ERR E10 "Congrats! The number was #."
ERR E11 "Error: The number # is smaller than zero."
ERR E12 "Error: The number # is larger than 1000."
@Frequently Used Values
REGSET S:150 1000 @Range
@-------------------------------------
CLS
OUTPUT C50
OUTPUT S:100
RUNPROC Generate_Random_Number_X
REGSET MAR 100 @Write the randomly generated number to USER_RAM 100
REGCOPY X MDR
MEMWRITE
:Get_Next_Guessed_Number
OUTPUT C53
RUNPROC Get_Integer_Input_To_X
REGSET Y 0
HIGHER X S:150 Y
CRUN Y CERR E12 X
CRUN Y OUTPUT S:100
CRUN Y GOTO Get_Next_Guessed_Number
LOWER X S:103 Y
CRUN Y CERR E11 X
CRUN Y OUTPUT S:100
CRUN Y GOTO Get_Next_Guessed_Number
HIGHER X MDR Y
CRUN Y OUTPUT C52
CRUN Y OUTPUT S:100
CRUN Y GOTO Get_Next_Guessed_Number
LOWER X MDR Y
CRUN Y OUTPUT C51
CRUN Y OUTPUT S:100
CRUN Y GOTO Get_Next_Guessed_Number
@Number Guessed
CERR E10 MDR
OUTPUT S:100
REGSET MAR 10
MEMLOAD
REGCOPY MDR PC
@-------------------------------------
@Procedures:
PROC Generate_Random_Number_X:
REGSET Y 31654698554654 @Seed
REGSET U 3 @Binary shift values
REGSET V 2 @Binary shift values
GETTIME X
ADD X Y X
SHIFTFORWARD X U Y
MUL X Y X
SHIFTBACK X V Y
ADD X Y X
REGSET Y 31654698554654 @Seed
SHIFTFORWARD Y U Y
INC Y @Division by zero prevension
DIV X Y X
@Get the final numbers value
LO X S:150 X
ENDPROC
@--------------------------------
PROC Get_Integer_Input_To_X:
REGSET X 0 @Output
REGSET Y 0 @Where input is stores and processed
REGSET U 0 @Logic
:Get_Integer_Input_To_X_Repeat
INPUT Y
EQ Y S:103 U
CRUN U ENDPROC
MUL X S:100 X
SUB Y S:104 Y
ADD X Y X
GOTO Get_Integer_Input_To_X_Repeat
ENDPROC
@----------------------------------
END