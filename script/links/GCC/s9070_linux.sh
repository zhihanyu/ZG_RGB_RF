#!/bin/bash

cd $2

tooldir=$2/../../script/tools
outdir=$2
tim=$(date "+%Y%m%d")
outputplatform=s907x
outputdir=Debug/Obj
ota_bin_ver="0x$tim"
bindir=Debug/Exe
cmd=$2/../../build/compiler/Linux64/bin/arm-none-eabi
OBJDUMP=$cmd-objdump
OBJCOPY=$cmd-objcopy
NM=$cmd-nm


echo $ota_bin_ver

echo $1 $2 > tmp.txt
echo $tooldir >> tmp.txt
echo $outputdir >> tmp.txt
echo $outputplatform >> tmp.txt
echo ota_bin_ver=$ota_bin_ver >>tmp.txt

cp $outputdir/$outputplatform.elf $bindir/application.axf
rm -f Debug/Exe/application.map Debug/Exe/application.asm Debug/Exe/ota1.bin

$NM Debug/Exe/application.axf > Debug/Exe/application.map
$OBJDUMP -d Debug/Exe/application.axf > Debug/Exe/application.asm

tmp_val=`grep "ota1_text_start" -rn "Debug/Exe/application.map"`
line_num=`echo $tmp_val | awk '{print $1}'`
flash_s="0x`echo $line_num | awk -F: '{print $2}'`"

tmp_val=`grep "ota1_text_end" -rn "Debug/Exe/application.map"`
line_num=`echo $tmp_val | awk '{print $1}'`
flash_e="0x`echo $line_num | awk -F: '{print $2}'`"

tmp_val=`grep "system_entry" -rn "Debug/Exe/application.map"`
line_num=`echo $tmp_val | awk '{print $1}'`
ram_s="0x`echo $line_num | awk -F: '{print $2}'`"

tmp_val=`grep "__bss_start__" -rn "Debug/Exe/application.map"`
line_num=`echo $tmp_val | awk '{print $1}'`
ram_e="0x`echo $line_num | awk -F: '{print $2}'`"

echo ram_s: $ram_s >> tmp.txt
echo ram_e: $ram_e >> tmp.txt
echo flash_s: $flash_s >> tmp.txt
echo flash_e: $flash_e >> tmp.txt

$OBJCOPY -j .system.entry -j .ota1_ram.data -j .ota1_ram.bss -Obinary ./Debug/Exe/application.axf ./Debug/Exe/ram.bin
$OBJCOPY -j .ota1_image.text -Obinary ./Debug/Exe/application.axf ./Debug/Exe/text.bin

$tooldir/merge $bindir/text.bin $bindir/ram.bin $bindir/ota1.bin ./tmp.txt
$tooldir/checksum $bindir/ota1.bin $bindir/ota1_checksum.bin

$tooldir/image F ALL ./Debug/Exe/
$tooldir/image F OTA1_WLAN ./Debug/Exe/
$tooldir/image F OTA1 ./Debug/Exe/
