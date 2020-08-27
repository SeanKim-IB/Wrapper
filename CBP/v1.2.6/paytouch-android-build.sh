#!/bin/bash

echo "################################################################################"
echo "#"
echo "# BUILD SCRIPT FOR PayTouch ANDROID IBScanUltimate SDK"
echo "#"
echo "################################################################################"

echo "#########################################################################"
echo "# Set up build"
echo "#########################################################################"

NDK_PATH=~/adt/ndk
SDK_PATH=~/adt/sdk
PATH=$PATH:$NDK_PATH:$SDK_PATH/tools:$SDK_PATH/platform-tools
REPO_FOLDER=`pwd`
BUILD_FOLDER=`pwd`
MY_KEYSTORE=`pwd`/temp-release-key.keystore
VERSION=`grep "#define pszLinuxFileVersion" $REPO_FOLDER/IBScanUltimate/CaptureLib/IBSU_Global_Def.h | tr -d '\n' | tr -d '\r' | sed s/"#define pszLinuxFileVersion[^\"]*\""/""/ | sed s/"\""/""/ | sed s/".0$"/""/`
RELEASE_NAME=IBScanUltimate_Android_Paytouch_$VERSION

echo ""

echo "#########################################################################"
echo "# Test build environment"
echo "#########################################################################"

WHICH_RESULT="`which ant`"
if [ $WHICH_RESULT != "" ]
then
    echo "Found 'ant' at '$WHICH_RESULT'"
else
    echo "Cannot find 'ant'"
    exit 0
fi

WHICH_RESULT="`which android`"
if [ $WHICH_RESULT != "" ]
then
    echo "Found 'android' at '$WHICH_RESULT'"
else
    echo "Cannot find 'android'"
    exit 0
fi

WHICH_RESULT="`which ndk-build`"
if [ $WHICH_RESULT != "" ]
then
    echo "Found 'ndk-build' at '$WHICH_RESULT'"
else
    echo "Cannot find 'ndk-build'"
    exit 0
fi

WHICH_RESULT="`which jarsigner`"
if [ $WHICH_RESULT != "" ]
then
    echo "Found 'jarsigner' at '$WHICH_RESULT'"
else
    echo "Cannot find 'jarsigner'"
    exit 0
fi

WHICH_RESULT="`which zipalign`"
if [ $WHICH_RESULT != "" ]
then
    echo "Found 'zipalign' at '$WHICH_RESULT'"
else
    echo "Cannot find 'zipalign'"
    exit 0
fi

echo ""

echo "#########################################################################"
echo "# Transfer files for build"
echo "#########################################################################"

rm   -f     $RELEASE_NAME".tar.gz"

TEMP_FOLDER=`pwd`/tmp
if [ -d "$TEMP_FOLDER" ]
then
    echo "Deleting existing tmp build folder"
    rm -rf "$TEMP_FOLDER"
fi
echo "Creating temporary build folder"
mkdir "$TEMP_FOLDER"
mkdir -p "$TEMP_FOLDER/Samples/Android"

RELEASE_FOLDER=`pwd`/$RELEASE_NAME
if [ -d "$RELEASE_FOLDER" ]
then
    echo "Deleting existing release folder"
    rm -rf "$RELEASE_FOLDER"
fi

echo  "Creating release folders"
mkdir "$RELEASE_FOLDER"
RELEASE_BIN=$RELEASE_FOLDER/bin
mkdir "$RELEASE_BIN"
RELEASE_LIB=$RELEASE_FOLDER/lib
mkdir "$RELEASE_LIB"
RELEASE_SRC=$RELEASE_FOLDER/Samples/Android
mkdir -p "$RELEASE_SRC"

echo  "Transferring IBScanUltimate sample"
cp -R "$REPO_FOLDER/Samples/Android/IBScanUltimate"              "$TEMP_FOLDER/Samples/Android"
cp -a "$REPO_FOLDER/Samples/Android/IBScanUltimate/.classpath"   "$TEMP_FOLDER/Samples/Android/IBScanUltimate"
cp -a "$REPO_FOLDER/Samples/Android/IBScanUltimate/.cproject"    "$TEMP_FOLDER/Samples/Android/IBScanUltimate"
cp -a "$REPO_FOLDER/Samples/Android/IBScanUltimate/.project"     "$TEMP_FOLDER/Samples/Android/IBScanUltimate"

