@ECHO OFF
:Mapname
REM Change dir first
cd .\compile

SET /P mapname=Map to copy to maps: 
IF "%mapname%"=="" GOTO Error

REM Check if the BSP file exists
IF NOT exist ./%mapname%.bsp GOTO Error_NoBSP

ECHO Copying compile/%mapname%.bsp to maps/%mapname%.bsp
copy %mapname%.bsp ..\..\maps\ /y

IF exist ./%mapname%.ald GOTO Copy_ALD

GOTO Exit

:Copy_ALD
ECHO Copying compile/%mapname%.ald to maps/%mapname%.ald
copy %mapname%.ald ..\..\maps\ /y
GOTO Exit

:Error
ECHO Error: No map specified!
GOTO Mapname

:Error_NoBSP
ECHO Error: File compile/%mapname%.bsp is missing
GOTO Exit

:Exit
set /p DUMMY=Hit ENTER to exit...