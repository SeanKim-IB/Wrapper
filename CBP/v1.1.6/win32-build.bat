REM ################################################################################
REM #
REM # Initial directory should have 'IBScanUltimate', 'IBScanFwUpdater',
REM # 'Sample sources' folders.
REM #
REM #
REM ################################################################################

REM #
REM # Set up build.
REM #

SET   JAVA_HOME=C:\Program Files (x86)\Java\jdk1.6.0_43\bin\
SET   VS_HOME=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\
SET   VB6_HOME=C:\Program Files (x86)\Microsoft Visual Studio\VB98\
SET   DELPHI7_HOME=C:\Program Files (x86)\Borland\Delphi7\Bin\
SET   QT_HOME=C:\QT\4.8.6-static\bin\
SET   QT_COMPILER_HOME=C:\QT\mingw32\bin\
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
IF EXIST "Win Released Data\include" (
	RD   /S /Q "Win Released Data\include"
)
IF EXIST "Win Released Data\x86" (
	RD   /S /Q "Win Released Data\x86"
)
MKDIR "Win Released Data\include"
MKDIR "Win Released Data\x86"

SET   VCPROJ_EXT=vcproj
SET   CSPROJ_EXT=csproj
SET   VBPROJ_EXT=vbproj
SET   VB6PROJ_EXT=vbp
SET   DELPHI7PROJ_EXT=dpr

REM #
REM # Copy  common files to Bin, Build and etc
REM #
COPY                CommonFiles\x86\winusb.dll "Sample sources\VisualBasic\IBScanUltimate_SampleForVisualBasic"
COPY                CommonFiles\x86\winusb.dll Bin
COPY                CommonFiles\x86\winusb.dll Build
COPY                CommonFiles\IBSU_FunctionTesterForJava.bat Bin\IBSU_FunctionTesterForJava.bat
COPY                CommonFiles\IBScanUltimate_SampleForJava.bat Bin\IBScanUltimate_SampleForJava.bat
COPY                CommonFiles\FirmwareWriterSetting.ini Bin
COPY                CommonFiles\FirmwareWriterSetting.ini Bin
COPY                IBScanUltimate.sln CommonFiles\IBScanUltimate.sln
COPY                CommonFiles\IBScanUltimate_ForBuild.sln IBScanUltimate.sln
COPY                ExternalLib\x86\IBScanMatcher.dll Bin
COPY                Plugin\IBScanNFIQ2\OpenCV_Libs\win32\opencv_core.dll Plugin\Bin
COPY                Plugin\IBScanNFIQ2\OpenCV_Libs\win32\opencv_imgproc.dll Plugin\Bin
COPY                Plugin\IBScanNFIQ2\OpenCV_Libs\win32\opencv_ml.dll Plugin\Bin

REM #
REM # Copy  header files to Win Released Data
REM #
robocopy            "include" "Win Released Data\include" *.h /S
robocopy            "Sample sources\CSharp\IBScanUltimate_SampleForCSharp\include" "Win Released Data\include" *.cs /S
robocopy            "Sample sources\Delphi\IBScanUltimate_SampleForDelphi" "Win Released Data\include" IBScanUltimate.pas IBScanUltimateApi*.pas /S
robocopy            "Sample sources\VB_Net\IBScanUltimate_SampleForVBNet\include" "Win Released Data\include" *.vb /S
robocopy            "Sample sources\VisualBasic\IBScanUltimate_SampleForVisualBasic" "Win Released Data\include" IBScanUltimate.bas IBScanUltimateApi*.bas /S

REM #
REM # Build IBScanUltimate DLL
REM #
SET                 SAMPLENAME=IBScanUltimate
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
IF NOT EXIST "Build\%SAMPLENAME%.dll" (
    ECHO "Build\%SAMPLENAME%.dll" does not exist
	GOTO :ERROR
)
IF NOT EXIST "Lib\%SAMPLENAME%.lib" (
    ECHO "Lib\%SAMPLENAME%.lib" does not exist
	GOTO :ERROR
)
COPY                Build\%SAMPLENAME%.dll Lib
COPY                Build\%SAMPLENAME%.dll "Sample sources\VisualBasic\IBScanUltimate_SampleForVisualBasic"
COPY                Build\%SAMPLENAME%.dll Bin
COPY                Build\%SAMPLENAME%.dll "Win Released Data\x86"
COPY                Lib\%SAMPLENAME%.lib "Win Released Data\x86"
COPY                Build\%SAMPLENAME%.dll "Plugin\Lib"
COPY                Build\%SAMPLENAME%.dll "Plugin\Bin"
COPY                Build\%SAMPLENAME%.dll "Sample sources\Python\TestScanForPython"
COPY                Build\%SAMPLENAME%.dll "Sample sources\Python\IBScanUltimate_SampleForPySide"