echo  "Transferring IBScanCommon sample"
cp -R "$REPO_FOLDER/Samples/Android/IBScanCommon"              "$TEMP_FOLDER/Samples/Android"
cp -a "$REPO_FOLDER/Samples/Android/IBScanCommon/.classpath"   "$TEMP_FOLDER/Samples/Android/IBScanCommon"
cp -a "$REPO_FOLDER/Samples/Android/IBScanCommon/.project"     "$TEMP_FOLDER/Samples/Android/IBScanCommon"

echo  "Transferring LibUsb1.0"
cp -R "$REPO_FOLDER/Samples/Android/Rooting/LibUsb1.0"                      "$TEMP_FOLDER/Samples/Android"
cp -a "$REPO_FOLDER/Samples/Android/Rooting/LibUsb1.0/.classpath"           "$TEMP_FOLDER/Samples/Android/LibUsb1.0"
cp -a "$REPO_FOLDER/Samples/Android/Rooting/LibUsb1.0/.cproject"            "$TEMP_FOLDER/Samples/Android/LibUsb1.0"
cp -a "$REPO_FOLDER/Samples/Android/Rooting/LibUsb1.0/.project"             "$TEMP_FOLDER/Samples/Android/LibUsb1.0"

echo  "Transferring SimpleScanSample"
cp -R "$REPO_FOLDER/Samples/Android/Rooting/Paytouch/SimpleScanSample"            "$TEMP_FOLDER/Samples/Android"
cp -a "$REPO_FOLDER/Samples/Android/Rooting/Paytouch/SimpleScanSample/.classpath" "$TEMP_FOLDER/Samples/Android/SimpleScanSample"
cp -a "$REPO_FOLDER/Samples/Android/Rooting/Paytouch/SimpleScanSample/.project"   "$TEMP_FOLDER/Samples/Android/SimpleScanSample"

echo  "Transferring TestScan sample"
cp -R "$REPO_FOLDER/Samples/Android/Rooting/Paytouch/TestScan"                    "$TEMP_FOLDER/Samples/Android"

echo "Transferring other files"
cp -R "$REPO_FOLDER/include"                                     "$TEMP_FOLDER/include"
cp -R "$REPO_FOLDER/IBScanUltimate"                              "$TEMP_FOLDER/IBScanUltimate"
 
echo "Cleaning unwanted files"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb1.0/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb1.0/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb1.0/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb1.0/obj"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanUltimate/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanUltimate/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanUltimate/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanUltimate/obj"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanCommon/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanCommon/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanCommon/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/IBScanCommon/obj"
rm -rf "$TEMP_FOLDER/Samples/Android/SimpleScanSample/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/SimpleScanSample/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/SimpleScanSample/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/SimpleScanSample/obj"
rm -rf "$TEMP_FOLDER/Samples/Android/TestScan/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/TestScan/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/TestScan/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/TestScan/obj"

echo ""

echo "#########################################################################"
echo "# Build LibUsb1.0 library"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/LibUsb1.0"

echo "Building the native library..."
ndk-build V=1 NDK_DEBUG=0

echo "Updating the project files..."
android update project --path . --name LibUsb1.0

echo "Building the Android Java library..."
ant -Denv.ANDROID_HOME="$SDK_PATH" -verbose release

echo "Check build..."
if [ -e "bin/classes.jar" ]
then
    echo "Found 'bin/classes.jar'"
else
    echo "Could not find 'bin/classes.jar'"
    exit 0
fi
if [ -e "libs/armeabi/libusb1.0.so" ]
then
    echo "Found 'libs/armeabi/libusb1.0.so'"
else
    echo "Could not find 'libs/armeabi/libusb1.0.so'"
    exit 0
