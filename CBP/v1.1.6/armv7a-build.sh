#!/bin/sh

echo "##########################################################"
echo "# SET BUILD VARIABLES"
echo "##########################################################"

OS=armv7a
VERSION=`grep "#define pszLinuxFileVersion" IBScanUltimate/CaptureLib/IBSU_Global_Def.h | tr -d '\n' | tr -d '\r' | sed s/"#define pszLinuxFileVersion[^\"]*\""/""/ | sed s/"\""/""/ | sed s/".0$"/""/`
JAVA_HOME=/usr/lib/jvm/java-6-oracle

CROSS_COMPILE_arm_linux_gnueabi=~/Toolchain/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/arm-none-linux-gnueabi-
CROSS_COMPILE_arm_linux_gnueabihf=~/Toolchain/Linaro/gcc-linaro-arm-linux-gnueabihf-4.8-2013.05_linux/bin/arm-linux-gnueabihf-
CROSS_COMPILE_arm_linux_uclibcgnueabi=~/Toolchain/Atmel-ATSAMA5D3-BuildRoot/buildroot-at91/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-

REMOVE_arm_linux_gnueabi=no
REMOVE_arm_linux_gnueabihf=no
REMOVE_arm_linux_uclibcgnueabi=yes

ABIS=(arm-linux-gnueabi arm-linux-gnueabihf arm-linux-uclibcgnueabi)

echo "##########################################################"
echo "# CREATE OUTPUT DIRECTORIES"
echo "##########################################################"

rm -rf lib
mkdir  lib
for abi in ${ABIS[@]} ; do
    mkdir lib/$abi
done

rm -rf bin
mkdir  bin
for abi in ${ABIS[@]} ; do
    mkdir bin/$abi
done

echo "##########################################################"
echo "# BUILD LIBRARY"
echo "##########################################################"

cd    "IBScanUltimate"

for abi in ${ABIS[@]} ; do
    declare "temp=CROSS_COMPILE_${abi//-/_}"
    export CROSS_COMPILE=${!temp}
    export ARCHABI=$abi
    make   -f Makefile""_$OS.mk clean
    make   -f Makefile""_$OS.mk all
    cp     lib/libIBScanUltimate.so.stripped ../lib/$ARCHABI/libIBScanUltimate.so.stripped
    cp     lib/libIBScanUltimate.so          ../lib/$ARCHABI/libIBScanUltimate.so
done
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

for abi in ${ABIS[@]} ; do
    declare "temp=CROSS_COMPILE_${abi//-/_}"
    export CROSS_COMPILE=${!temp}
    export ARCHABI=$abi
    make   -f Makefile""_$OS.mk clean
    make   -f Makefile""_$OS.mk all
    cp     bin/testScanU ../../../bin/$ARCHABI/testScanU
    cp     ../../../CommonFiles/IBSU_FunctionTesterForJava_Linux.sh ../../../bin/$ARCHABI/IBSU_FunctionTesterForJava.sh
    cp     ../../../CommonFiles/IBScanUltimate_SampleForJava_Linux.sh ../../../bin/$ARCHABI/IBScanUltimate_SampleForJava.sh
done

cd    ../../..

echo "##########################################################"
echo "# BUILD JAVA SAMPLES"
echo "##########################################################"

WHICH_RESULT="`which java`"
if [ $WHICH_RESULT != "" ]
then
    echo "Found 'java' at '$WHICH_RESULT'"
else
    echo "Cannot find 'java'"
    exit 0
fi

cp  -R    "Sample sources/Java" "Samples"

echo "#"
echo "# Build Java wrapper around library"
echo "#"

cd        "Sample sources/Java/IBScanCommon"
rm -rf    bin
mkdir     bin

"$JAVA_HOME/bin/javac" -sourcepath "src" "src/com/integratedbiometrics/ibscancommon/IBCommon.java" -d bin
"$JAVA_HOME/bin/jar" cf "IBScanCommon.jar" -C bin .
if [ -e "IBScanCommon.jar" ]
then
    echo "Found 'IBScanCommon.jar'"
else
    echo "Could not find 'IBScanCommon.jar'"
    exit 0
fi

for abi in ${ABIS[@]} ; do
    cp     IBScanCommon.jar ../../../bin/$abi/
    cp     IBScanCommon.jar ../../../lib/$abi/
done
cp         IBScanCommon.jar ../../../bin/
rm         IBScanCommon.jar
rm   -r -f bin
cd         ../../..  


cd        "Sample sources/Java/IBScanUltimate"
rm -rf    bin
mkdir     bin

