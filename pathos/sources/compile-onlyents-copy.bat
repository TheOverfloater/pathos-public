@ECHO OFF
:Mapname
REM Change dir first
cd .\compile

SET /P mapname=Map to update: 
IF "%mapname%"=="" GOTO Error

IF NOT exist ../../maps/%mapname%.bsp GOTO Error_NoBSP

REM Check if the MAP file exists
IF NOT exist %mapname%.map GOTO Error_NoMAP

ECHO Copying maps/%mapname%.bsp to compile/maps/%mapname%.bsp
copy ..\..\maps\%mapname%.bsp .\ /y

ECHO Updating %mapname%...
hlcsg -chart -estimate -onlyents %mapname%.map
GOTO CopyPrompt

:CopyPrompt
SET /P CopyInput=Copy BSP to maps?(Y/N):
IF "%CopyInput%"=="Y" ( GOTO CopyBSP ) ELSE ( IF "%CopyInput%"=="N" ( GOTO Exit ) ELSE ( GOTO CopyPrompt_Error ) )

:CopyBSP
ECHO Copying "compile/%mapname%.bsp" to "maps/%mapname%.bsp"
copy %mapname%.bsp ..\..\maps\ /y

IF exist ./%mapname%.ald GOTO Copy_ALD

GOTO Exit

:Error_NoMAP
ECHO Error: File %mapname%.map is missing
GOTO Exit

:Error_NoBSP
ECHO Error: File maps/%mapname%.bsp is missing
GOTO Exit

:Copy_ALD
ECHO Copying "compile/%mapname%.ald" to "maps/%mapname%.ald"
copy %mapname%.ald ..\..\maps\ /y
GOTO Exit

:CopyPrompt_Error
ECHO Only "Y/N" accepted
GOTO CopyPrompt

:Error
ECHO Error: No map specified!
GOTO Mapname

:Exit
set /p DUMMY=Hit ENTER to exit...