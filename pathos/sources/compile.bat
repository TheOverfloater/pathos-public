@ECHO OFF
:Mapname
REM Change dir first
cd .\compile

SET /P mapname=Map to compile: 
IF "%mapname%"=="" GOTO Error

REM Check if the BSP file exists
IF NOT exist %mapname%.map GOTO Error_NoMAP

ECHO Compiling %mapname%...
hlcsg -chart -low -cliptype precise -estimate %mapname%.map
hlbsp -lightmapscale 4.0 -chart -low -estimate %mapname%.map

REM Fail if we have a leak pointfile
IF exist %mapname%.lin GOTO Error_Leak

hlvis -chart -low -estimate %mapname%.map
hlrad -extra -compress 0 -rgbcompress 0 -compressionlevel uber_compression -bumpmaps -low -chart -blur 2.0 -smooth 150 -smooth2 60 -notexscale -minlight 1 -estimate %mapname%.map
GOTO CopyPrompt

:CopyPrompt
SET /P CopyInput=Copy BSP to maps?(Y/N):
IF "%CopyInput%"=="Y" ( GOTO CopyBSP ) ELSE ( IF "%CopyInput%"=="N" ( GOTO Exit ) ELSE ( GOTO CopyPrompt_Error ) )

:CopyBSP
ECHO Copying "compile/%mapname%.bsp" to "maps/%mapname%.bsp"
copy %mapname%.bsp ..\..\maps\ /y

IF exist ./%mapname%.ald ( GOTO Copy_ALD ) ELSE ( GOTO Exit )

:Error
ECHO Error: No map specified!
GOTO Mapname

:Error_NoMAP
ECHO Error: File %mapname%.map is missing
GOTO Exit

:Error_Leak
ECHO Error: Leak detected. Ceasing compile.
GOTO Exit

:Copy_ALD
ECHO "Copying compile/%mapname%.ald" to "maps/%mapname%.ald"
copy %mapname%.ald ..\..\maps\ /y
GOTO Exit

:CopyPrompt_Error
ECHO Only "Y/N" accepted
GOTO CopyPrompt

:Exit
set /p DUMMY=Hit ENTER to exit...