#!/bin/bash

echo "################################################################################"
echo "#"
echo "# BUILD SCRIPT FOR ANDROID IBScanUltimatePI SDK"
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
RELEASE_NAME=IBScanUltimate_AndroidPI_$VERSION

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

echo  "Transferring LibUsb"
cp -R "$REPO_FOLDER/Samples/Android/LibUsb"                      "$TEMP_FOLDER/Samples/Android"
cp -a "$REPO_FOLDER/Samples/Android/LibUsb/.classpath"           "$TEMP_FOLDER/Samples/Android/LibUsb"
cp -a "$REPO_FOLDER/Samples/Android/LibUsb/.cproject"            "$TEMP_FOLDER/Samples/Android/LibUsb"
cp -a "$REPO_FOLDER/Samples/Android/LibUsb/.project"             "$TEMP_FOLDER/Samples/Android/LibUsb"

echo  "Transferring LibDeviceParallel"
cp -R "$REPO_FOLDER/LibDeviceParallel"                 			  "$TEMP_FOLDER/Samples/Android"
rm -f "$TEMP_FOLDER/Samples/Android/libDeviceParallel/jni/Makefile*.mk"

echo  "Transferring SimpleScanSample"
cp -R "$REPO_FOLDER/Samples/Android/SimpleScanSample"            "$TEMP_FOLDER/Samples/Android"
cp -a "$REPO_FOLDER/Samples/Android/SimpleScanSample/.classpath" "$TEMP_FOLDER/Samples/Android/SimpleScanSample"
cp -a "$REPO_FOLDER/Samples/Android/SimpleScanSample/.project"   "$TEMP_FOLDER/Samples/Android/SimpleScanSample"
sed s/'IB SimpleScan'/'IB SimpleScanPI'/g "$TEMP_FOLDER/Samples/Android/SimpleScanSample/res/values/strings.xml" > "$TEMP_FOLDER/Samples/Android/SimpleScanSample/res/values/strings.tmp"
mv "$TEMP_FOLDER/Samples/Android/SimpleScanSample/res/values/strings.tmp" "$TEMP_FOLDER/Samples/Android/SimpleScanSample/res/values/strings.xml"

echo "Transferring other files"
cp -R "$REPO_FOLDER/include"                                     "$TEMP_FOLDER/include"
cp -R "$REPO_FOLDER/IBScanUltimate"                              "$TEMP_FOLDER/IBScanUltimate"

echo "Cleaning unwanted files"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/LibUsb/obj"
rm -rf "$TEMP_FOLDER/Samples/Android/LibDeviceParallel/bin"
rm -rf "$TEMP_FOLDER/Samples/Android/LibDeviceParallel/gen"
rm -rf "$TEMP_FOLDER/Samples/Android/LibDeviceParallel/libs"
rm -rf "$TEMP_FOLDER/Samples/Android/LibDeviceParallel/obj"
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

echo ""

echo "#########################################################################"
echo "# Build LibUsb library"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/LibUsb"

echo "Building the native library..."
ndk-build V=1 NDK_DEBUG=0

echo "Updating the project files..."
android update project --path . --name LibUsb --target android-15

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
if [ -e "libs/armeabi/libusb.so" ]
then
    echo "Found 'libs/armeabi/libusb.so'"
else
    echo "Could not find 'libs/armeabi/libusb.so'"
    exit 0
fi
if [ -e "libs/armeabi-v7a/libusb.so" ]
then
    echo "Found 'libs/armeabi-v7a/libusb.so'"
else
    echo "Could not find 'libs/armeabi-v7a/libusb.so'"
    exit 0
fi
if [ -e "libs/x86/libusb.so" ]
then
    echo "Found 'libs/x86/libusb.so'"
else
    echo "Could not find 'libs/x86/libusb.so'"
    exit 0
fi

echo  "Moving binary files to output folder..."
mkdir "$RELEASE_LIB/libusb"
mkdir "$RELEASE_LIB/libusb/armeabi"
mkdir "$RELEASE_LIB/libusb/armeabi-v7a"
mkdir "$RELEASE_LIB/libusb/arm64-v8a"
mkdir "$RELEASE_LIB/libusb/x86"
mkdir "$RELEASE_LIB/libusb/x86_64"
cp    "bin/classes.jar"            "$RELEASE_LIB/libusb/libusb.jar"
cp    "libs/armeabi/libusb.so"     "$RELEASE_LIB/libusb/armeabi/libusb.so"
cp    "libs/armeabi-v7a/libusb.so" "$RELEASE_LIB/libusb/armeabi-v7a/libusb.so"
cp    "libs/arm64-v8a/libusb.so"   "$RELEASE_LIB/libusb/arm64-v8a/libusb.so"
cp    "libs/x86/libusb.so"     	   "$RELEASE_LIB/libusb/x86/libusb.so"
cp    "libs/x86_64/libusb.so"  	   "$RELEASE_LIB/libusb/x86_64/libusb.so"

