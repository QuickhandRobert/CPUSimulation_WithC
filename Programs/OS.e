@----------------------------------
@Constant Strings (Menu Items) : C0 -> C4
CONST C0 ----------------------------------------------
CONST C1 "Welcome to PCBox v"
CONST C2 1.0	@OS Version
CONST C3 "Use help to display a list of commands"
CONST C4 "~[PCBox - " @Temp TODO: Account system (hash passwords)
@----------------------------------
@Error Definitions
ERR E0 "File $ doesn't exist"
ERR E1 "File $ has been removed successfuly"
ERR E2 "File $ already exists!"
ERR E3 "File $ created successfuly"
ERR E4 "File $ successfuly renamed to $"
@----------------------------------
@Frequently Used Numbers
REGSET S:100 10 @Newline terminator (\n) + Used in string to number stuff
REGSET S:101 > @waiting for commands
REGSET S:102 : @Username & password seperator in accounts.db
REGSET S:103 0 @Used in many places
REGSET S:104 48 @Used in string to number stuff
@Used for finding EOF
REGSET S:105 E
REGSET S:106 N
REGSET S:107 D
REGSET S:108 10  TODO: Use this for something else
REGSET S:109 ]
REGSET S:130 32 @Space, used for tokenizing user input
REGSET S:131 86400 @(Seconds in day)
REGSET S:132 3600  @(Seconds in an hour)
REGSET S:133 60 @(Yeah u know what this is)
REGSET S:134 4
REGSET S:135 34
@----------------------------------
@Hashed Commands
REGSET S:110 210714636462 @help
REGSET S:111 6385120071 @cls
REGSET S:112 249906522640248705 @Shutdown
REGSET S:113 193504740 @rm
REGSET S:114 6385477923 @mkf
REGSET S:115 229481155741629 @rename
REGSET S:116 210729011988 @time
REGSET S:117 6954030892083 @start
REGSET S:118 6385152708 @dir
REGSET S:119 210711002436 @echo
REGSET S:120 8246405298631896759 @Hibernate
@----------------------------------
@Help Menu Stuff : C5 -> C16
CONST C5 "CLS         Clears the Screen"
CONST C6 "COPY        Copying files"
CONST C7 "TIME        Displays the current time"
CONST C8 "DIR         List all files in the current directory"
CONST C9 "ECHO        Displays a message"
CONST C10 "HELP        Displays this menu"
CONST C11 "MF          Creates a file with a given name"
CONST C12 "MD          Creates a directory with a given name"
CONST C13 "RM          Removes a given file"
CONST C14 "RMDIR       Removes a given directory"
CONST C15 "RENAME      Renames a file or directory"
CONST C16 "SHUTDOWN    System shutdown"
CONST C17 "HIBERNATE   Puts the system into hibernate mode"
CONST C18 "START       Starts a given program"
CONST C19 "Invalid command!"
@----------------------------------
@User Login
CONST C20 "Enter your Username:"
CONST C21 "Enter your Password:"
CONST C22 "Wrong Username/Password entered!"
@----------------------------------
@Time & Date
CONST C23 "The time in your timezone is: "
@----------------------------------
@General OS Stuff
RUNPROC User_Login
RUNPROC PRINT_BOOT_MENU
:Waiting_For_CMD
RUNPROC PRINT_COMMAND_WAIT
RUNPROC Get_Input_To_A0_An
RUNPROC HASH_A0toAn_ToX
RUNPROC Execute_Command_From_X
GOTO Waiting_For_CMD
@----------------------------------
@Procedures
@---------------------------------
PROC Start_Program:
REGSET X 0
RUNPROC Get_Input_To_A0_An
FEX X A
NOT X
CRUN X CERR E0 A
CRUN X OUTPUT S:100
CRUN X ENDPROC
FOPEN A
LOADTOMEMORY X
REGCOPY PC MDR
ADD MDR S:134 MDR
REGSET MAR 10
MEMWRITE
REGCOPY X PC
UNLOADFROMMEMORY
ENDPROC
@---------------------------------
PROC Echo_Input:
REGSET X 0 @Where the input goes
REGSET Y 0 @Logic
:Echo_Next_Char
INPUT X
EQ X S:103 Y
CRUN Y OUTPUT S:100
CRUN Y ENDPROC
EQ X S:135 Y
CRUN Y GOTO Echo_Next_Char
OUTPUT X
GOTO Echo_Next_Char
ENDPROC
@---------------------------------
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
@---------------------------------
PROC Print_Time:
REGSET X 0
REGSET Y 0
REGSET Z 0
OUTPUT C23
GETTIME X
GETTIMEZONE Y
ADD X Y X
LO X S:131 X @(86400)
DIV X S:132 Z @(Get number of hours)
LOWER Z S:100 U @2 digits padding
CRUN U OUTPUT S:104
RUNPROC Print_Number_Z_A
OUTPUT S:102
LO X S:132 X
DIV X S:133 Z
LOWER Z S:100 U @2 digits padding
CRUN U OUTPUT S:104
RUNPROC Print_Number_Z_A
OUTPUT S:102
LO X S:133 Z
LOWER Z S:100 U @2 digits padding
CRUN U OUTPUT S:104
RUNPROC Print_Number_Z_A
OUTPUT S:100
REGSET U 1
ENDPROC
@---------------------------------
PROC List_Files:
REGSET U 0 @Iterator
REGSET V 0 @Logic stuff
:Next_File
GETFILEINFO U A
INC U
EQ A S:103 V
CRUN V ENDPROC
RUNPROC PRINT_A_Z
OUTPUT S:100
GOTO Next_File
ENDPROC
@---------------------------------
PROC PRINT_A_Z:
REGSET Z -1 @Iterator
REGSET Y 0 @Logic
:A_Next_Char
INC Z
EQ A:Z S:103 Y
CRUN Y ENDPROC
OUTPUT A:Z
GOTO A_Next_Char
ENDPROC
@---------------------------------
PROC Print_Number_Z_A:
REGSET U 0 (@Iterator)
REGSET V 0 (Used for logic)
@Convert number to string:
:Number_To_String_Continue
LO Z S:100 A:U
DIV Z S:100 Z
INC U
EQ Z S:103 V
NOT V
CRUN V GOTO Number_To_String_Continue
@Print in Reverse:
:Print_In_Reverse_Continue
DEC U
ADD A:U S:104 A:U (@Convert to ASCII character)
OUTPUT A:U
EQ U S:103 V
NOT V
CRUN V GOTO Print_In_Reverse_Continue
ENDPROC
@---------------------------------
PROC Get_Input_To_S0_Sn:
@Used Registers:
REGSET X 0 @Input
REGSET U 0 @Used for 0 value
REGSET V 0 @Comparison result
REGSET Y -1 @Itterator
:Get_Input_To_S0_Sn_Iterate
INC Y
INPUT X
EQ X S:103 U @Is input finished?
EQ X S:130 V @Or space is detected?
OR U V V
CRUN V GOTO Get_Input_To_S0_Sn_END @End reading stdin then
REGCOPY X S:Y
GOTO Get_Input_To_S0_Sn_Iterate
:Get_Input_To_S0_Sn_END
REGCOPY S:103 S:Y
ENDPROC
@---------------------------------
PROC REMOVE_FILE:
REGSET U 0
RUNPROC Get_Input_To_A0_An
FEX U A
NOT U
CRUN U CERR E0 A
CRUN U OUTPUT S:100
CRUN U ENDPROC
RM A
CERR E1 A
OUTPUT S:100
REGSET U 1
ENDPROC
@---------------------------------
PROC MAKE_FILE:
REGSET U 0
RUNPROC Get_Input_To_A0_An
FEX U A
CRUN U CERR E2 A
CRUN U OUTPUT S:100
CRUN U ENDPROC
MKF A
CERR E3 A
OUTPUT S:100
REGSET U 1
ENDPROC
@---------------------------------
PROC RENAME_FILE:
REGSET U 0
RUNPROC Get_Input_To_A0_An
RUNPROC Get_Input_To_S0_Sn
FEX U A
NOT U
CRUN U CERR E0 A
CRUN U OUTPUT S:100
CRUN U ENDPROC
FEX U S
CRUN U CERR E2 S
CRUN U OUTPUT S:100
CRUN U ENDPROC
RENAME A S
CERR E4 A S
OUTPUT S:100
REGSET U 1
ENDPROC
@---------------------------------
PROC PRINT_HELP_MENU:
@Used Registers:
REGSET U 4 @Iterator
REGSET V 0
REGSET Z 17 @Maximum
:PRINT_HELP_MENU_REPEAT
INC U
OUTPUT C:U
OUTPUT S:100 @Newline
EQ U Z V
NOT V
CRUN V GOTO PRINT_HELP_MENU_REPEAT
ENDPROC
@---------------------------------
PROC PRINT_BOOT_MENU:
OUTPUT C0
OUTPUT S:100
OUTPUT C1
OUTPUT C2
OUTPUT S:100
OUTPUT C3
OUTPUT S:100
OUTPUT C0
OUTPUT S:100
ENDPROC
@---------------------------------
PROC PRINT_COMMAND_WAIT:
OUTPUT C4
REGSET MAR 50
REGSET MDR 0
:PRINT_COMMAND_WAIT_Repeat
MEMLOAD
INC MAR
EQ MDR S:103 A:255 @S:103 -> Zero is stored | A:255 Result
NOT A:255
CRUN A:255 OUTPUT MDR
CRUN A:255 GOTO PRINT_COMMAND_WAIT_Repeat
OUTPUT S:109
OUTPUT S:101
ENDPROC
@---------------------------------
PROC HASH_A0toAn_ToX:
@Used Registers
REGSET U 0
REGSET V 0
REGSET X 5381 @Hash Key
REGSET Y -1 @Iterator
REGSET Z 5 @Shift Value
:HASH_Iterate
INC Y
SHIFTBACK X Z V
ADD X V X
ADD X A:Y X
REGSET U 0
EQ A:Y U U
NOT U
CRUN U GOTO HASH_Iterate
ENDPROC
@---------------------------------
PROC Hash_Input_To_X:
OUTPUT C21
REGSET V 0
REGSET X 5381
REGSET Y 0
REGSET A:255 5 @Shift Value
DISABLEECHO
:HASH_Iterate_File
INPUT V
EQ V S:103 Y
CRUN Y ENABLEECHO
CRUN Y ENDPROC
SHIFTBACK X A:255 Y
ADD X Y X
ADD X V X
NOT V
GOTO HASH_Iterate_File
ENDPROC
@---------------------------------
PROC Execute_Command_From_X:
REGSET U 0 @Used for logic stuff
EQ X S:110 U
CRUN U RUNPROC PRINT_HELP_MENU
CRUN U ENDPROC
EQ X S:111 U
CRUN U CLS
CRUN U ENDPROC
EQ X S:112 U
CRUN U SHUTDOWN
CRUN U ENDPROC
EQ X S:113 U
CRUN U RUNPROC REMOVE_FILE
CRUN U ENDPROC
EQ X S:114 U
CRUN U RUNPROC MAKE_FILE
CRUN U ENDPROC
EQ X S:115 U
CRUN U RUNPROC RENAME_FILE
CRUN U ENDPROC
EQ X S:116 U
CRUN U RUNPROC Print_Time
CRUN U ENDPROC
EQ X S:117 U
CRUN U RUNPROC Start_Program
CRUN U ENDPROC
EQ X S:118 U
CRUN U RUNPROC List_Files
CRUN U ENDPROC
EQ X S:119 U
CRUN U RUNPROC Echo_Input
CRUN U ENDPROC
EQ X S:120 U
CRUN U HIBERNATE
CRUN U ENDPROC
OUTPUT C19
OUTPUT S:100
ENDPROC
@---------------------------------
PROC Load_UserName_Input_Into_RAM:
OUTPUT C20
REGSET MAR 50 @Starting point
REGSET MDR 0
:Load_UserName_Input_Into_RAM_NextChar
INPUT MDR
MEMWRITE
INC MAR
EQ MDR S:103 MDR
CRUN MDR ENDPROC
GOTO Load_UserName_Input_Into_RAM_NextChar
ENDPROC
@---------------------------------
PROC GOTO_NEXT_LINE: @Skips everything until a newline terminator is found
@Uses U, V
:GOTO_NEXT_LINE_Repeat
RNC U V
INC U
EQ V S:100 V
CRUN V ENDPROC
GOTO GOTO_NEXT_LINE_Repeat
ENDPROC
@---------------------------------
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
@---------------------------------
PROC User_Login:
@Used Registers
REGSET U 0
REGSET V 0
REGSET Z 0
FEX U accounts.db
NOT U
CRUN U MKF accounts.db
FOPEN accounts.db
:Login_Failed
REGSET U 0 @iterator
RUNPROC Load_UserName_Input_Into_RAM
:Read_Next_Username
REGSET MAR 50
REGSET MDR 0
REGSET Z 0
:Next_Char
MEMLOAD
RNC U V @Read the uth character into v
INC MAR
INC U
EQ V MDR Z
CRUN Z GOTO Next_Char
EQ V S:102 Z @Correct Username entered
CRUN Z GOTO Read_Password
RUNPROC GOTO_NEXT_LINE
RUNPROC Search_For_EOF
CRUN Y GOTO Read_Password
GOTO Read_Next_Username
:Read_Password
RUNPROC Hash_Input_To_X
REGSET Y 0
:String_To_Number_Iterate
RNC U V
INC U
EQ V S:100 A:255 @TODO: change this
CRUN A:255 GOTO User_Final_Output
MUL Y S:100 Y @Multiply by 10
SUB V S:104 V
ADD Y V Y
GOTO String_To_Number_Iterate
:User_Final_Output
EQ Y X Y @Compare the db with user input
AND Y Z Y @If Username & password are both correct, then
CRUN Y GOTO System_Start
OUTPUT C22
OUTPUT S:100
GOTO Login_Failed
:System_Start
CLS
ENDPROC
END
