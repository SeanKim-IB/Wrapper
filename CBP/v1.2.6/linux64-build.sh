#!/bin/sh

echo "##########################################################"
echo " SET VERSION NUMBER"
echo "##########################################################"

OS=x64
VERSION=`grep "#define pszLinuxFileVersion" IBScanUltimate/CaptureLib/IBSU_Global_Def.h | tr -d '\n' | tr -d '\r' | sed s/"#define pszLinuxFileVersion[^\"]*\""/""/ | sed s/"\""/""/ | sed s/".0$"/""/`
JAVA_HOME=/usr/lib/jvm/java-6-oracle
QT_HOME=/usr/local/QT/4.8.6-static

echo "##########################################################"
echo "# CREATE OUTPUT DIRECTORIES"
echo "##########################################################"

rm -rf lib
mkdir  lib

rm -rf bin
mkdir  bin

echo "##########################################################"
echo "# BUILD LIBRARY"
echo "##########################################################"

cd    "IBScanUltimate"
make  -f Makefile""_$OS.mk clean
make  -f Makefile""_$OS.mk all
cd    ..

cp    IBScanUltimate/lib/libIBScanUltimate.so.stripped install/libIBScanUltimate.so
cp    IBScanUltimate/lib/libIBScanUltimate.so.stripped lib/libIBScanUltimate.so
if [ -e "/usr/lib/libIBScanUltimate.so" ]
then
    echo "Found 'libIBScanUltimate.so' from /usr/lib folder"
    sudo cp   IBScanUltimate/lib/libIBScanUltimate.so.stripped /usr/lib/libIBScanUltimate.so
else
    sudo cp   IBScanUltimate/lib/libIBScanUltimate.so.stripped /usr/lib/libIBScanUltimate.so
fi

echo "##########################################################"
echo "# BUILD TEST APPLICATION"
echo "##########################################################"

cd    "Samples/Linux/TestScan"
make  -f Makefile""_$OS.mk clean
make  -f Makefile""_$OS.mk all
cd    ../../..
if [ -e "Samples/Linux/TestScan/bin/testScanU" ]
then
    cp Samples/Linux/TestScan/bin/testScanU bin/
    cp CommonFiles/IBSU_FunctionTesterForJava_Linux.sh bin/IBSU_FunctionTesterForJava.sh
    cp CommonFiles/IBScanUltimate_SampleForJava_Linux.sh bin/IBScanUltimate_SampleForJava.sh
else
    echo "Could not find 'testScanU'"
    exit 0
fi

echo "##########################################################"
echo "# BUILD QT SAMPLES"
echo "##########################################################"

cp  -R    "Sample sources/QT/IBScanUltimate_SampleForQT" "Samples/Linux/"
cp  -f    CommonFiles/IBScanUltimate_SampleForQT_Linux.pro "Samples/Linux/IBScanUltimate_SampleForQT/IBScanUltimate_SampleForQT.pro"

cd    "Samples/Linux/IBScanUltimate_SampleForQT"
rm -rf    bin
mkdir     bin

"$QT_HOME/bin/qmake" IBScanUltimate_SampleForQT.pro
make  -f Makefile
cd    ../../..
if [ -e "Samples/Linux/IBScanUltimate_SampleForQT/bin/IBScanUltimate_SampleForQT" ]
then
    cp Samples/Linux/IBScanUltimate_SampleForQT/bin/IBScanUltimate_SampleForQT bin/
else
    echo "Could not find 'IBScanUltimate_SampleForQT'"
    exit 0
fi


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

cp         IBScanCommon.jar ../../../bin/
cp         IBScanCommon.jar ../../../lib/
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

cp         IBScanUltimate.jar ../../../bin/
cp         IBScanUltimate.jar ../../../lib/
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

cp         FunctionTester.jar ../../../bin/
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

cp         IBScanUltimate_SampleForJava.jar ../../../bin/
rm         IBScanUltimate_SampleForJava.jar
rm   -r -f bin
cd         ../../..  


