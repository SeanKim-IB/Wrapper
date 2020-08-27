REM ################################################################################
REM #
REM # Initial directory should have 'IBScanUltimate', 'IBScanFwUpdater',
REM # 'Sample sources' folders.
REM #
REM ################################################################################

REM #
REM # Set up build.
REM #

SET   JAVA_HOME=C:\Program Files (x86)\Java\jdk1.6.0_43\bin\
SET   VS_HOME=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\
IF EXIST Bin (
	RD   /S /Q Bin
)
MKDIR Bin
IF EXIST Lib (
	RD   /S /Q Lib
)
MKDIR Lib
IF EXIST Build (
	RD   /S /Q Build
)
MKDIR Build

IF EXIST "Plugin\Bin" (
	RD   /S /Q "Plugin\Bin"
)
MKDIR "Plugin\Bin"
IF EXIST "Plugin\Lib" (
	RD   /S /Q "Plugin\Lib"
)
MKDIR "Plugin\Lib"
IF EXIST "Plugin\Build" (
	RD   /S /Q "Plugin\Build"
)
MKDIR "Plugin\Build"

IF NOT EXIST "Win Released Data" (
	MKDIR "Win Released Data"
)
IF EXIST "Win Released Data\x64" (
	RD   /S /Q "Win Released Data\x64"
)
MKDIR "Win Released Data\x64"

SET   VCPROJ_EXT=vcproj
SET   CSPROJ_EXT=csproj
SET   VBPROJ_EXT=vbproj

REM #
REM # Copy  common files to Bin, Build and etc
REM #
COPY                CommonFiles\IBSU_FunctionTesterForJava.bat Bin\IBSU_FunctionTesterForJava.bat
COPY                CommonFiles\IBScanUltimate_SampleForJava.bat Bin\IBScanUltimate_SampleForJava.bat
COPY                CommonFiles\FirmwareWriterSetting.ini Bin
COPY                ExternalLib\x64\IBScanMatcher.dll Bin
COPY                Plugin\IBScanNFIQ2\OpenCV_Libs\x64\opencv_core.dll Plugin\Bin
COPY                Plugin\IBScanNFIQ2\OpenCV_Libs\x64\opencv_imgproc.dll Plugin\Bin
COPY                Plugin\IBScanNFIQ2\OpenCV_Libs\x64\opencv_ml.dll Plugin\Bin

REM #
REM # Build IBScanUltimate DLL
REM #
SET                 SAMPLENAME=IBScanUltimate
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.dll" (
    ECHO "Build\%SAMPLENAME%.dll" does not exist
	GOTO :ERROR
)
IF NOT EXIST "Lib\%SAMPLENAME%.lib" (
    ECHO "Lib\%SAMPLENAME%.lib" does not exist
	GOTO :ERROR
)
COPY                Build\%SAMPLENAME%.dll Lib
COPY                Build\%SAMPLENAME%.dll Bin
COPY                Build\%SAMPLENAME%.dll "Win Released Data\x64"
COPY                Lib\%SAMPLENAME%.lib "Win Released Data\x64"
COPY                Build\%SAMPLENAME%.dll "Plugin\Lib"
COPY                Build\%SAMPLENAME%.dll "Plugin\Bin"
COPY                Build\%SAMPLENAME%.dll "Sample sources\Python\TestScanForPython"
COPY                Build\%SAMPLENAME%.dll "Sample sources\Python\IBScanUltimate_SampleForPySide"

REM #
REM # Clean NBIS library
REM #
FOR %%A in (bozorth3 commonnbis ioutil jpegl mindtct nfiq nfseg pcasys wsq) DO (
	IF EXIST "IBScanUltimate\NBIS_Libs\src\VSProject\VisualStudio2008\%%A\Release" (
		RD /S /Q "IBScanUltimate\NBIS_Libs\src\VSProject\VisualStudio2008\%%A\Release"
	)
)

