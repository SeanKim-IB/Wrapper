APP_MODULES = usb1.0
APP_ABI    := armeabi armeabi-v7a arm64-v8a x86 x86_64
APP_STL    := stlport_static

ifeq ($(NDK_DEBUG), 1)
    APP_OPTIM = debug
    APP_CFLAGS = -g -O0
else
    APP_OPTIM = release
    APP_CFLAGS = -O3
endif