fi
if [ -e "libs/armeabi-v7a/libusb1.0.so" ]
then
    echo "Found 'libs/armeabi-v7a/libusb1.0.so'"
else
    echo "Could not find 'libs/armeabi-v7a/libusb1.0.so'"
    exit 0
fi
if [ -e "libs/x86/libusb1.0.so" ]
then
    echo "Found 'libs/x86/libusb1.0.so'"
else
    echo "Could not find 'libs/x86/libusb1.0.so'"
    exit 0
fi

echo  "Moving binary files to output folder..."
mkdir "$RELEASE_LIB/libusb1.0"
mkdir "$RELEASE_LIB/libusb1.0/armeabi"
mkdir "$RELEASE_LIB/libusb1.0/armeabi-v7a"
mkdir "$RELEASE_LIB/libusb1.0/arm64-v8a"
mkdir "$RELEASE_LIB/libusb1.0/x86"
mkdir "$RELEASE_LIB/libusb1.0/x86_64"
cp    "bin/classes.jar"            "$RELEASE_LIB/libusb1.0/libusb.jar"
cp    "libs/armeabi/libusb1.0.so"     "$RELEASE_LIB/libusb1.0/armeabi/libusb1.0.so"
cp    "libs/armeabi-v7a/libusb1.0.so" "$RELEASE_LIB/libusb1.0/armeabi-v7a/libusb1.0.so"
cp    "libs/arm64-v8a/libusb1.0.so"   "$RELEASE_LIB/libusb1.0/arm64-v8a/libusb1.0.so"
cp    "libs/x86/libusb1.0.so"     	   "$RELEASE_LIB/libusb1.0/x86/libusb1.0.so"
cp    "libs/x86_64/libusb1.0.so"  	   "$RELEASE_LIB/libusb1.0/x86_64/libusb1.0.so"

echo  "LOCAL_PATH := \$(call my-dir)"                    >> "$RELEASE_LIB/libusb1.0/Android.mk"
echo  "include \$(CLEAR_VARS)"                           >> "$RELEASE_LIB/libusb1.0/Android.mk"
echo  "LOCAL_MODULE     := usb1.0"                          >> "$RELEASE_LIB/libusb1.0/Android.mk"
echo  "LOCAL_SRC_FILES  := \$(TARGET_ARCH_ABI)/libusb1.0.so">> "$RELEASE_LIB/libusb1.0/Android.mk"
echo  "include \$(PREBUILT_SHARED_LIBRARY)"              >> "$RELEASE_LIB/libusb1.0/Android.mk"

echo  "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/libusb1.0"
cp -R AndroidManifest.xml "$RELEASE_SRC/libusb1.0"
cp -R jni                 "$RELEASE_SRC/libusb1.0"
cp -R src                 "$RELEASE_SRC/libusb1.0"
cp -a .project            "$RELEASE_SRC/libusb1.0"
cp -a .cproject           "$RELEASE_SRC/libusb1.0"
cp -a .classpath          "$RELEASE_SRC/libusb1.0"

cd    "$BUILD_FOLDER"
echo  ""

echo "#########################################################################"
echo "# Build IBScanCommon library"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/IBScanCommon"

echo "Updating the project files..."
android update project --path . --name IBScanCommon

echo "Building the Android Java library..."
ant -Denv.ANDROID_HOME="$SDK_PATH" -verbose release

echo "Check build..."
if [ -e "bin/classes.jar" ]
then
echo "Found 'bin/classes.jar'"
else
echo "Could not find 'bin/classes.jar'"
exit 0
fi

echo  "Moving binary files to output folder..."
mkdir "$RELEASE_LIB/libibscancommon"
cp    "bin/classes.jar" "$RELEASE_LIB/libibscancommon/libibscancommon.jar"

echo  "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/IBScanCommon"
cp -R AndroidManifest.xml "$RELEASE_SRC/IBScanCommon"
cp -R src                 "$RELEASE_SRC/IBScanCommon"
cp -a .project            "$RELEASE_SRC/IBScanCommon"
cp -a .classpath          "$RELEASE_SRC/IBScanCommon"

