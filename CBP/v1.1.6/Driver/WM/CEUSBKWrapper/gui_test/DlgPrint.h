int WDlgPrintf(const WCHAR *format, ... );
int DlgPrintf(const char *format, ... );
BOOL handleCommand(char line[]);
void createDeviceNotificationQueue(HANDLE& queue, HANDLE& notif);
void destroyDeviceNotificationQueue(HANDLE& queue, HANDLE& notif);
void checkDeviceNotificationQueue(HANDLE& queue);
void printMenu();

#ifdef USE_WINDOW_FOR_UI
#pragma message ("USE_WINDOW_FOR_UI is set. A dialog box will be used for all UI")
#undef printf
#define printf DlgPrintf
#undef wprintf
#define wprintf WDlgPrintf
#endif 