@Constant Strings
CONST C50 "Enter the Filename to Open:"
CONST C51 "Error: file doesn't exist"
CONST C52 "Enter the Filename to write the Image Data to:"
CONST C53 "Select One of the Following Effects"
CONST C54 "1. Grayscale"
CONST C56 "2. Negative"
CONST C57 "3. Color Correction"
CONST C55 "Enter Effect's Index (1 ~ n):"
CONST C58 "Select Color Channel (R - G - B):"
CONST C59 "Enter Correction Values (-255 ~ 255):"
CONST C60 "File already exists!"
CONST C61 "Number is Invalid (Correction value must be between -255 and 255)"
@ERR E15 "File already exists!"
@ERR E16 "Number is Invalid (Correction value must be between -255 and 255)"
@--------------------------------
@Frequently Used Values (Cache)
REGSET S:150 3
REGSET S:151 100
REGSET S:152 255
REGSET S:153 8 @Shift Value
REGSET S:154 16 @Shift Value
REGSET S:155 1
REGSET S:156 45 (@Negative Sign '-')
REGSET S:157 R
REGSET S:158 G
REGSET S:159 B
@--------------------------------
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
OUTPUT C53
OUTPUT S:100
OUTPUT C54
OUTPUT S:100
OUTPUT C56
OUTPUT S:100
OUTPUT C57
OUTPUT S:100
OUTPUT C55
INPUT X
SUB X S:104 X
INPUT Y @Read '\n'
DEC X
EQ X S:103 Y
CRUN Y RUNPROC Image_Grayscale
CRUN Y REGSET S:156 1 @Set RGB or Grayscale State
CRUN Y GOTO Get_File_Output_Again
EQ X S:155 Y
CRUN Y RUNPROC Image_Negative
CRUN Y REGSET S:156 0 @Set RGB or Grayscale State
CRUN Y GOTO Get_File_Output_Again
DEC X
EQ X S:155 Y
CRUN Y RUNPROC Color_Correction
CRUN Y REGSET S:156 0 @Set RGB or Grayscale State
CRUN Y GOTO Get_File_Output_Again
:Get_File_Output_Again
OUTPUT C52
RUNPROC Get_Input_To_A0_An @Get filename
FEX X A
CRUN X OUTPUT C:55
CRUN X OUTPUT S:100
CRUN X GOTO Get_File_Input_Again
MKF A
FOPEN A
CRUN S:156 RUNPROC Write_Image_Data_From_RAM
NOT S:156
CRUN S:156 RUNPROC Write_Image_Data_From_RAM_RGB
RUNPROC Quit_Img_Fx
@--------------------------------
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
PROC Quit_Img_Fx:
REGSET MAR 10
MEMLOAD
REGCOPY MDR PC
ENDPROC
PROC Image_Grayscale:
REGSET U 0 @Iterator
REGSET S:1 0 @R
REGSET S:2 0 @G
REGSET S:3 0 @B
REGCOPY S:151 MAR @Image data starting points (to RAM)
RNC U S @Width
INC U
RNC U V @Height
REGCOPY V S:10 @Backup height Value
MUL V S V @Total number of Pixels
ADD V MAR V
INC U
INC U
:Next_Pixel
RNC U S:1
INC U
RNC U S:2
INC U
RNC U S:3
INC U
@Average RGB Values
ADD S:1 S:2 S:1
ADD S:1 S:3 S:1
DIV S:1 S:150 MDR
@Write averaged values to RAM
MEMWRITE
INC MAR
LOWER MAR V X
NOT X
CRUN X REGSET Y 1
CRUN X ENDPROC
GOTO Next_Pixel
REGSET Y 1
ENDPROC
PROC Color_Correction:
REGSET U 0 @Iterator
REGSET S:1 0 @R
REGSET S:2 0 @G
REGSET S:3 0 @B
REGCOPY S:151 MAR @Image data starting points (to RAM)
RNC U S @Width
INC U
RNC U V @Height
REGCOPY V S:10 @Backup height
MUL V S V @Total number of Pixels
ADD V MAR V
INC U
INC U
@How Much?
:How_Much
OUTPUT C59
RUNPROC Get_Integer_Input_To_X
DEBUG
RUNPROC Check_Validity
CRUN Y GOTO How_Much
@Which Channel?
:Which_Channel
REGCOPY S:150 U
OUTPUT C58
INPUT Y
INPUT S:11 @Discard '\n'
EQ Y S:157 Z
CRUN Z GOTO Correct_R
EQ Y S:158 Z
CRUN Z GOTO Correct_G
EQ Y S:159 Z
CRUN Z GOTO Correct_B
GOTO Which_Channel
:Correct_R
RNC U S:1
INC U
RNC U S:2
INC U
RNC U S:3
INC U
@Correct R
ADD X S:1 S:1
HIGHER S:1 S:152 Z
CRUN Z REGCOPY S:152 S:1
LOWER S:1 S:103 Z
CRUN Z REGCOPY S:103 S:1
@Write values to RAM
SHIFTBACK S:1 S:154 S:1 @Shift 16 bits
SHIFTBACK S:2 S:153 S:2 @Shift 8 Bits
BITOR S:1 S:2 MDR
BITOR MDR S:3 MDR
MEMWRITE
INC MAR
LOWER MAR V Z
NOT Z
CRUN Z REGSET Y 1
CRUN Z ENDPROC
GOTO Correct_R
:Correct_G
RNC U S:1
INC U
RNC U S:2
INC U
RNC U S:3
INC U
@Correct G
ADD X S:2 S:2
HIGHER S:2 S:152 Z
CRUN Z REGCOPY S:152 S:2
LOWER S:2 S:103 Z
CRUN Z REGCOPY S:103 S:2
@Write values to RAM
SHIFTBACK S:1 S:154 S:1 @Shift 16 bits
SHIFTBACK S:2 S:153 S:2 @Shift 8 Bits
BITOR S:1 S:2 MDR
BITOR MDR S:3 MDR
MEMWRITE
INC MAR
LOWER MAR V Z
NOT Z
CRUN Z REGSET Y 1
CRUN Z ENDPROC
GOTO Correct_G
:Correct_B
RNC U S:1
INC U
RNC U S:2
INC U
RNC U S:3
INC U
@Correct B
ADD X S:3 S:3
HIGHER S:3 S:152 Z
CRUN Z REGCOPY S:152 S:3
LOWER S:3 S:103 Z
CRUN Z REGCOPY S:103 S:3
@Write values to RAM
SHIFTBACK S:1 S:154 S:1 @Shift 16 bits
SHIFTBACK S:2 S:153 S:2 @Shift 8 Bits
BITOR S:1 S:2 MDR
BITOR MDR S:3 MDR
MEMWRITE
INC MAR
LOWER MAR V Z
NOT Z
CRUN Z REGSET Y 1
CRUN Z ENDPROC
GOTO Correct_B
ENDPROC
PROC Image_Negative:
REGSET U 0 @Iterator
REGSET S:1 0 @R
RESET S:2 0 @G
REGSET S:3 0 @B
REGCOPY S:151 MAR @Image data starting points (to RAM)
RNC U S @Width
INC U
RNC U V @Height
REGCOPY V S:10 @Backup height Value
MUL V S V @Total number of Pixels
ADD V MAR V
INC U
INC U
:Next_Pixel_Negative
RNC U S:1
INC U
RNC U S:2
INC U
RNC U S:3
INC U
@Negative Pixels
SUB S:152 S:1 S:1
SUB S:152 S:2 S:2
SUB S:152 S:3 S:3
@Write averaged values to RAM
SHIFTBACK S:1 S:154 S:1 @Shift 16 bits
SHIFTBACK S:2 S:153 S:2 @Shift 8 Bits
BITOR S:1 S:2 MDR
BITOR MDR S:3 MDR
MEMWRITE
INC MAR
LOWER MAR V X
NOT X
CRUN X ENDPROC
GOTO Next_Pixel_Negative
ENDPROC
PROC Write_Image_Data_From_RAM:
REGCOPY MAR V @Maximum value of MAR
REGCOPY S:151 MAR
REGSET U 0 @Iterator on File
WNC U S @Write image width
INC U
WNC U S:10 @Write image height
INC U
INC U
:Write_Next_Pixel
MEMLOAD
WNC U MDR
INC U
WNC U MDR
INC U
WNC U MDR
INC U
INC MAR
LOWER MAR V X
NOT X
CRUN X ENDPROC
GOTO Write_Next_Pixel
ENDPROC
PROC Write_Image_Data_From_RAM_RGB:
REGSET S:1 0 @R
REGSET S:2 0 @G
REGSET S:3 0 @B
REGCOPY MAR V @Maximum value of MAR
REGCOPY S:151 MAR
REGSET U 0 @Iterator on File
WNC U S @Write image width
INC U
WNC U S:10 @Write image height
INC U
INC U
:Write_Next_RGB_Pixel
MEMLOAD
SHIFTFORWARD MDR S:154 S:1
BITAND S:1 S:152 S:1
SHIFTFORWARD MDR S:153 S:2
BITAND S:2 S:152 S:2
BITAND MDR S:152 S:3
WNC U S:1
INC U
WNC U S:2
INC U
WNC U S:3
INC U
INC MAR
LOWER MAR V X
NOT X
CRUN X ENDPROC
GOTO Write_Next_RGB_Pixel
ENDPROC
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
PROC Check_Validity:
HIGHER X S:152 Z
NEG S:152
LOWER X S:152 Y
NEG S:152
OR Y Z Y
CRUN Y OUTPUT C61
CRUN Y OUTPUT S:100
ENDPROC
END