REM #
REM # Build IBUsbFwUpdater
REM #
SET                 SAMPLENAME=IBUsbFwUpdater
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_SampleForVC
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForVC
SET                 SAMPLEDIR=Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_TenScanSampleForVC
REM #
SET                 SAMPLENAME=IBScanUltimate_TenScanSampleForVC
SET                 SAMPLEDIR=Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBSU_NonCallbackSampleForVC
REM #
SET                 SAMPLENAME=IBSU_NonCallbackSampleForVC
SET                 SAMPLEDIR=Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBSU_FunctionTesterForVC
REM #
SET                 SAMPLENAME=IBScanUltimate_FunctionTester
SET                 SAMPLEDIR=Sample sources\VisualC++\IBSU_FunctionTesterForVC
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBSU_NewFunctionTesterForVC
REM #
SET                 SAMPLENAME=IBSU_NewFunctionTesterForVC
SET                 SAMPLEDIR=Sample sources\VisualC++\IBSU_NewFunctionTesterForVC
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBSU_DynamicLinkedSampleForVC
REM #
SET                 SAMPLENAME=IBSU_DynamicLinkedSampleForVC
SET                 SAMPLEDIR=Sample sources\VisualC++\IBSU_DynamicLinkedSampleForVC
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanNFIQ2 DLL
REM #
SET                 SAMPLENAME=IBScanNFIQ2
SET                 SAMPLEDIR=Plugin\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanNFIQ2.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Plugin\Build\%SAMPLENAME%.dll" (
    ECHO "Plugin\Build\%SAMPLENAME%.dll" does not exist
	GOTO :ERROR
)
IF NOT EXIST "Plugin\Lib\%SAMPLENAME%.lib" (
    ECHO "Plugin\Lib\%SAMPLENAME%.lib" does not exist
	GOTO :ERROR
)
COPY                Plugin\Build\%SAMPLENAME%.dll "Plugin\Lib"
COPY                Plugin\Build\%SAMPLENAME%.dll "Plugin\Bin"
COPY                Plugin\Build\%SAMPLENAME%.dll "Win Released Data\x64"
COPY                Plugin\Build\%SAMPLENAME%.lib "Win Released Data\x64"

REM #
REM # Build IBScanNFIQ2_SampleForVC
REM #
SET                 SAMPLENAME=IBScanNFIQ2_SampleForVC
SET                 SAMPLEDIR=Plugin\IBScanNFIQ2\Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanNFIQ2.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Plugin\Build\%SAMPLENAME%.exe" (
    ECHO "Plugin\Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Plugin\Build\%SAMPLENAME%.exe "Plugin\Bin"

REM #
REM # Build IBScanUltimate_SampleForCSharp
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForCSharp
SET                 SAMPLEDIR=Sample sources\CSharp\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\obj"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%CSPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_SampleForVBNet
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForVBNet
SET                 SAMPLEDIR=Sample sources\VB_Net\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\obj"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VBPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBLogTracer
REM #
SET                 SAMPLENAME=IBLogTracer
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_SalesDemo
REM #
SET                 SAMPLENAME=IBScanUltimate_SalesDemo
SET                 SAMPLEDIR=Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|x64" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|x64"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM ###########################################################################
REM # Clean projects
REM ###########################################################################

REM #
REM # Clean C++ samples
REM #
FOR %%A in (IBSU_NewFunctionTesterForVC IBSU_FunctionTesterForVC IBScanUltimate_TenScanSampleForVC IBSU_NonCallbackSampleForVC IBScanUltimate_SampleForVC IBSU_ConsoleSampleForVC IBSU_Holmes_DebuggerForVC IBSU_MultipleScanSampleForVC) DO (
	IF EXIST "Sample sources\VisualC++\%%A\x64" (
		RD /S /Q "Sample sources\VisualC++\%%A\x64"
	)
)