REM #
REM # Build IBUsbFwUpdater
REM #
SET                 SAMPLENAME=IBUsbFwUpdater
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBSU_DynamicLinkedSampleForVC
REM #
SET                 SAMPLENAME=IBSU_DynamicLinkedSampleForVC
SET                 SAMPLEDIR=Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
"%VS_HOME%devenv.exe" IBScanNFIQ2.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
COPY                Plugin\Build\%SAMPLENAME%.dll "Win Released Data\x86"
COPY                Plugin\Build\%SAMPLENAME%.lib "Win Released Data\x86"

REM #
REM # Build IBScanNFIQ2_SampleForVC
REM #
SET                 SAMPLENAME=IBScanNFIQ2_SampleForVC
SET                 SAMPLEDIR=Plugin\IBScanNFIQ2\Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanNFIQ2.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release" /project "%SAMPLEDIR%\%SAMPLENAME%.%CSPROJ_EXT%" /projectconfig "Release|AnyCPU"
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
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release" /project "%SAMPLEDIR%\%SAMPLENAME%.%VBPROJ_EXT%" /projectconfig "Release|AnyCPU"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
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
RD            /S /Q "%SAMPLEDIR%\Release"
"%VS_HOME%devenv.exe" IBScanUltimate.sln /build "Release|win32" /project "%SAMPLEDIR%\%SAMPLENAME%.%VCPROJ_EXT%" /projectconfig "Release|win32"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_SampleForVisualBasic
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForVB
SET                 SAMPLEDIR=Sample sources\VisualBasic\IBScanUltimate_SampleForVisualBasic
"%VB6_HOME%vb6.exe" /make "%SAMPLEDIR%\%SAMPLENAME%.%VB6PROJ_EXT%"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_SampleForDelphi
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForDelphi
SET                 SAMPLEDIR=Sample sources\Delphi\IBScanUltimate_SampleForDelphi
"%DELPHI7_HOME%dcc32.exe" -Q -B "%SAMPLEDIR%\%SAMPLENAME%.%DELPHI7PROJ_EXT%" -E"Build"
IF NOT EXIST "Build\%SAMPLENAME%.exe" (
    ECHO "Build\%SAMPLENAME%.exe" does not exist
	GOTO :ERROR
)
MOVE                Build\%SAMPLENAME%.exe Bin