"$JAVA_HOME/bin/javac" -sourcepath "src" -classpath "../../../bin/IBScanCommon.jar" \
"src/com/integratedbiometrics/ibscanultimate/IBScan.java" \
"src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java" \
"src/com/integratedbiometrics/ibscanultimate/IBScanListener.java" \
"src/com/integratedbiometrics/ibscanultimate/IBScanDeviceListener.java" \
"src/com/integratedbiometrics/ibscanultimate/IBScanException.java" -d bin
"$JAVA_HOME/bin/jar" cf "IBScanUltimate.jar" -C bin .
if [ -e "IBScanUltimate.jar" ]
then
    echo "Found 'IBScanUltimate.jar'"
else
    echo "Could not find 'IBScanUltimate.jar'"
    exit 0
fi

for abi in ${ABIS[@]} ; do
    cp     IBScanUltimate.jar ../../../bin/$abi/
    cp     IBScanUltimate.jar ../../../lib/$abi/
done
cp         IBScanUltimate.jar ../../../bin/
rm         IBScanUltimate.jar
rm   -r -f bin
cd         ../../..  


echo "#"
echo "# Build FunctionTester sample"
echo "#"

cd        "Sample sources/Java/FunctionTesterForJava"
rm -rf    bin
mkdir     bin
cp  -R    "src/resources" bin

"$JAVA_HOME/bin/javac" -sourcepath "src" -classpath "../../../bin/IBScanCommon.jar:../../../bin/IBScanUltimate.jar" \
"src/com/sample/functiontester/FunctionTester.java" -d bin
"$JAVA_HOME/bin/jar" cfm "FunctionTester.jar" "Manifest.txt" -C bin .
if [ -e "FunctionTester.jar" ]
then
    echo "Found 'FunctionTester.jar'"
else
    echo "Could not find 'FunctionTester.jar'"
    exit 0
fi

for abi in ${ABIS[@]} ; do
    cp     FunctionTester.jar ../../../bin/$abi/
done
rm         FunctionTester.jar
rm   -r -f bin
cd         ../../..  


echo "#"
echo "# Build IBScanUltimate_SampleForJava sample"
echo "#"

cd        "Sample sources/Java/IBScanUltimate_SampleForJava"
rm -rf    bin
mkdir     bin
cp  -R    "src/resources" bin

"$JAVA_HOME/bin/javac" -sourcepath "src" -classpath "../../../bin/IBScanCommon.jar:../../../bin/IBScanUltimate.jar" \
"src/com/sample/IBScanUltimate_SampleforJava/IBScanUltimate_Sample.java" -d bin
"$JAVA_HOME/bin/jar" cfm "IBScanUltimate_SampleForJava.jar" "Manifest.txt" -C bin .
if [ -e "IBScanUltimate_SampleForJava.jar" ]
then
    echo "Found 'IBScanUltimate_SampleForJava.jar'"
else
    echo "Could not find 'IBScanUltimate_SampleForJava.jar'"
    exit 0
fi

for abi in ${ABIS[@]} ; do
    cp     IBScanUltimate_SampleForJava.jar ../../../bin/$abi/
done
rm         IBScanUltimate_SampleForJava.jar
rm   -r -f bin
cd         ../../..  


echo "#"
echo "# Build JNI wrapper around library"
echo "#"

cd        "Sample sources/Java/IBScanUltimateJNI"
for abi in ${ABIS[@]} ; do
    declare "temp=CROSS_COMPILE_${abi//-/_}"
    export CROSS_COMPILE=${!temp}
    export ARCHABI=$abi
    make   -f Makefile""_$OS.mk clean
    make   -f Makefile""_$OS.mk all
    cp     lib/libIBScanUltimateJNI.so.stripped ../../../lib/$ARCHABI/libIBScanUltimateJNI.so.stripped
    cp     lib/libIBScanUltimateJNI.so          ../../../lib/$ARCHABI/libIBScanUltimateJNI.so
done

cd         ../../..  


echo "##########################################################"
echo "# CREATE OUTPUT PACKAGE"
echo "##########################################################"

OUTFOLDER="IBScanUltimate_$OS""_$VERSION"
PKG_UCLIBC="IBScanUltimate_$OS""_uclibc_$VERSION"

