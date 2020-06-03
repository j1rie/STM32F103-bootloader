#! /bin/bash
mkdir ./binaries &>/dev/null
make clean
make -j3 Platform=MapleMini
cp usbdfu.bin ./binaries/boot.MapleMini.bin
make clean
make -j3 Platform=MapleMini_2k
cp usbdfu.bin ./binaries/boot.MapleMini_2k.bin
make clean
make -j3 Platform=Dev
cp usbdfu.bin ./binaries/boot.Dev.bin
make clean
make -j3 Platform=blueDev
cp usbdfu.bin ./binaries/boot.blueDev.bin
make clean
make -j3 Platform=blackDev
cp usbdfu.bin ./binaries/boot.blackDev.bin
make clean
make -j3 Platform=Red
cp usbdfu.bin ./binaries/boot.Red.bin
make clean
make -j3 Platform=Red_noHW
cp usbdfu.bin ./binaries/boot.Red_noHW.bin
make clean
make -j3 Platform=Blue
cp usbdfu.bin ./binaries/boot.Blue.bin
make clean
make -j3 Platform=Blue_noHW
cp usbdfu.bin ./binaries/boot.Blue_noHW.bin
make clean
