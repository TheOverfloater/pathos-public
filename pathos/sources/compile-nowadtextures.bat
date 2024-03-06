@ECHO OFF
:Mapname
REM Change dir first
cd .\compile

SET /P mapname=Map to compile: 
IF "%mapname%"=="" GOTO Error

REM Check if the BSP file exists
IF NOT exist %mapname%.map GOTO Error_NoMAP

ECHO Compiling %mapname%...
hlcsg -nowadtextures -chart -cliptype simple -estimate %mapname%.map

GOTO Exit

:Error
ECHO Error: No map specified!
GOTO Mapname

:Error_NoMAP
ECHO Error: File %mapname%.map is missing
GOTO Exit

:Exit
set /p DUMMY=Hit ENTER to exit...