rm   -f     $OUTFOLDER".tgz"
rm   -f     $OUTFOLDER".full.tgz"
rm   -f     $PKG_UCLIBC".tgz"
rm   -r -f  $OUTFOLDER
rm   -r -f  install/*.so
rm -f       bin/IBScanCommon.jar
rm -f       bin/IBScanUltimate.jar

mkdir       $OUTFOLDER
cp -R       bin     $OUTFOLDER
cp -R       include $OUTFOLDER
cp -R       install $OUTFOLDER
cp -R       lib     $OUTFOLDER
mkdir -p    $OUTFOLDER/Samples/Linux/TestScan
mkdir -p    $OUTFOLDER"/Samples/Java/IBScanUltimateJNI"
mkdir -p    $OUTFOLDER"/Samples/Python"

cp          "Samples/Linux/TestScan/Makefile_$OS.mk" $OUTFOLDER/Samples/Linux/TestScan/Makefile
cp          "Samples/Linux/TestScan/testScanU.cpp"   $OUTFOLDER/Samples/Linux/TestScan/testScanU.cpp

cp  -R      "Samples/Java/FunctionTesterForJava" $OUTFOLDER"/Samples/Java/"
cp  -R      "Samples/Java/IBScanUltimate_SampleForJava" $OUTFOLDER"/Samples/Java/"
cp  -R      "Samples/Java/IBScanCommon" $OUTFOLDER"/Samples/Java/"
cp  -R      "Samples/Java/IBScanUltimate" $OUTFOLDER"/Samples/Java/"
cp          "Samples/Java/IBScanUltimateJNI/Makefile_$OS.mk"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/Makefile"
cp          "Samples/Java/IBScanUltimateJNI/IBScan.c"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScan.c"
cp          "Samples/Java/IBScanUltimateJNI/IBScanDevice.c"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScanDevice.c"
cp          "Samples/Java/IBScanUltimateJNI/IBScan.h"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScan.h"
cp          "Samples/Java/IBScanUltimateJNI/IBScanDevice.h"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScanDevice.h"

cp  -R      "Sample sources/Python/TestScanForPython" $OUTFOLDER"/Samples/Python/"
cp  -R      "Sample sources/Python/IBScanUltimate_SampleForPySide" $OUTFOLDER"/Samples/Python/"

rm -f       $OUTFOLDER/install/install-IBScanUltimatePPI

tar --exclude-vcs -cvpzf $OUTFOLDER".full.tgz" $OUTFOLDER

cp -R       $OUTFOLDER tmp

for abi in ${ABIS[@]} ; do
    rm -f $OUTFOLDER/lib/$abi/libIBScanUltimate.so
    mv    $OUTFOLDER/lib/$abi/libIBScanUltimate.so.stripped $OUTFOLDER/lib/$abi/libIBScanUltimate.so
    rm -f $OUTFOLDER/lib/$abi/libIBScanUltimateJNI.so
    mv    $OUTFOLDER/lib/$abi/libIBScanUltimateJNI.so.stripped $OUTFOLDER/lib/$abi/libIBScanUltimateJNI.so

    declare "temp=REMOVE_${abi//-/_}"
    doremove=${!temp}
    if [ "$doremove" != "no" ]; then
        rm -rf $OUTFOLDER/lib/$abi
        rm -rf $OUTFOLDER/bin/$abi
    fi
done

tar --exclude-vcs -cvpzf $OUTFOLDER".tgz" $OUTFOLDER

rm -rf $OUTFOLDER
mv     tmp $OUTFOLDER
for abi in ${ABIS[@]} ; do
    rm -f $OUTFOLDER/lib/$abi/libIBScanUltimate.so
    mv    $OUTFOLDER/lib/$abi/libIBScanUltimate.so.stripped $OUTFOLDER/lib/$abi/libIBScanUltimate.so
    rm -f $OUTFOLDER/lib/$abi/libIBScanUltimateJNI.so
    mv    $OUTFOLDER/lib/$abi/libIBScanUltimateJNI.so.stripped $OUTFOLDER/lib/$abi/libIBScanUltimateJNI.so

    declare "temp=REMOVE_${abi//-/_}"
    doremove=${!temp}
    if [ "$doremove" != "yes" ]; then
        rm -rf $OUTFOLDER/lib/$abi
        rm -rf $OUTFOLDER/bin/$abi
    fi
done

tar --exclude-vcs -cvpzf $PKG_UCLIBC".tgz" $OUTFOLDER

echo "##########################################################"
echo "# CLEAN UP"
echo "##########################################################"

rm -rf $OUTFOLDER
rm -rf bin
rm -rf lib
rm -rf install/*.so
rm -rf IBScanUltimate/lib
rm -rf IBScanUltimate/obj
rm -rf Samples/Linux/TestScan/bin
rm -rf Samples/Linux/TestScan/obj
rm -rf Samples/Java

echo "Build complete"
echo ""