cd    "$BUILD_FOLDER"
echo  ""

echo "#########################################################################"
echo "# Build IBScanUltimate library"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/IBScanUltimate"
mv "$TEMP_FOLDER/IBScanUltimate/Android_root.mk"                            "$TEMP_FOLDER/IBScanUltimate/Android.mk"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Android_root.mk"        "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Android.mk"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Application_root.mk"    "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Application.mk"
sed s/'System.loadLibrary("usb");'/'System.loadLibrary("usb1.0");'/g "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScan.java" > "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScan.java.tmp"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScan.java.tmp" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScan.java"
sed s/'System.loadLibrary("usb");'/'System.loadLibrary("usb1.0");'/g "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java" > "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java.tmp"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java.tmp" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java"
sed s/'LibUsb'/'LibUsb1.0'/g "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties" > "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties.tmp"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties.tmp" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties"

echo "Building the native library..."
ndk-build NDK_MODULE_PATH="$RELEASE_LIB:$TEMP_FOLDER:$TEMP_FOLDER/IBScanUltimate/NBIS_Libs/android:$TEMP_FOLDER/IBScanUltimate/CxImage_Libs/android" V=1 NDK_DEBUG=0

echo "Updating the project files..."
android update project --path . --name IBScanUltimate

echo "Building the Android Java library..."
ant -Denv.ANDROID_HOME="$SDK_PATH" -verbose release

echo "Check build..."
if [ -e "bin/classes.jar" ]
then
    echo "Found 'bin/classes.jar'"
else
    echo "Could not find 'bin/classes.jar'"
    exit 0
fi
if [ -e "libs/armeabi/libibscanultimate.so" ]
then
    echo "Found 'libs/armeabi/libibscanultimate.so'"
else
    echo "Could not find 'libs/armeabi/libibscanultimate.so'"
    exit 0
fi
if [ -e "libs/armeabi-v7a/libibscanultimate.so" ]
then
    echo "Found 'libs/armeabi-v7a/libibscanultimate.so'"
else
    echo "Could not find 'libs/armeabi-v7a/libibscanultimate.so'"
    exit 0
fi
if [ -e "libs/x86/libibscanultimate.so" ]
then
    echo "Found 'libs/x86/libibscanultimate.so'"
else
    echo "Could not find 'libs/x86/libibscanultimate.so'"
    exit 0
fi
if [ -e "libs/armeabi/libibscanultimatejni.so" ]
then
    echo "Found 'libs/armeabi/libibscanultimatejni.so'"
else
    echo "Could not find 'libs/armeabi/libibscanultimatejni.so'"
    exit 0
fi
if [ -e "libs/armeabi-v7a/libibscanultimatejni.so" ]
then
    echo "Found 'libs/armeabi-v7a/libibscanultimatejni.so'"
else
    echo "Could not find 'libs/armeabi-v7a/libibscanultimatejni.so'"
    exit 0
fi
if [ -e "libs/x86/libibscanultimatejni.so" ]
then
    echo "Found 'libs/x86/libibscanultimatejni.so'"
else
    echo "Could not find 'libs/x86/libibscanultimatejni.so'"
    exit 0
fi

