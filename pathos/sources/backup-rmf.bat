@ECHO OFF
:Mapname
REM Change dir first
cd .\mapsrc

SET /P mapname=RMF to copy to mapsrc/backups: 
IF "%mapname%"=="" GOTO Error

REM Check if the RMF file exists
IF NOT exist ./%mapname%.rmf GOTO Error_NoRMF

ECHO Backing up mapsrc/%mapname%.rmf to mapsrc/backups/

set Number=0
GOTO BackupLoop

:BackupLoop
set /A Number=%Number%+1
set NumberFinal=%Number%

REM Add one zero for less than ten
IF %NumberFinal% LSS 10 SET NumberFinal=0%NumberFinal%
REM Add one zero for less than hundred
IF %NumberFinal% LSS 100 SET NumberFinal=0%NumberFinal%

set Filename=%mapname%_%NumberFinal%
IF exist .\backups\%Filename%.rmf GOTO BackupLoop
copy %mapname%.rmf .\backups\%Filename%.rmf /y
ECHO Copied mapsrc/%mapname%.rmf to mapsrc/backups/%Filename%.rmf
GOTO Exit

:Error
ECHO Error: No map specified!
GOTO Mapname

:Error_NoRMF
ECHO Error: File mapsrc/%mapname%.rmf is missing
GOTO Exit

:Exit
set /p DUMMY=Hit ENTER to exit...