echo  "LOCAL_PATH := \$(call my-dir)"                    >> "$RELEASE_LIB/libusb/Android.mk"
echo  "include \$(CLEAR_VARS)"                           >> "$RELEASE_LIB/libusb/Android.mk"
echo  "LOCAL_MODULE     := usb"                          >> "$RELEASE_LIB/libusb/Android.mk"
echo  "LOCAL_SRC_FILES  := \$(TARGET_ARCH_ABI)/libusb.so">> "$RELEASE_LIB/libusb/Android.mk"
echo  "include \$(PREBUILT_SHARED_LIBRARY)"              >> "$RELEASE_LIB/libusb/Android.mk"

echo  "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/libusb"
cp -R AndroidManifest.xml "$RELEASE_SRC/libusb"
cp -R jni                 "$RELEASE_SRC/libusb"
cp -R src                 "$RELEASE_SRC/libusb"
cp -a .project            "$RELEASE_SRC/libusb"
cp -a .cproject           "$RELEASE_SRC/libusb"
cp -a .classpath          "$RELEASE_SRC/libusb"

cd    "$BUILD_FOLDER"
echo  ""

echo "#########################################################################"
echo "# Build LibDeviceParallel library"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/LibDeviceParallel"

echo "Building the native library..."
ndk-build V=1 NDK_DEBUG=0

echo "Updating the project files..."
android update project --path . --name LibDeviceParallel --target android-15

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
if [ -e "libs/armeabi/libDeviceParallel.so" ]
then
    echo "Found 'libs/armeabi/libDeviceParallel.so'"
else
    echo "Could not find 'libs/armeabi/libDeviceParallel.so'"
    exit 0
fi
if [ -e "libs/armeabi-v7a/libDeviceParallel.so" ]
then
    echo "Found 'libs/armeabi-v7a/libDeviceParallel.so'"
else
    echo "Could not find 'libs/armeabi-v7a/libDeviceParallel.so'"
    exit 0
fi
if [ -e "libs/x86/libDeviceParallel.so" ]
then
    echo "Found 'libs/x86/libDeviceParallel.so'"
else
    echo "Could not find 'libs/x86/libDeviceParallel.so'"
    exit 0
fi

echo  "Moving binary files to output folder..."
mkdir "$RELEASE_LIB/libDeviceParallel"
mkdir "$RELEASE_LIB/libDeviceParallel/armeabi"
mkdir "$RELEASE_LIB/libDeviceParallel/armeabi-v7a"
mkdir "$RELEASE_LIB/libDeviceParallel/arm64-v8a"
mkdir "$RELEASE_LIB/libDeviceParallel/x86"
mkdir "$RELEASE_LIB/libDeviceParallel/x86_64"
cp    "libs/armeabi/libDeviceParallel.so"     "$RELEASE_LIB/libDeviceParallel/armeabi/libDeviceParallel.so"
cp    "libs/armeabi-v7a/libDeviceParallel.so" "$RELEASE_LIB/libDeviceParallel/armeabi-v7a/libDeviceParallel.so"
cp    "libs/arm64-v8a/libDeviceParallel.so"   "$RELEASE_LIB/libDeviceParallel/arm64-v8a/libDeviceParallel.so"
cp    "libs/x86/libDeviceParallel.so"     	   "$RELEASE_LIB/libDeviceParallel/x86/libDeviceParallel.so"
cp    "libs/x86_64/libDeviceParallel.so"  	   "$RELEASE_LIB/libDeviceParallel/x86_64/libDeviceParallel.so"

echo  "LOCAL_PATH := \$(call my-dir)"                               >> "$RELEASE_LIB/libDeviceParallel/Android.mk"
echo  "include \$(CLEAR_VARS)"                                      >> "$RELEASE_LIB/libDeviceParallel/Android.mk"
echo  "LOCAL_MODULE     := DeviceParallel"                          >> "$RELEASE_LIB/libDeviceParallel/Android.mk"
echo  "LOCAL_SRC_FILES  := \$(TARGET_ARCH_ABI)/libDeviceParallel.so">> "$RELEASE_LIB/libDeviceParallel/Android.mk"
echo  "include \$(PREBUILT_SHARED_LIBRARY)"                         >> "$RELEASE_LIB/libDeviceParallel/Android.mk"