echo  "Moving binary files to output folder..."
mkdir "$RELEASE_LIB/libibscanultimate"
mkdir "$RELEASE_LIB/libibscanultimate/armeabi"
mkdir "$RELEASE_LIB/libibscanultimate/armeabi-v7a"
mkdir "$RELEASE_LIB/libibscanultimate/arm64-v8a"
mkdir "$RELEASE_LIB/libibscanultimate/x86"
mkdir "$RELEASE_LIB/libibscanultimate/x86_64"
cp    "libs/armeabi/libibscanultimate.so"     "$RELEASE_LIB/libibscanultimate/armeabi/libibscanultimate.so"
cp    "libs/armeabi-v7a/libibscanultimate.so" "$RELEASE_LIB/libibscanultimate/armeabi-v7a/libibscanultimate.so"
cp    "libs/arm64-v8a/libibscanultimate.so"   "$RELEASE_LIB/libibscanultimate/arm64-v8a/libibscanultimate.so"
cp    "libs/x86/libibscanultimate.so"         "$RELEASE_LIB/libibscanultimate/x86/libibscanultimate.so"
cp    "libs/x86_64/libibscanultimate.so"      "$RELEASE_LIB/libibscanultimate/x86_64/libibscanultimate.so"
mkdir "$RELEASE_LIB/libibscanultimatejni"
mkdir "$RELEASE_LIB/libibscanultimatejni/armeabi"
mkdir "$RELEASE_LIB/libibscanultimatejni/armeabi-v7a"
mkdir "$RELEASE_LIB/libibscanultimatejni/arm64-v8a"
mkdir "$RELEASE_LIB/libibscanultimatejni/x86"
mkdir "$RELEASE_LIB/libibscanultimatejni/x86_64"
cp    "bin/classes.jar"                          "$RELEASE_LIB/libibscanultimatejni/libibscanultimatejni.jar"
cp    "libs/armeabi/libibscanultimatejni.so"     "$RELEASE_LIB/libibscanultimatejni/armeabi/libibscanultimatejni.so"
cp    "libs/armeabi-v7a/libibscanultimatejni.so" "$RELEASE_LIB/libibscanultimatejni/armeabi-v7a/libibscanultimatejni.so"
cp    "libs/arm64-v8a/libibscanultimatejni.so" "$RELEASE_LIB/libibscanultimatejni/arm64-v8a/libibscanultimatejni.so"
cp    "libs/x86/libibscanultimatejni.so"     "$RELEASE_LIB/libibscanultimatejni/x86/libibscanultimatejni.so"
cp    "libs/x86_64/libibscanultimatejni.so"     "$RELEASE_LIB/libibscanultimatejni/x86_64/libibscanultimatejni.so"

echo  "LOCAL_PATH := \$(call my-dir)"                               >> "$RELEASE_LIB/libibscanultimate/Android.mk"
echo  "include \$(CLEAR_VARS)"                                      >> "$RELEASE_LIB/libibscanultimate/Android.mk"
echo  "LOCAL_MODULE     := ibscanultimate"                          >> "$RELEASE_LIB/libibscanultimate/Android.mk"
echo  "LOCAL_SRC_FILES  := \$(TARGET_ARCH_ABI)/libibscanultimate.so">> "$RELEASE_LIB/libibscanultimate/Android.mk"
echo  "include \$(PREBUILT_SHARED_LIBRARY)"                         >> "$RELEASE_LIB/libibscanultimate/Android.mk"

echo  "LOCAL_PATH := \$(call my-dir)"                                  >> "$RELEASE_LIB/libibscanultimatejni/Android.mk"
echo  "include \$(CLEAR_VARS)"                                         >> "$RELEASE_LIB/libibscanultimatejni/Android.mk"
echo  "LOCAL_MODULE     := ibscanultimatejni"                          >> "$RELEASE_LIB/libibscanultimatejni/Android.mk"
echo  "LOCAL_SRC_FILES  := \$(TARGET_ARCH_ABI)/libibscanultimatejni.so">> "$RELEASE_LIB/libibscanultimatejni/Android.mk"
echo  "include \$(PREBUILT_SHARED_LIBRARY)"                            >> "$RELEASE_LIB/libibscanultimatejni/Android.mk"

echo "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/IBScanUltimate"
cp -R AndroidManifest.xml "$RELEASE_SRC/IBScanUltimate"
cp -R jni                 "$RELEASE_SRC/IBScanUltimate"
cp -R src                 "$RELEASE_SRC/IBScanUltimate"
cp -a .classpath          "$RELEASE_SRC/IBScanUltimate"
cp -a .cproject           "$RELEASE_SRC/IBScanUltimate"
cp -a .project            "$RELEASE_SRC/IBScanUltimate"
cp -R "$TEMP_FOLDER/include"  "$RELEASE_FOLDER"

echo   "Clean up to help build SimpleScan sample"
rm -rf "libs"

