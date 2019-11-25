

@echo off

set cur_dir=%cd%

if not exist build\compiler\Win32 (
	@echo "auto download compiler from https://code.aliyun.com/gcc_group_public/gcc-arm-none-eabi-win32.git"
	cmd /c git clone https://code.aliyun.com/gcc_group_public/gcc-arm-none-eabi-win32.git ./build/compiler/Win32
	move build\compiler\Win32\main\bin build\compiler\Win32\bin
	move build\compiler\Win32\main\lib build\compiler\Win32\lib
	move build\compiler\Win32\main\share build\compiler\Win32\share
	move build\compiler\Win32\main\arm-none-eabi build\compiler\Win32\arm-none-eabi
	rd /s /q build\compiler\Win32\main\
	rd /s /q build\compiler\Win32\.git\
	del /a /q /f build\compiler\Win32\.gitignore
	del /a /q /f build\compiler\Win32\readme.txt
	del /a /q /f build\compiler\Win32\release.txt
	del /a /q /f build\compiler\Win32\license.txt	
)

if "%1" == "clean" (
	@echo "Clean in windows enviroment."
    cmd /c %cur_dir%\build\cmd\win32\make.exe clean -f windows.mk
) else (
    @echo "Compile in windows enviroment."
    cmd /c %cur_dir%\build\cmd\win32\make.exe -f windows.mk
)

