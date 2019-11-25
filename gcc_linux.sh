cur_dir=.

if [ ! -d "./build/compiler/Linux64/" ];then
	echo "auto download compiler from https://code.aliyun.com/gcc_group_public/gcc-arm-none-eabi-linux.git"
	sudo git clone https://code.aliyun.com/gcc_group_public/gcc-arm-none-eabi-linux.git ./build/compiler/Linux64
	sudo mv build/compiler/Linux64/main/bin/ build/compiler/Linux64/bin/
	sudo mv build/compiler/Linux64/main/lib/ build/compiler/Linux64/lib/
	sudo mv build/compiler/Linux64/main/share/ build/compiler/Linux64/share/
	sudo mv build/compiler/Linux64/main/arm-none-eabi/ build/compiler/Linux64/arm-none-eabi/
	sudo rm -rf build/compiler/Linux64/main/
	sudo rm -rf build/compiler/Linux64/.git/
	sudo rm -f build/compiler/Linux64/.gitignore
	sudo rm -f build/compiler/Linux64/readme.txt
	sudo rm -f build/compiler/Linux64/release.txt
	sudo rm -f build/compiler/Linux64/license.txt
fi

if [ "$1" = "clean" ];then
	echo "Clean in linux enviroment."
	sudo $cur_dir/build/cmd/linux64/make clean -f linux.mk
else
    echo "Compile in linux enviroment."
    sudo $cur_dir/build/cmd/linux64/make -f linux.mk
fi
