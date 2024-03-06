@ECHO OFF
:Mapname
REM Change dir first
cd .\compile

SET /P mapname=Map to copy from maps: 
IF "%mapname%"=="" GOTO Error

REM Check if the BSP file exists
IF NOT exist ../../maps/%mapname%.bsp GOTO Error_NoMAP

ECHO Copying maps/%mapname%.bsp to compile/maps/%mapname%.bsp
copy ..\..\maps\%mapname%.bsp .\ /y

GOTO Exit

:Error
ECHO Error: No map specified!
GOTO Mapname

:Error_NoMAP
ECHO Error: File maps/%mapname%.bsp is missing
GOTO Exit

:Exit
set /p DUMMY=Hit ENTER to exit...