@Simple Text Editor Using Simple stdin Buffer Techniques
@Written for PCBox
@----------------------------------
@Constant Strings
CONST C50 "Enter the Filename to Open: "
CONST C51 "Error: file doesn't exist"
CONST C52 "Press F11 to Exit Discarding Changes"
CONST C53 "Press F12 to Exit Saving Changes"
@----------------------------------
@Frequently Used Values
REGSET S:150 75 @Left Arrow
REGSET S:151 77 @Right arrow
REGSET S:152 8 @Backspace
REGSET S:153 10 @Enter
REGSET S:154 72 @Up arrow
REGSET S:155 80 @Down arrow
REGSET S:156 600 @Lines handler starting points
REGSET S:157 100 @String starting points
REGSET S:158 133 @F11
REGSET S:159 134 @F12
@----------------------------------
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
RUNPROC Print_File
@From here on, don't touch Z ,U And A (A is the line count thingy)
DEC A
REGSET Y 0 @Used for logic
REGCOPY MAR Z @Store text len
REGCOPY Z U @Cursor position
REGCOPY A S:160 @Maximum number of lines, store it.
INC S:156
:Next_Key
GETKEY X
EQ X S:150 Y
CRUN Y RUNPROC MOVE_CURSOR_BACK
CRUN Y GOTO Next_Key
EQ X S:151 Y
CRUN Y RUNPROC MOVE_CURSOR_FORWARD
CRUN Y GOTO Next_Key
EQ X S:154 Y
CRUN Y RUNPROC MOVE_CURSOR_UP
CRUN Y GOTO Next_Key
EQ X S:155 Y
CRUN Y RUNPROC MOVE_CURSOR_DOWN
CRUN Y GOTO Next_Key
EQ X S:152 Y
CRUN Y RUNPROC ERASE_CHARACTER
CRUN Y GOTO Next_Key
EQ X S:158 Y
CRUN Y RUNPROC Quit_Txt
EQ X S:159 Y
CRUN Y RUNPROC Txt_Exit_Save
RUNPROC INSERT_CHARACTER
GOTO Next_Key
@----------------------------------
@Procedures
@----------------------------------
PROC Quit_Txt:
CLS
REGSET MAR 10
MEMLOAD
REGCOPY MDR PC
ENDPROC
@----------------------------------
PROC Txt_Exit_Save:
REGSET Y 0 @Helper iterator
REGSET V 0 @Logic
REGCOPY S:157 MAR
:Write_To_File_Txt_Next_Char
MEMLOAD
WNC Y MDR
LOWER MAR Z V
NOT V
CRUN V RUNPROC Write_EOF
CRUN V RUNPROC Quit_Txt
INC Y
INC MAR
GOTO Write_To_File_Txt_Next_Char
ENDPROC
@----------------------------------
PROC Write_EOF:
WNC Y S:100
INC Y
WNC Y S:105
INC Y
WNC Y S:106
INC Y
WNC Y S:107
INC Y
WNC Y S:100
ENDPROC
@----------------------------------
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
@----------------------------------
PROC Print_File:
REGCOPY S:156 A @Iterator for line lenghts
REGSET S 0 @Temp buffer for MAR value
REGSET X 0 @Logic
REGSET Y 0 @Also Logic
REGSET U 0 @Iterator
REGSET Z 0 @Temp buffer
REGCOPY S:157 MAR @Write the outputed character into RAM, for later use
OUTPUT C52
OUTPUT S:100
OUTPUT C53
OUTPUT S:100
OUTPUT C0
OUTPUT S:100
:Print_File_Nextchar
RNC U Z
EQ Z S:100 X
CRUN X REGCOPY MAR S
CRUN X REGCOPY A MAR
CRUN X REGCOPY S MDR
CRUN X INC MDR
CRUN X MEMWRITE
CRUN X REGCOPY S MAR
CRUN X INC A
CRUN X RUNPROC Search_For_EOF
CRUN Y ENDPROC @If EOF is found end the search
INC U
REGCOPY Z MDR
OUTPUT Z
MEMWRITE
INC MAR
GOTO Print_File_Nextchar
ENDPROC
@----------------------------------
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
@----------------------------------
PROC MOVE_CURSOR_FORWARD:
REGCOPY A MAR
MEMLOAD
LOWER U Z X @Don't let the cursor go further than the text's length
NOT X
CRUN X ENDPROC
DEC MDR
LOWER U MDR X
NOT X
CRUN X ENDPROC
REGCOPY U MAR
MEMLOAD
OUTPUT MDR
INC U
REGSET Y 1
ENDPROC
@----------------------------------
PROC MOVE_CURSOR_BACK:
HIGHER U S:103 X
DEC A
REGCOPY A MAR
MEMLOAD
HIGHER U MDR X
INC A
NOT X
CRUN X ENDPROC
OUTPUT S:152
DEC U
REGSET Y 1
ENDPROC
@----------------------------------
PROC MOVE_CURSOR_UP:
DEC A
LOWER A S:156 X
INC A
INC A
CRUN A EQ A S:156 U
DEC A
DEC A
NOT U
CRUN X CRUN U CURSORUP
CRUN X CRUN U DEC A
CRUN X REGCOPY S:157 U
CRUN X INC A
CRUN X ENDPROC
DEC A
REGCOPY A MAR
MEMLOAD
INC A
CURSORUP
REGCOPY MDR U
REGSET Y 1
ENDPROC
@----------------------------------
PROC MOVE_CURSOR_DOWN:
LOWER A S:160 X
NOT X
CRUN X DEC A
REGCOPY A MAR
MEMLOAD
REGCOPY MDR U
NOT X
CRUN X CURSORDOWN
INC A
REGSET Y 1
ENDPROC
@----------------------------------
PROC INSERT_CHARACTER:
@---------
@Update RAM
REGSET V 0 @Used for Logic operations
REGCOPY Z Y @Helper incremenetor
DEC Y
REGCOPY S:160 MAR
REGCOPY Z MDR
INC MDR
MEMWRITE
REGCOPY Z MAR
INC Z @Add text len by 1
LOWER Y U S:200
CRUN S:200 GOTO Print_Characters_Skip_Shift @Did we add a character to end? if yes, s:200 is True, and skip shifting stuff
:Shift_Next_Char
COPY Y @Copy Y to MAR (From RAM)
HIGHER Y U V
NOT V
CRUN V GOTO Print_Characters
DEC MAR
DEC Y
GOTO Shift_Next_Char
:Print_Characters
DEC MAR
:Print_Characters_Skip_Shift
REGCOPY X MDR
MEMWRITE
:Print_Next_Char
EQ MAR Z V
CRUN V GOTO Reset_Cursor
MEMLOAD
OUTPUT MDR
INC MAR
GOTO Print_Next_Char
:Reset_Cursor
INC U
LOWER A S:160 V
CRUN V REGCOPY A X @Backup A Value
CRUN V RUNPROC SET_CURSOR_Y
CRUN V REGCOPY X A @Restore backuped value
CRUN V RUNPROC SET_CURSOR_X
CRUN V ENDPROC
REGCOPY MAR X @Backup MAR value
@Update line length
REGCOPY A MAR
MEMLOAD
INC MDR
MEMWRITE
DEBUG
CRUN S:200 ENDPROC @If the character was added to the end, no need to reset the cursor
REGCOPY X MAR @Restore backuped value
:Cursor_Back_Repeat
OUTPUT S:152
DEC MAR
EQ MAR U V
NOT V
CRUN V GOTO Cursor_Back_Repeat
ENDPROC
PROC SET_CURSOR_Y:
REGCOPY A MAR
:Cursor_Up_Repeat
MEMLOAD
INC MDR
MEMWRITE
CURSORUP
INC MAR
EQ MAR S:160 V
CRUN V ENDPROC
GOTO Cursor_Up_Repeat
ENDPROC
PROC SET_CURSOR_X:
LOWER A S:156 V
CRUN V REGCOPY S:157 MAR
CRUN V GOTO Cursor_Forward_Insert_Repeat
REGCOPY A MAR
DEC MAR
MEMLOAD
REGCOPY MDR MAR
:Cursor_Forward_Insert_Repeat
EQ MAR U V
CRUN V ENDPROC
MEMLOAD
OUTPUT MDR
INC MAR
@Inja Boodesh
GOTO Cursor_Forward_Insert_Repeat
ENDPROC
@----------------------------------
PROC ERASE_CHARACTER:
@Check if erasing is allowed or not
LOWER A S:156 V
CRUN V HIGHER U S:157 Y
CRUN V NOT Y
CRUN V CRUN Y ENDPROC
REGCOPY A MAR
DEC MAR
MEMLOAD
HIGHER U MDR V
NOT V
CRUN V REGSET Y 1
CRUN V ENDPROC
@Update RAM
REGSET V 0 @Used for Logic operations
REGCOPY U Y @Helper incremenetor
REGCOPY S:160 MAR
REGCOPY Z MDR
DEC MDR
MEMWRITE
REGCOPY U MAR
DEC MAR
DEC U
DEC Z @Decrement text len
:Shift_Next_Char_Back
COPY Y @Copy Y to MAR (From RAM)
LOWER Y Z V
NOT V
CRUN V GOTO Print_Characters_Back
INC MAR
INC Y
GOTO Shift_Next_Char_Back
:Print_Characters_Back
HIGHER Y Z S:200 @S:200 is true if the last character has been deleted
CLS
OUTPUT C52
OUTPUT S:100
OUTPUT C53
OUTPUT S:100
OUTPUT C0
OUTPUT S:100
REGCOPY S:157 MAR
:Print_Next_Char_Back
EQ MAR Z V
CRUN V GOTO Reset_Cursor_Back
MEMLOAD
OUTPUT MDR
INC MAR
GOTO Print_Next_Char_Back
:Reset_Cursor_Back
CRUN S:200 ENDPROC @If the last character was the character that got deleted, no need to reset cursor back
LOWER A S:160 V
CRUN V REGCOPY A X @Backup A Value
CRUN V RUNPROC SET_CURSOR_Y_Back
CRUN V REGCOPY X A @Restore backuped value
CRUN V RUNPROC SET_CURSOR_X
CRUN V ENDPROC
REGCOPY MAR X @Backup MAR value
@Update line length
REGCOPY A MAR
MEMLOAD
DEC MDR
MEMWRITE
REGCOPY X MAR @Restore backuped value
:Cursor_Back_Repeat_Back
OUTPUT S:152
DEC MAR
EQ MAR U V
NOT V
CRUN V GOTO Cursor_Back_Repeat_Back
REGSET Y 1
ENDPROC
PROC SET_CURSOR_Y_Back:
REGCOPY A MAR
:Cursor_Up_Repeat_Back
MEMLOAD
DEC MDR
MEMWRITE
CURSORUP
INC MAR
EQ MAR S:160 V
CRUN V ENDPROC
GOTO Cursor_Up_Repeat_Back
ENDPROC
END