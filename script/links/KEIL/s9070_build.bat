cd /D %2

set axf_file=%2\..\..\example\main\project\KEIL\Output\application.axf
set linkdir=%2\..\..\script\links\KEIL
set tooldir=%2\..\..\script\tools
set outdir =%2

set ota_bin_ver=0x%date:~0,4%%date:~5,2%%date:~8,2%

cmd /c "copy %axf_file% Debug\Exe\application.axf"

echo %1 %2 %3 %4 > tmp.txt
echo %1 %2 %3 %4 > tmp.txt
echo %tooldir% >tmp.txt
echo %outputdir% >> tmp.txt
echo %outputplatform% >> tmp.txt
echo ota_bin_ver=%ota_bin_ver% >>tmp.txt


 
del Debug\Exe\application.map Debug\Exe\application.asm *.bin

cmd /c "%tooldir%\nm Debug/Exe/application.axf > Debug/Exe/application.map"
cmd /c "%tooldir%\nm Debug/Exe/application.axf > Debug/Exe/application.map.bak"

cmd /c "%tooldir%\objdump -d Debug/Exe/application.axf > Debug/Exe/application.asm"

cmd /c "%tooldir%\grep OTA_RAM_DATA.*Base Debug/Exe/application.map > tmp1.txt"
cmd /c "%tooldir%\gawk '{print $1}' tmp1.txt > dbgmsg"

cmd /c "%tooldir%\grep OTA_RAM_DATA.*Base Debug/Exe/application.map > tmp1.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' tmp1.txt"') do set ram_s=0x%%i
cmd /c "%tooldir%\grep OTA_RAM_DATA.*Limit Debug/Exe/application.map > tmp1.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' tmp1.txt"') do set ram_e=0x%%i
cmd /c "%tooldir%\grep USER_CODE_REGION.*Base Debug/Exe/application.map > tmp1.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' tmp1.txt"') do set flash_s=0x%%i
cmd /c "%tooldir%\grep USER_CODE_REGION.*Limit Debug/Exe/application.map > tmp1.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' tmp1.txt"') do set flash_e=0x%%i

#for /f "delims=" %%i in ('cmd /c "%tooldir%\grep OTA_RAM_DATA.*Base Debug/Exe/application.map | %tooldir%\gawk '{print $1}'"') do set ram_s=0x%%i
#for /f "delims=" %%i in ('cmd /c "%tooldir%\grep OTA_RAM_DATA.*Limit Debug/Exe/application.map | %tooldir%\gawk '{print $1}'"') do set ram_e=0x%%i

#for /f "delims=" %%i in ('cmd /c "%tooldir%\grep USER_CODE_REGION Debug/Exe/application.map | %tooldir%\grep Base | %tooldir%\gawk '{print $1}'"') do set flash_s=0x%%i
#for /f "delims=" %%i in ('cmd /c "%tooldir%\grep USER_CODE_REGION Debug/Exe/application.map | %tooldir%\grep Limit | %tooldir%\gawk '{print $1}'"') do ( 
#set flash_e=0x%%i 
#)

echo "Test " >> tmp.txt
echo ram_s: %ram_s% >> tmp.txt
echo ram_e: %ram_e% >> tmp.txt
echo flash_s: %flash_s% >> tmp.txt
echo flash_e: %flash_e% >> tmp.txt

findstr /rg "place" Debug\List\application.map >> tmp.txt
setlocal enabledelayedexpansion
for /f "delims=:" %%i in ('findstr /rg "0x10015900" tmp.txt') do (
    set "var=%%i"
    set "sec_ram=!var:~1,2!"
)
for /f "delims=:" %%i in ('findstr /rg ".ota1_image.text" tmp.txt') do (
    set "var=%%i"
    set "sec_text=!var:~1,2!"
)

setlocal disabledelayedexpansion

echo sec_ram: %sec_ram% sec_text: %sec_text%  >> tmp.txt


::del tmp.txt
%tooldir%\objcopy -j OTA_RAM_DATA -j OTA_RAM_BSS -Obinary Debug/Exe/application.axf Debug/Exe/ram.bin
%tooldir%\objcopy -j USER_CODE_REGION -Obinary Debug/Exe/application.axf Debug/Exe/text.bin

%tooldir%\merge %2/Debug/Exe/text.bin %2/Debug/Exe/ram.bin %2/Debug/Exe/ota1.bin %2\tmp.txt
%tooldir%\checksum %2/Debug/Exe/ota1.bin %2/Debug/Exe/ota1_checksum.bin

set build_type=4

if "%build_type%"=="4" (
set flash_t=F
) else (
set flash_t=T
)

set ota_type=2

if "%ota_type%"=="1" (
set ota_t=1
) else (
set ota_t=2
)

%tooldir%\image %flash_t% ALL .\Debug\Exe\ 
%tooldir%\image %flash_t% OTA%ota_t%_WLAN .\Debug\Exe\
%tooldir%\image %flash_t% OTA%ota_t% .\Debug\Exe\
::s9070.board

@echo ^<?xml version="1.0" encoding="iso-8859-1"?^> > s9070.board

@echo ^<flash_board^> >> s9070.board

@echo   ^<pass^> >> s9070.board
@echo   ^<loader^>$PROJ_DIR$\flashloader\s907x.flash^</loader^>    >> s9070.board
@echo   ^<range^>CODE %flash_s% %flash_e%^</range^> >> s9070.board
@echo   ^<abs_offset^>0x60020^</abs_offset^>  >> s9070.board
@echo   ^<args^>--s907xMsg^</args^> >> s9070.board
@echo   ^</pass^> >> s9070.board

@echo   ^<pass^> >> s9070.board
@echo   ^<loader^>$PROJ_DIR$\flashloader\s907x.flash^</loader^>    >> s9070.board
@echo   ^<range^>CODE %ram_s% %ram_e%^</range^> >> s9070.board
@echo   ^<abs_offset^>0xb000^</abs_offset^>  >> s9070.board
@echo   ^<args^>--isram^</args^> >> s9070.board
@echo   ^</pass^> >> s9070.board

@echo  ^</flash_board^> >> s9070.board    >> s9070.board


exit

@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
set /a dec=%~1
set "hex="
set "map=0123456789ABCDEF"
for /L %%N in (1,1,8) do (
    set /a "d=dec&15,dec>>=4"
    for %%D in (!d!) do set "hex=!map:~%%D,1!!hex!"
)

( ENDLOCAL & REM RETURN VALUES
    IF "%~2" NEQ "" (SET %~2=%hex%) ELSE ECHO.%hex%
)
EXIT /b
