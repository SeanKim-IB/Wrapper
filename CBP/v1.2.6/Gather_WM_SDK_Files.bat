@echo off
echo Gathering Files for WM SDK
echo. The retail build of IBScanUltiamte_WINCE_VS2008.sln must be built prior to running this script.
echo. WARNING: The debug build overwrites the IBScanUltimate_ARM.lib and IBScanUltimate_ARM.dll's so be sure
echo. clean retail build is done prior to running this batch file
pause
@echo on


set sdk=build\WM6_SDK
set rel=build\WM6\Release

rem create WM6_SDK directory tree
rmdir /s /q build\WM6_SDK >nul 2>nul
md %sdk%
md %sdk%\Bin
md %sdk%\Bin\WM6
md %sdk%\Documents
md %sdk%\Driver
md %sdk%\Driver\WM6
md %sdk%\Include
md %sdk%\Lib
md %sdk%\Lib\WM6
md "%sdk%\WM6 Sample Sources"
md "%sdk%\WM6 Sample Sources\VisualC++"

rem Copy bin files
copy %rel%\IBSimpleScanWM.exe %sdk%\bin\WM6
copy %rel%\SimpleScanWMInstall.cab %sdk%\bin\WM6
copy %rel%\SimpleScanWMInstall.inf %sdk%\bin\WM6
copy %rel%\ceusbkwrapper.dll %sdk%\bin\WM6
copy %rel%\libusb-1.0.dll %sdk%\bin\WM6
copy %rel%\IBScanUltimate_WM6.Dll %sdk%\bin\WM6


rem Copy Documents
copy "Documents\IBScanUltimate API Manual for C on Windows Mobile.pdf"  %sdk%\Documents 
copy "Documents\IBScanUltimate Windows Mobile Getting Started Guide.pdf" %sdk%\Documents
copy "Documents\IBScanUltimate API Manual for C.pdf" %sdk%\Documents
copy "Documents\IBScanUltimate Version History.pdf" %sdk%\Documents

rem Copy Drivers
copy %rel%\ceusbkwrapperdrv.dll %sdk%\Driver\WM6
copy %rel%\IBSU_InstallDrivers.cab %sdk%\Driver\WM6
copy %rel%\IBSU_InstallDrivers.inf %sdk%\Driver\WM6

rem Copy include files
robocopy include %sdk%\include IbScanUltimate*.h
robocopy include %sdk%\include ReservedApi*.h


rem Copy Lib Files
copy Lib\WM6\IBScanUltimate_WM6.lib %sdk%\lib\WM6


rem Copy Sample Sources
robocopy "Wince Sample Sources\VisualC++\IBSU_SimpleScanWM" "%sdk%\WM6 Sample Sources\VisualC++\IBSU_SimpleScanWM" *.cpp *.h *.bmp *.ico *.rc2 *.rc *.vcproj *.sln /S
robocopy "Wince Sample Sources\CSharp\IBSU_CSharpWM" "%sdk%\WM6 Sample Sources\CSharp\IBSU_CSharpWM" *.cs *.resx *.csproj /S


rem Zip up Public source

dir /s /b "Windows Mobile 6 Professional SDK (ARMV4I)" >tmp
"c:\program files (x86)\WinRar\WinRar.exe" A -r -x@tmp %sdk%\Driver\WM6\CEUSBKWrapper_Source.zip Driver\WM\CEUSBKWrapper
del tmp

"c:\program files (x86)\WinRar\WinRar.exe" A -r -x*\ARMV4I\ %sdk%\Driver\WM6\Libusbx_Source.zip Driver\WM\libusbx

cd %sdk%
del ..\IBScanUltimate_WM6.zip >nul 2>nul
"c:\program files (x86)\WinRar\WinRar.exe" A -r ..\IBScanUltimate_WM6.zip *.*
cd ..\..

