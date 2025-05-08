@echo off
REM ================= PSX DEVELOPMENT ENVIRONMENT VARIABLES =============
REM       RELEASE 1.8 LIBRARY 3.6.1 Date: 1-31-1998 Time: 21:38:44

set PATH=C:\Psyq\bin;
set PSX_PATH=C:\Psyq\bin
set LIBRARY_PATH=C:\Psyq\lib
set C_PLUS_INCLUDE_PATH=C:\Psyq\include
set C_INCLUDE_PATH=C:\Psyq\include
set PSYQ_PATH=C:\Psyq\bin
set COMPILER_PATH=C:\Psyq\bin
set GO32=DPMISTACK 1000000
set G032TMP=C:\WINDOWS\TEMP
set TMPDIR=C:\WINDOWS\TEMP

REM 
cd ..\src

REM 
ccpsx -O3 -Xo$80010000 main.c -o ..\main.cpe, ..\main.sym, ..\main.map

REM 
cpe2x32 ..\main.cpe

REM 
if exist ..\main.sym del ..\main.sym
if exist ..\main.cpe del ..\main.cpe
if exist ..\main.map del ..\main.map

pause