REM #
REM # Clean IBSU_Holmes_DebuggerForCSharp
REM #
SET                 SAMPLEDIR=Sample sources\CSharp\IBSU_Holmes_DebuggerForCSharp
IF EXIST "%SAMPLEDIR%\obj" (
	RD        /S /Q "%SAMPLEDIR%\obj"
)
REM #
REM # Clean IBScanUltimate DLL
REM #
SET                 SAMPLENAME=IBScanUltimate
SET                 SAMPLEDIR=%SAMPLENAME%
DEL              /Q Lib\%SAMPLENAME%.exp
RD            /S /Q "%SAMPLEDIR%\x64"
RD            /S /Q "%SAMPLEDIR%\Release"

REM #
REM # Clean IBUsbFwUpdater
REM #
SET                 SAMPLENAME=IBUsbFwUpdater
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"

REM #
REM # Clean IBScanUltimate_SampleForCSharp
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForCSharp
SET                 SAMPLEDIR=Sample sources\CSharp\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\obj"

REM #
REM # Clean IBScanUltimate_SampleForVBNet
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForVBNet
SET                 SAMPLEDIR=Sample sources\VB_Net\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\obj"

REM #
REM # Clean IBLogTracer
REM #
SET                 SAMPLENAME=IBLogTracer
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"

REM #
REM # Clean IBScanUltimate_SalesDemo
REM #
SET                 SAMPLENAME=IBScanUltimate_SalesDemo
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"

REM #
REM # Clean IBScanNFIQ2 DLL
REM #
SET                 SAMPLENAME=IBScanNFIQ2
SET                 SAMPLEDIR=Plugin\%SAMPLENAME%
DEL              /Q Plugin\Lib\%SAMPLENAME%.exp
RD            /S /Q "%SAMPLEDIR%\x64"
RD            /S /Q "%SAMPLEDIR%\Release"

REM #
REM # Clean IBScanNFIQ2_SampleForVC
REM #
SET                 SAMPLENAME=IBScanNFIQ2_SampleForVC
SET                 SAMPLEDIR=Plugin\Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\x64"

REM ###########################################################################
REM # Build Java samples
REM ###########################################################################

REM #
REM # Build Java wrapper around library
REM #

CD    "Sample sources\Java\IBScanCommon"
RD    /S /Q bin
MKDIR bin
"%JAVA_HOME%javac.exe" -sourcepath "src" "src\com\integratedbiometrics\ibscancommon\IBCommon.java" -d bin
"%JAVA_HOME%jar.exe" cf "IBScanCommon.jar" -C bin .
IF NOT EXIST "IBScanCommon.jar" (
    CD ..\..\..
    ECHO "IBScanCommon.jar" does not exist
	GOTO :ERROR
)
COPY  IBScanCommon.jar ..\..\..\Bin
COPY  IBScanCommon.jar ..\..\..\Build
COPY  IBScanCommon.jar "..\..\..\Win Released Data\x64"
MOVE  IBScanCommon.jar ..\..\..\Lib
RD    /S /Q bin
CD    ..\..\..

CD    "Sample sources\Java\IBScanUltimate"
RD    /S /Q bin
MKDIR bin
"%JAVA_HOME%javac.exe" -sourcepath "src" -classpath "..\..\..\Build\IBScanCommon.jar" "src\com\integratedbiometrics\ibscanultimate\IBScan.java" "src\com\integratedbiometrics\ibscanultimate\IBScanDevice.java" "src\com\integratedbiometrics\ibscanultimate\IBScanListener.java" "src\com\integratedbiometrics\ibscanultimate\IBScanDeviceListener.java" "src\com\integratedbiometrics\ibscanultimate\IBScanException.java" -d bin
"%JAVA_HOME%jar.exe" cf "IBScanUltimate.jar" -C bin .
IF NOT EXIST "IBScanUltimate.jar" (
    CD ..\..\..
    ECHO "IBScanUltimate.jar" does not exist
	GOTO :ERROR
)
COPY  IBScanUltimate.jar ..\..\..\Bin
COPY  IBScanUltimate.jar ..\..\..\Build
COPY  IBScanUltimate.jar "..\..\..\Win Released Data\x64"
MOVE  IBScanUltimate.jar ..\..\..\Lib
RD    /S /Q bin
CD    ..\..\..