cd   "$BUILD_FOLDER"
echo ""

echo "#########################################################################"
echo "# Build TestScan application for rooted device"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/TestScan"

echo "Building the native library..."
ndk-build NDK_MODULE_PATH="$RELEASE_LIB:$TEMP_FOLDER:$TEMP_FOLDER/IBScanUltimate/NBIS_Libs/android:$TEMP_FOLDER/IBScanUltimate/CxImage_Libs/android" V=1 NDK_DEBUG=0


echo "Check build..."
if [ -e "libs/armeabi/testScanU" ]
then
    echo "Found 'libs/armeabi/testScanU'"
else
    echo "Could not find 'libs/armeabi/testScanU'"
    exit 0
fi
if [ -e "libs/armeabi-v7a/testScanU" ]
then
    echo "Found 'libs/armeabi-v7a/testScanU'"
else
    echo "Could not find 'libs/armeabi-v7a/testScanU'"
    exit 0
fi
if [ -e "libs/x86/testScanU" ]
then
    echo "Found 'libs/x86/testScanU'"
else
    echo "Could not find 'libs/x86/testScanU'"
    exit 0
fi

echo  "Moving binary files to output folder..."
mkdir "$RELEASE_BIN/TestScan"
mkdir "$RELEASE_BIN/TestScan/armeabi"
mkdir "$RELEASE_BIN/TestScan/armeabi-v7a"
mkdir "$RELEASE_BIN/TestScan/arm64-v8a"
mkdir "$RELEASE_BIN/TestScan/x86"
mkdir "$RELEASE_BIN/TestScan/x86_64"
cp    "libs/armeabi/testScanU"     "$RELEASE_BIN/TestScan/armeabi/testScanU"
cp    "libs/armeabi-v7a/testScanU" "$RELEASE_BIN/TestScan/armeabi-v7a/testScanU"
cp    "libs/arm64-v8a/testScanU"   "$RELEASE_BIN/TestScan/arm64-v8a/testScanU"
cp    "libs/x86/testScanU"         "$RELEASE_BIN/TestScan/x86/testScanU"
cp    "libs/x86_64/testScanU"      "$RELEASE_BIN/TestScan/x86_64/testScanU"

echo "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/TestScan"
cp -a Android.mk          "$RELEASE_SRC/TestScan"
cp -R jni                 "$RELEASE_SRC/TestScan"

echo   "Clean up to help build SimpleScan sample"
rm -rf "libs"

cd   "$BUILD_FOLDER"
echo ""

echo "#########################################################################"
echo "# Build IB SimpleScan sample application for rooted device"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/SimpleScanSample"

echo "Updating the project files..."
android update project --path . --name SimpleScanSample

echo  "Building the Android app..."
echo  "Copy libraries into sample application..."
mkdir "libs"
mkdir "libs/armeabi"
mkdir "libs/armeabi-v7a"
mkdir "libs/arm64-v8a"
mkdir "libs/x86"
mkdir "libs/x86_64"
cp    "$RELEASE_LIB/libibscanultimate/armeabi/libibscanultimate.so"           "libs/armeabi"
cp    "$RELEASE_LIB/libibscanultimate/armeabi-v7a/libibscanultimate.so"       "libs/armeabi-v7a"
cp    "$RELEASE_LIB/libibscanultimate/arm64-v8a/libibscanultimate.so"         "libs/arm64-v8a"
cp    "$RELEASE_LIB/libibscanultimate/x86/libibscanultimate.so"               "libs/x86"
cp    "$RELEASE_LIB/libibscanultimate/x86_64/libibscanultimate.so"            "libs/x86_64"
cp    "$RELEASE_LIB/libibscanultimatejni/armeabi/libibscanultimatejni.so"     "libs/armeabi"
cp    "$RELEASE_LIB/libibscanultimatejni/armeabi-v7a/libibscanultimatejni.so" "libs/armeabi-v7a"
cp    "$RELEASE_LIB/libibscanultimatejni/arm64-v8a/libibscanultimatejni.so"   "libs/arm64-v8a"
cp    "$RELEASE_LIB/libibscanultimatejni/x86/libibscanultimatejni.so"         "libs/x86"
cp    "$RELEASE_LIB/libibscanultimatejni/x86_64/libibscanultimatejni.so"      "libs/x86_64"
cp    "$RELEASE_LIB/libibscanultimatejni/libibscanultimatejni.jar"            "libs"
cp    "$RELEASE_LIB/libibscancommon/libibscancommon.jar"                      "libs"
cp    "$RELEASE_LIB/libusb1.0/armeabi/libusb1.0.so"                           "libs/armeabi"
cp    "$RELEASE_LIB/libusb1.0/armeabi-v7a/libusb1.0.so"                       "libs/armeabi-v7a"
cp    "$RELEASE_LIB/libusb1.0/arm64-v8a/libusb1.0.so"                               "libs/arm64-v8a"
cp    "$RELEASE_LIB/libusb1.0/x86/libusb1.0.so"                                     "libs/x86"
cp    "$RELEASE_LIB/libusb1.0/x86_64/libusb1.0.so"                                  "libs/x86_64"
cp    "$RELEASE_LIB/libusb1.0/libusb.jar"                                     "libs"
cp -R project.properties.release project.properties
ant -Denv.ANDROID_HOME="$SDK_PATH" -verbose release