echo "#"
echo "# Build JNI wrapper around library"
echo "#"

cd        "Sample sources/Java/IBScanUltimateJNI"
make      -f Makefile""_$OS.mk clean
make      -f Makefile""_$OS.mk all

cp        lib/libIBScanUltimateJNI.so.stripped ../../../install/libIBScanUltimateJNI.so
cp        lib/libIBScanUltimateJNI.so.stripped ../../../lib/libIBScanUltimateJNI.so
rm -rf    lib
rm -rf    obj
cd         ../../..  


echo "##########################################################"
echo "# CREATE OUTPUT PACKAGE"
echo "##########################################################"

OUTFOLDER="IBScanUltimate_$OS""_$VERSION"

rm   -f     $OUTFOLDER".tgz"
rm   -r -f  $OUTFOLDER

mkdir       $OUTFOLDER
cp -R       bin $OUTFOLDER
cp -R       lib $OUTFOLDER
cp -R       install $OUTFOLDER
cp -R       include $OUTFOLDER
mkdir -p    $OUTFOLDER"/Samples/Linux/TestScan"
mkdir -p    $OUTFOLDER"/Samples/Java/IBScanUltimateJNI"
mkdir -p    $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT"
mkdir -p    $OUTFOLDER"/Samples/Python"

cp          "Samples/Linux/TestScan/Makefile_$OS.mk" $OUTFOLDER"/Samples/Linux/TestScan/Makefile"
cp          "Samples/Linux/TestScan/testScanU.cpp"   $OUTFOLDER"/Samples/Linux/TestScan/testScanU.cpp"

cp  -R      "Samples/Java/FunctionTesterForJava" $OUTFOLDER"/Samples/Java/"
cp  -R      "Samples/Java/IBScanUltimate_SampleForJava" $OUTFOLDER"/Samples/Java/"
cp  -R      "Samples/Java/IBScanCommon" $OUTFOLDER"/Samples/Java/"
cp  -R      "Samples/Java/IBScanUltimate" $OUTFOLDER"/Samples/Java/"
cp          "Samples/Java/IBScanUltimateJNI/Makefile_$OS.mk"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/Makefile"
cp          "Samples/Java/IBScanUltimateJNI/IBScan.c"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScan.c"
cp          "Samples/Java/IBScanUltimateJNI/IBScanDevice.c"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScanDevice.c"
cp          "Samples/Java/IBScanUltimateJNI/IBScan.h"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScan.h"
cp          "Samples/Java/IBScanUltimateJNI/IBScanDevice.h"   $OUTFOLDER"/Samples/Java/IBScanUltimateJNI/IBScanDevice.h"

cp          "Samples/Linux/IBScanUltimate_SampleForQT/IB_logo.bmp"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/IB_logo.bmp"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/Scanner.bmp"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/Scanner.bmp"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/IBScanUltimate_SampleForQT.pro"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/IBScanUltimate_SampleForQT.pro"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/IBSU_SampleForQT.cpp"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/IBSU_SampleForQT.cpp"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/IBSU_SampleForQT.h"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/IBSU_SampleForQT.h"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/IBSU_SampleForQT.ui"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/IBSU_SampleForQT.ui"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/Images.qrc"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/Images.qrc"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/main.cpp"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/main.cpp"
cp          "Samples/Linux/IBScanUltimate_SampleForQT/TypeReDef.h"   $OUTFOLDER"/Samples/Linux/IBScanUltimate_SampleForQT/TypeReDef.h"

cp  -R      "Sample sources/Python/TestScanForPython" $OUTFOLDER"/Samples/Python/"
cp  -R      "Sample sources/Python/IBScanUltimate_SampleForPySide" $OUTFOLDER"/Samples/Python/"

rm -f       $OUTFOLDER/install/install-IBScanUltimatePPI

tar --exclude-vcs -cvpzf $OUTFOLDER".tgz" $OUTFOLDER

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
rm -rf Samples/Linux/IBScanUltimate_SampleForQT

echo "Build complete"
echo ""

