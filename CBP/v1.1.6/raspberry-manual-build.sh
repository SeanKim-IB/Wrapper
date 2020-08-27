#!/bin/sh

echo "##########################################################"
echo "# SET BUILD VARIABLES"
echo "##########################################################"

OS=armv6
VERSION=`grep "#define pszLinuxFileVersion" IBScanUltimate/CaptureLib/IBSU_Global_Def.h | tr -d '\n' | tr -d '\r' | sed s/"#define pszLinuxFileVersion[^\"]*\""/""/ | sed s/"\""/""/`

CROSS_COMPILE_arm_linux_rpi_gnueabihf=
abi=arm-linux-rpi-gnueabihf

echo "##########################################################"
echo "# CREATE OUTPUT DIRECTORIES"
echo "##########################################################"

rm -rf lib
mkdir  lib
mkdir lib/$abi

rm -rf bin
mkdir  bin
mkdir bin/$abi

echo "##########################################################"
echo "# BUILD LIBRARY"
echo "##########################################################"

cd    "IBScanUltimate"

export CROSS_COMPILE=$CROSS_COMPILE_arm_linux_rpi_gnueabihf
export ARCHABI=$abi
make   -f Makefile""_$OS""_rpi.mk clean
make   -f Makefile""_$OS""_rpi.mk all
cp     lib/libIBScanUltimate.so.stripped ../lib/$ARCHABI/libIBScanUltimate.so.stripped
cp     lib/libIBScanUltimate.so          ../lib/$ARCHABI/libIBScanUltimate.so

if [ -e "/usr/lib/libIBScanUltimate.so" ]
then
    echo "Found 'libIBScanUltimate.so' from /usr/lib folder"
else
    sudo cp   IBScanUltimate/lib/libIBScanUltimate.so.stripped /usr/lib/libIBScanUltimate.so
fi

cd     ..

echo "##########################################################"
echo "# BUILD TEST APPLICATION"
echo "##########################################################"

cd    "Samples/Linux/TestScan"

export CROSS_COMPILE=$CROSS_COMPILE_arm_linux_rpi_gnueabihf
export ARCHABI=$abi
make   -f Makefile""_$OS""_rpi.mk clean
make   -f Makefile""_$OS""_rpi.mk all
cp     bin/testScanU ../../../bin/$ARCHABI/testScanU

cd    ../../..


echo "##########################################################"
echo "# CREATE OUTPUT PACKAGE"
echo "##########################################################"

OUTFOLDER="IBScanUltimate_$OS""_$VERSION"
PKG_RPI="IBScanUltimate_$OS""_RaspberryPi_$VERSION"

rm   -f     $PKG_RPI".tar.gz"
rm   -r -f  $OUTFOLDER
rm   -r -f  install/*.so

mkdir       $OUTFOLDER
cp -R       bin     $OUTFOLDER
cp -R       include $OUTFOLDER
cp -R       install $OUTFOLDER
cp -R       lib     $OUTFOLDER
mkdir -p    $OUTFOLDER/Samples/Linux/TestScan
mkdir -p    $OUTFOLDER"/Samples/Python"

cp          "Samples/Linux/TestScan/Makefile_$OS""_rpi.mk" $OUTFOLDER/Samples/Linux/TestScan/Makefile
cp          "Samples/Linux/TestScan/testScanU.cpp"   $OUTFOLDER/Samples/Linux/TestScan/testScanU.cpp

cp  -R      "Sample sources/Python/TestScanForPython" $OUTFOLDER"/Samples/Python/"
cp  -R      "Sample sources/Python/IBScanUltimate_SampleForPySide" $OUTFOLDER"/Samples/Python/"

rm -f       $OUTFOLDER/install/install-IBScanUltimatePPI

cp -R       $OUTFOLDER tmp

rm -f $OUTFOLDER/lib/$abi/libIBScanUltimate.so
mv    $OUTFOLDER/lib/$abi/libIBScanUltimate.so.stripped $OUTFOLDER/lib/$abi/libIBScanUltimate.so

rm -rf $OUTFOLDER
mv     tmp $OUTFOLDER
rm -f $OUTFOLDER/lib/$abi/libIBScanUltimate.so
mv    $OUTFOLDER/lib/$abi/libIBScanUltimate.so.stripped $OUTFOLDER/lib/$abi/libIBScanUltimate.so

#Need to make compress using Raspberry OS (SHIT!!!!)
#tar -cvf $PKG_RPI".tar.gz" $OUTFOLDER

echo "##########################################################"
echo "# CLEAN UP"
echo "##########################################################"

#rm -rf $OUTFOLDER
rm -rf bin
rm -rf lib
rm -rf install/*.so
rm -rf IBScanUltimate/lib
rm -rf IBScanUltimate/obj
rm -rf Samples/Linux/TestScan/bin
rm -rf Samples/Linux/TestScan/obj

echo "Build complete"
echo ""

