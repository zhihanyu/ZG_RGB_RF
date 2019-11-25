@echo off
cd /D %2

set tooldir=%2\..\..\script\tools
set outdir =%2

set outputplatform=s907x
set outputdir=Debug\Obj
set ota_bin_ver=0x%date:~0,4%%date:~5,2%%date:~8,2%
set bindir=Debug\Exe

echo %1 %2 %3 %4 > tmp.txt
echo %tooldir% >>tmp.txt
echo %outputdir% >> tmp.txt
echo %outputplatform% >> tmp.txt
echo ota_bin_ver=%ota_bin_ver% >>tmp.txt

IF NOT EXIST %bindir% MD %bindir%
copy %outputdir%\%outputplatform%.elf %bindir%\application.axf > image.txt
del Debug\Exe\application.map Debug\Exe\application.asm Debug\Exe\ota1.bin

cmd /c "%tooldir%\nm Debug\Exe\application.axf > Debug\Exe\application.map"
cmd /c "%tooldir%\objdump -d Debug\Exe\application.axf > Debug\Exe\application.asm"

cmd /c "%tooldir%\grep __ota1_text_start__ .\Debug\Exe\application.map > sections.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' sections.txt"') do @set flash_s=0x%%i
cmd /c "%tooldir%\grep __ota1_text_end__ .\Debug\Exe\application.map > sections.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' sections.txt"') do @set flash_e=0x%%i
cmd /c "%tooldir%\grep system_entry .\Debug\Exe\application.map > sections.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' sections.txt"') do @set ram_s=0x%%i
cmd /c "%tooldir%\grep __bss_start__ .\Debug\Exe\application.map > sections.txt"
for /f "delims=" %%i in ('cmd /c "%tooldir%\gawk '{print $1}' sections.txt"') do @set ram_e=0x%%i

echo ram_s: %ram_s% >> tmp.txt
echo ram_e: %ram_e% >> tmp.txt
echo flash_s: %flash_s% >> tmp.txt
echo flash_e: %flash_e% >> tmp.txt

%tooldir%\objcopy -j .system.entry -j .ota1_ram.data -j .ota1_ram.bss -Obinary .\Debug\Exe\application.axf .\Debug\Exe\ram.bin
%tooldir%\objcopy -j .ota1_image.text -Obinary .\Debug\Exe\application.axf .\Debug\Exe\text.bin
%tooldir%\merge %bindir%/text.bin %bindir%/ram.bin %bindir%/ota1.bin .\tmp.txt > image.txt
%tooldir%\checksum %bindir%/ota1.bin %bindir%/ota1_checksum.bin >> image.txt

echo Building images...
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
echo Finish building, images in out\GCC\Debug\Exe

::%tooldir%\image F ALL .\Debug\Exe\ 
::%tooldir%\image F OTA_WLAN .\Debug\Exe\	
::%tooldir%\image F OTA .\Debug\Exe\


