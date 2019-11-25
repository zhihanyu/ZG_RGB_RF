cd /D %2


set linkdir=%2\..\..\script\links\IAR
set tooldir=%2\..\..\script\tools
set outdir =%2
set platform=%2

set ota_bin_ver=0x%date:~0,4%%date:~5,2%%date:~8,2%

 

echo %tooldir% > tmp.txt
echo %outputplatform% >> tmp.txt
echo ota_bin_ver=%ota_bin_ver% >> tmp.txt
 

 
del Debug\Exe\application.map %Debug\Exe\application.asm *.bin

cmd /c "%tooldir%\nm Debug/Exe/application.axf > %Debug/Exe/application.map"

cmd /c "%tooldir%\objdump -d Debug/Exe/application.axf > Debug/Exe/application.asm"


for /f "delims=" %%i in ('cmd /c "%tooldir%\grep ota1_ram Debug/Exe/application.map | %tooldir%\grep Base | %tooldir%\gawk '{print $1}'"') do set ram_s=0x%%i
for /f "delims=" %%i in ('cmd /c "%tooldir%\grep ota1_ram Debug/Exe/application.map | %tooldir%\grep Limit | %tooldir%\gawk '{print $1}'"') do (
set ram_e=0x%%i 
)

for /f "delims=" %%i in ('cmd /c "%tooldir%\grep ota1_image Debug/Exe/application.map | %tooldir%\grep Base | %tooldir%\gawk '{print $1}'"') do set flash_s=0x%%i
for /f "delims=" %%i in ('cmd /c "%tooldir%\grep ota1_image Debug/Exe/application.map | %tooldir%\grep Limit | %tooldir%\gawk '{print $1}'"') do ( 
set flash_e=0x%%i 
)

echo flash_s: %flash_s% >> tmp.txt
echo flash_e: %flash_e% >> tmp.txt
echo ram_s: %ram_s% >> tmp.txt
echo ram_e: %ram_e% >> tmp.txt


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
%tooldir%\objcopy -j "%sec_ram% rw" -Obinary Debug/Exe/application.axf Debug/Exe/ram.bin
%tooldir%\objcopy -j "%sec_text% rw" -Obinary Debug/Exe/application.axf Debug/Exe/text.bin

%tooldir%\merge %2/Debug/Exe/text.bin %2/Debug/Exe/ram.bin %2/Debug/Exe/ota1.bin %2\tmp.txt
%tooldir%\checksum %2/Debug/Exe/ota1.bin %2/Debug/Exe/ota1_checksum.bin

set build_type=4

if "%build_type%"=="4" (
set flash_t=F
) else (
set flash_t=T
)

set ota_type=1

if "%ota_type%"=="1" (
set ota_t=1
) else (
set ota_t=2
)

%tooldir%\image %flash_t% ALL .\Debug\Exe\ 
%tooldir%\image %flash_t% OTA%ota_t%_WLAN .\Debug\Exe\
%tooldir%\image %flash_t% OTA%ota_t% .\Debug\Exe\

::s9070.board

@echo ^<?xml version="1.0" encoding="iso-8859-1"?^> > %linkdir%/s9070.board

@echo ^<flash_board^> >> %linkdir%/s9070.board

@echo   ^<pass^> >> %linkdir%/s9070.board
@echo   ^<loader^>$PROJ_DIR$\..\..\..\..\debug\flashloader\s907x.flash^</loader^>    >> %linkdir%/s9070.board
@echo   ^<range^>CODE %flash_s% %flash_e%^</range^> >> %linkdir%/s9070.board
@echo   ^<abs_offset^>0x60020^</abs_offset^>  >> %linkdir%/s9070.board
@echo   ^<args^>--s907xMsg^</args^> >> %linkdir%/s9070.board
@echo   ^</pass^> >> %linkdir%/s9070.board

@echo   ^<pass^> >> %linkdir%/s9070.board
@echo   ^<loader^>$PROJ_DIR$\..\..\..\..\debug\flashloader\s907x.flash^</loader^>    >> %linkdir%/s9070.board
@echo   ^<range^>CODE %ram_s% %ram_e%^</range^> >> %linkdir%/s9070.board
@echo   ^<abs_offset^>0xb000^</abs_offset^>  >> %linkdir%/s9070.board
@echo   ^<args^>--isram^</args^> >> %linkdir%/s9070.board
@echo   ^</pass^> >> %linkdir%/s9070.board

@echo  ^</flash_board^> >> %linkdir%/s9070.board    >> %linkdir%/s9070.board


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