REM #
REM # Build FunctionTester sample
REM #

CD    "Sample sources\Java\FunctionTesterForJava"
MKDIR bin
MKDIR bin\resources
COPY  src\resources\* bin\resources
"%JAVA_HOME%javac.exe" -sourcepath "src" -classpath "..\..\..\Build\IBScanCommon.jar:..\..\..\Build\IBScanUltimate.jar" "src\com\sample\functiontester\FunctionTester.java" -d bin
"%JAVA_HOME%jar.exe" cfm "FunctionTester.jar" "Manifest.txt" -C bin .
IF NOT EXIST "FunctionTester.jar" (
    CD ..\..\..
    ECHO "FunctionTester.jar" does not exist
	GOTO :ERROR
)
MOVE  FunctionTester.jar ..\..\..\Bin
RD    /S /Q bin
CD    ..\..\..

REM #
REM # Build IBScanUltimate_SampleForJava sample
REM #

CD    "Sample sources\Java\IBScanUltimate_SampleForJava"
MKDIR bin
MKDIR bin\resources
COPY  src\resources\* bin\resources
"%JAVA_HOME%javac.exe" -sourcepath "src" -classpath "..\..\..\Build\IBScanCommon.jar:..\..\..\Build\IBScanUltimate.jar" "src\com\sample\IBScanUltimate_SampleforJava\IBScanUltimate_Sample.java" "src\com\sample\IBScanUltimate_SampleforJava\PlaySound.java" -d bin
"%JAVA_HOME%jar.exe" cfm "IBScanUltimate_SampleForJava.jar" "Manifest.txt" -C bin .
IF NOT EXIST "IBScanUltimate_SampleForJava.jar" (
    CD ..\..\..
    ECHO "IBScanUltimate_SampleForJava.jar" does not exist
	GOTO :ERROR
)
MOVE  IBScanUltimate_SampleForJava.jar ..\..\..\Bin
RD    /S /Q bin
CD    ..\..\..

REM #
REM # Build JNI wrapper around library
REM #

CD   "Sample sources\Java\IBScanUltimateJNI"
RD   /S /Q x64
"%VS_HOME%devenv.exe" IBScanUltimateJNI.sln /rebuild "Release|x64" /project IBScanUltimateJNI.%VCPROJ_EXT% /projectconfig "Release|x64"
IF NOT EXIST "x64\Release\IBScanUltimateJNI.dll" (
    CD ..\..\..
    ECHO "x64\Release\IBScanUltimateJNI.dll" does not exist
	GOTO :ERROR
)
IF NOT EXIST "x64\Release\IBScanUltimateJNI.lib" (
    CD ..\..\..
    ECHO "x64\Release\IBScanUltimateJNI.lib" does not exist
	GOTO :ERROR
)
COPY x64\Release\IBScanUltimateJNI.dll ..\..\..\Bin
COPY x64\Release\IBScanUltimateJNI.dll ..\..\..\Build
COPY x64\Release\IBScanUltimateJNI.dll "..\..\..\Win Released Data\x64"
COPY x64\Release\IBScanUltimateJNI.lib "..\..\..\Win Released Data\x64"
MOVE x64\Release\IBScanUltimateJNI.dll ..\..\..\Lib
MOVE x64\Release\IBScanUltimateJNI.lib ..\..\..\Lib
RD   /S /Q x64
CD ..\..\..
GOTO :OK

:ERROR
ECHO "-------------------------------------"
ECHO "-------- BUILD ABORTED EARLY --------"
ECHO "-------------------------------------"
GOTO :EOF

:OK
ECHO "-------------------------------------"
ECHO "---------- BUILD SUCCEEDED ----------"
ECHO "-------------------------------------"

:EOF

PAUSE