REM #
REM # Build IBScanUltimate_SampleForQT
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForQT
SET                 SAMPLEDIR=Sample sources\QT\IBScanUltimate_SampleForQT
CD    %SAMPLEDIR%
DEL   /Q "Makefile.*"
"%QT_HOME%qmake.exe" "%SAMPLENAME%.pro" -r -spec win32-g++ CONFIG+=release
"%QT_COMPILER_HOME%mingw32-make" -f Makefile.release
DEL   /Q "Makefile.*"
CD    ..\..\..
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
FOR %%A in (IBSU_NewFunctionTesterForVC IBSU_FunctionTesterForVC IBScanUltimate_TenScanSampleForVC IBSU_NonCallbackSampleForVC IBScanUltimate_SampleForVC IBSU_ConsoleSampleForVC IBSU_Holmes_DebuggerForVC IBSU_MultipleScanSampleForVC IBScanUltimate_SalesDemo) DO (
	IF EXIST "Sample sources\VisualC++\%%A\Release" (
		RD /S /Q "Sample sources\VisualC++\%%A\Release"
	)
	IF EXIST "Sample sources\VisualC++\%%A\Build" (
		RD /S /Q "Sample sources\VisualC++\%%A\Build"
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
RD            /S /Q "%SAMPLEDIR%\Release"

REM #
REM # Clean NBIS library
REM #
FOR %%A in (bozorth3 commonnbis ioutil jpegl mindtct nfiq nfseg pcasys wsq) DO (
	IF EXIST "IBScanUltimate\NBIS_Libs\src\VSProject\VisualStudio2008\%%A\Release" (
		RD /S /Q "IBScanUltimate\NBIS_Libs\src\VSProject\VisualStudio2008\%%A\Release"
	)
)

REM #
REM # Clean IBUsbFwUpdater
REM #
SET                 SAMPLENAME=IBUsbFwUpdater
SET                 SAMPLEDIR=%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"

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
RD            /S /Q "%SAMPLEDIR%\Release"
IF EXIST "%SAMPLEDIR%\Debug" (
	RD        /S /Q "%SAMPLEDIR%\Debug"
)

REM #
REM # Clean IBScanUltimate_SampleForQT
REM #
SET                 SAMPLENAME=IBScanUltimate_SampleForQT
SET                 SAMPLEDIR=Sample sources\QT\IBScanUltimate_SampleForQT
RD            /S /Q "%SAMPLEDIR%\Release"
IF EXIST "%SAMPLEDIR%\Debug" (
	RD        /S /Q "%SAMPLEDIR%\Debug"
)

REM #
REM # Clean IBScanNFIQ2 DLL
REM #
SET                 SAMPLENAME=IBScanNFIQ2
SET                 SAMPLEDIR=Plugin\%SAMPLENAME%
DEL              /Q Plugin\Lib\%SAMPLENAME%.exp
RD            /S /Q "%SAMPLEDIR%\Release"
IF EXIST "%SAMPLEDIR%\Debug" (
	RD        /S /Q "%SAMPLEDIR%\Debug"
)

REM #
REM # Clean IBScanNFIQ2_SampleForVC
REM #
SET                 SAMPLENAME=IBScanNFIQ2_SampleForVC
SET                 SAMPLEDIR=Plugin\Sample sources\VisualC++\%SAMPLENAME%
RD            /S /Q "%SAMPLEDIR%\Release"
IF EXIST "%SAMPLEDIR%\Debug" (
	RD        /S /Q "%SAMPLEDIR%\Debug"
)

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
COPY  IBScanCommon.jar "..\..\..\Win Released Data\x86"
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
COPY  IBScanUltimate.jar "..\..\..\Win Released Data\x86"
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
RD   /S /Q Release
"%VS_HOME%devenv.exe" IBScanUltimateJNI.sln /rebuild "Release|win32" /project IBScanUltimateJNI.%VCPROJ_EXT% /projectconfig "Release|win32"
IF NOT EXIST "Release\IBScanUltimateJNI.dll" (
    CD ..\..\..
    ECHO "Release\IBScanUltimateJNI.dll" does not exist
	GOTO :ERROR
)
IF NOT EXIST "Release\IBScanUltimateJNI.lib" (
    CD ..\..\..
    ECHO "Release\IBScanUltimateJNI.lib" does not exist
	GOTO :ERROR
)
COPY Release\IBScanUltimateJNI.dll ..\..\..\Bin
COPY Release\IBScanUltimateJNI.dll ..\..\..\Build
COPY Release\IBScanUltimateJNI.dll "..\..\..\Win Released Data\x86"
COPY Release\IBScanUltimateJNI.lib "..\..\..\Win Released Data\x86"
MOVE Release\IBScanUltimateJNI.dll ..\..\..\Lib
MOVE Release\IBScanUltimateJNI.lib ..\..\..\Lib
RD   /S /Q Release
CD ..\..\..
GOTO :OK

:ERROR
COPY                CommonFiles\IBScanUltimate.sln IBScanUltimate.sln
ECHO "-------------------------------------"
ECHO "-------- BUILD ABORTED EARLY --------"
ECHO "-------------------------------------"
GOTO :EOF

:OK
COPY                CommonFiles\IBScanUltimate.sln IBScanUltimate.sln
ECHO "-------------------------------------"
ECHO "---------- BUILD SUCCEEDED ----------"
ECHO "-------------------------------------"

:EOF

PAUSE