echo "Check build..."
if [ -e "bin/SimpleScanSample-release-unsigned.apk" ]
then
    echo "Found 'bin/SimpleScanSample-release-unsigned.apk'"
else
    echo "Could not find 'bin/SimpleScanSample-release-unsigned.apk'"
    exit 0
fi

echo "Signing the app..."
jarsigner -verbose -sigalg MD5withRSA -digestalg SHA1 -keystore "$MY_KEYSTORE" -storepass r1A2f4F8L1e6S -keypass r1A2f4F8L1e6S -signedjar "bin/SimpleScanSample-release-signed-unaligned.apk" "bin/SimpleScanSample-release-unsigned.apk" temp-release-key
if [ -e "bin/SimpleScanSample-release-signed-unaligned.apk" ]
then
    echo "Found 'bin/SimpleScanSample-release-signed-unaligned.apk'"
else
	echo "Cound not find 'bin/SimpleScanSample-release-signed-unaligned.apk'"
	exit 0
fi

echo "Aligning the app..."
zipalign -v 4 "bin/SimpleScanSample-release-signed-unaligned.apk" "bin/SimpleScanSample-release-signed.apk"
if [ -e "bin/SimpleScanSample-release-signed.apk" ]
then
    echo "Found 'bin/SimpleScanSample-release-signed.apk'"
else
	echo "Cound not find 'bin/SimpleScanSample-release-signed.apk'"
	exit 0
fi

echo "Moving binary files to output folder..."
cp "bin/SimpleScanSample-release-signed.apk" "$RELEASE_BIN/SimpleScanSample.apk"

echo "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/SimpleScanSample"
cp -R AndroidManifest.xml        "$RELEASE_SRC/SimpleScanSample"
cp -R ant.properties             "$RELEASE_SRC/SimpleScanSample"
cp -R build.xml                  "$RELEASE_SRC/SimpleScanSample"
cp -R custom-rules.xml           "$RELEASE_SRC/SimpleScanSample"
cp -R proguard-project.txt       "$RELEASE_SRC/SimpleScanSample"
cp -R proguard.cfg               "$RELEASE_SRC/SimpleScanSample"
cp -R project.properties.release "$RELEASE_SRC/SimpleScanSample/project.properties"
cp -R res                        "$RELEASE_SRC/SimpleScanSample"
cp -R src                        "$RELEASE_SRC/SimpleScanSample"
cp -a .project                   "$RELEASE_SRC/SimpleScanSample"
cp -a .classpath                 "$RELEASE_SRC/SimpleScanSample"