echo  "Moving source files to output folder..."
mkdir -p "$RELEASE_SRC/libDeviceParallel"
cp -R AndroidManifest.xml "$RELEASE_SRC/libDeviceParallel"
cp -R jni                 "$RELEASE_SRC/libDeviceParallel"
cp -R src                 "$RELEASE_SRC/libDeviceParallel"
cp -a .project            "$RELEASE_SRC/libDeviceParallel"
cp -a .cproject           "$RELEASE_SRC/libDeviceParallel"
cp -a .classpath          "$RELEASE_SRC/libDeviceParallel"

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
mv "$TEMP_FOLDER/IBScanUltimate/Android_ppi.mk"                            "$TEMP_FOLDER/IBScanUltimate/Android.mk"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Android_ppi.mk"        "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Android.mk"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Application_ppi.mk"    "$TEMP_FOLDER/Samples/Android/IBScanUltimate/jni/Application.mk"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScan.java.ppi" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScan.java"
#sed s/'System.loadLibrary("usb");'/'System.loadLibrary("DeviceParallel");'/g "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java" > "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java.tmp"
#mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java.tmp" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java"
mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java.ppi" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/src/com/integratedbiometrics/ibscanultimate/IBScanDevice.java"
#sed s/'LibUsb'/'LibDeviceParallel'/g "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties" > "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties.tmp"
#mv "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties.tmp" "$TEMP_FOLDER/Samples/Android/IBScanUltimate/project.properties"

echo "Building the native library..."
ndk-build NDK_MODULE_PATH="$RELEASE_LIB:$TEMP_FOLDER:$TEMP_FOLDER/IBScanUltimate/NBIS_Libs/android:$TEMP_FOLDER/IBScanUltimate/CxImage_Libs/android" V=1 NDK_DEBUG=0

echo "Updating the project files..."
android update project --path . --name IBScanUltimate --target android-15

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
echo "# Build IB SimpleScan sample application"
echo "#########################################################################"

cd "$TEMP_FOLDER/Samples/Android/SimpleScanSample"

echo "Updating the project files..."
android update project --path . --name SimpleScanSample --target android-15

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
cp    "$RELEASE_LIB/libusb/armeabi/libusb.so"                                 "libs/armeabi"
cp    "$RELEASE_LIB/libusb/armeabi-v7a/libusb.so"                             "libs/armeabi-v7a"
cp    "$RELEASE_LIB/libusb/arm64-v8a/libusb.so"                               "libs/arm64-v8a"
cp    "$RELEASE_LIB/libusb/x86/libusb.so"                                     "libs/x86"
cp    "$RELEASE_LIB/libusb/x86_64/libusb.so"                                  "libs/x86_64"
cp    "$RELEASE_LIB/libusb/libusb.jar"                                        "libs"
cp    "$RELEASE_LIB/libDeviceParallel/armeabi/libDeviceParallel.so"           "libs/armeabi"
cp    "$RELEASE_LIB/libDeviceParallel/armeabi-v7a/libDeviceParallel.so"       "libs/armeabi-v7a"
cp    "$RELEASE_LIB/libDeviceParallel/arm64-v8a/libDeviceParallel.so"         "libs/arm64-v8a"
cp    "$RELEASE_LIB/libDeviceParallel/x86/libDeviceParallel.so"               "libs/x86"
cp    "$RELEASE_LIB/libDeviceParallel/x86_64/libDeviceParallel.so"            "libs/x86_64"
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
cp    "$RELEASE_LIB/libusb/armeabi/libusb.so"                                 "$RELEASE_SRC/SimpleScanSample/libs/armeabi"
cp    "$RELEASE_LIB/libusb/armeabi-v7a/libusb.so"                             "$RELEASE_SRC/SimpleScanSample/libs/armeabi-v7a"
cp    "$RELEASE_LIB/libusb/arm64-v8a/libusb.so"                               "$RELEASE_SRC/SimpleScanSample/libs/arm64-v8a"
cp    "$RELEASE_LIB/libusb/x86/libusb.so"                                     "$RELEASE_SRC/SimpleScanSample/libs/x86"
cp    "$RELEASE_LIB/libusb/x86_64/libusb.so"                                  "$RELEASE_SRC/SimpleScanSample/libs/x86_64"
cp    "$RELEASE_LIB/libusb/libusb.jar"                                        "$RELEASE_SRC/SimpleScanSample/libs"
cp    "$RELEASE_LIB/libDeviceParallel/armeabi/libDeviceParallel.so"           "$RELEASE_SRC/SimpleScanSample/libs/armeabi"
cp    "$RELEASE_LIB/libDeviceParallel/armeabi-v7a/libDeviceParallel.so"       "$RELEASE_SRC/SimpleScanSample/libs/armeabi-v7a"
cp    "$RELEASE_LIB/libDeviceParallel/arm64-v8a/libDeviceParallel.so"         "$RELEASE_SRC/SimpleScanSample/libs/arm64-v8a"
cp    "$RELEASE_LIB/libDeviceParallel/x86/libDeviceParallel.so"               "$RELEASE_SRC/SimpleScanSample/libs/x86"
cp    "$RELEASE_LIB/libDeviceParallel/x86_64/libDeviceParallel.so"            "$RELEASE_SRC/SimpleScanSample/libs/x86_64"

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