echo  "Copy libraries into sample application..."
mkdir "$RELEASE_SRC/SimpleScanSample/libs"
mkdir "$RELEASE_SRC/SimpleScanSample/libs/armeabi"
mkdir "$RELEASE_SRC/SimpleScanSample/libs/armeabi-v7a"
mkdir "$RELEASE_SRC/SimpleScanSample/libs/arm64-v8a"
mkdir "$RELEASE_SRC/SimpleScanSample/libs/x86"
mkdir "$RELEASE_SRC/SimpleScanSample/libs/x86_64"
cp    "$RELEASE_LIB/libibscanultimate/armeabi/libibscanultimate.so"           "$RELEASE_SRC/SimpleScanSample/libs/armeabi"
cp    "$RELEASE_LIB/libibscanultimate/armeabi-v7a/libibscanultimate.so"       "$RELEASE_SRC/SimpleScanSample/libs/armeabi-v7a"
cp    "$RELEASE_LIB/libibscanultimate/arm64-v8a/libibscanultimate.so"         "$RELEASE_SRC/SimpleScanSample/libs/arm64-v8a"
cp    "$RELEASE_LIB/libibscanultimate/x86/libibscanultimate.so"               "$RELEASE_SRC/SimpleScanSample/libs/x86"
cp    "$RELEASE_LIB/libibscanultimate/x86_64/libibscanultimate.so"            "$RELEASE_SRC/SimpleScanSample/libs/x86_64"
cp    "$RELEASE_LIB/libibscanultimatejni/armeabi/libibscanultimatejni.so"     "$RELEASE_SRC/SimpleScanSample/libs/armeabi"
cp    "$RELEASE_LIB/libibscanultimatejni/armeabi-v7a/libibscanultimatejni.so" "$RELEASE_SRC/SimpleScanSample/libs/armeabi-v7a"
cp    "$RELEASE_LIB/libibscanultimatejni/arm64-v8a/libibscanultimatejni.so"   "$RELEASE_SRC/SimpleScanSample/libs/arm64-v8a"
cp    "$RELEASE_LIB/libibscanultimatejni/x86/libibscanultimatejni.so"         "$RELEASE_SRC/SimpleScanSample/libs/x86"
cp    "$RELEASE_LIB/libibscanultimatejni/x86_64/libibscanultimatejni.so"      "$RELEASE_SRC/SimpleScanSample/libs/x86_64"
cp    "$RELEASE_LIB/libibscanultimatejni/libibscanultimatejni.jar"            "$RELEASE_SRC/SimpleScanSample/libs"
cp    "$RELEASE_LIB/libibscancommon/libibscancommon.jar"                      "$RELEASE_SRC/SimpleScanSample/libs"
cp    "$RELEASE_LIB/libusb1.0/armeabi/libusb1.0.so"                           "$RELEASE_SRC/SimpleScanSample/libs/armeabi"
cp    "$RELEASE_LIB/libusb1.0/armeabi-v7a/libusb1.0.so"                       "$RELEASE_SRC/SimpleScanSample/libs/armeabi-v7a"
cp    "$RELEASE_LIB/libusb1.0/arm64-v8a/libusb1.0.so"                               "$RELEASE_SRC/SimpleScanSample/libs/arm64-v8a"
cp    "$RELEASE_LIB/libusb1.0/x86/libusb1.0.so"                                     "$RELEASE_SRC/SimpleScanSample/libs/x86"
cp    "$RELEASE_LIB/libusb1.0/x86_64/libusb1.0.so"                                  "$RELEASE_SRC/SimpleScanSample/libs/x86_64"
cp    "$RELEASE_LIB/libusb1.0/libusb.jar"                                     "$RELEASE_SRC/SimpleScanSample/libs"

echo "Creating archive from release..."
cd   "$BUILD_FOLDER"
tar -vczf "$RELEASE_NAME.tar.gz" --exclude=".DS_Store" --exclude=".svn" "$RELEASE_NAME"

echo ""

echo "#########################################################################"
echo "# Cleanup"
echo "#########################################################################"

rm -rf $RELEASE_NAME
rm -rf $TEMP_FOLDER
#rm -f  $MY_KEYSTORE

echo "Build complete"
echo ""
