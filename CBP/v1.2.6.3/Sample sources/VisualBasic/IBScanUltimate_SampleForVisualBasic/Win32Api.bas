Attribute VB_Name = "Module5"
Option Explicit

Public Const CB_FINDSTRING = &H14C


Public Declare Function GetClientRect Lib "user32" _
        (ByVal hwnd As Long, _
        ByRef lpRect As IBSU_RECT) As Long

Public Declare Function SendMessage Lib "user32" Alias "SendMessageA" _
        (ByVal hwnd As Long, _
        ByVal wMsg As Long, _
        ByVal wParam As Long, _
        lParam As Any) As Long

Public Declare Sub Sleep Lib "kernel32" _
        (ByVal dwMilliseconds As Long)

Public Declare Function Beep Lib "kernel32" _
        (ByVal dwFreq As Long, _
        ByVal dwDuration As Long) As Long
        
Public Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" _
        (Destination As Any, ByRef Source As Any, ByVal Length As Long)


'/// Select directory
Public Const BIF_RETURNONLYFSDIRS = 1
Public Const BIF_DONTGOBELOWDOMAIN = 2
Public Const MAX_PATH = 260

Public Declare Function SHBrowseForFolder Lib "shell32" _
        (lpbi As BrowseInfo) As Long

Public Declare Function SHGetPathFromIDList Lib "shell32" _
        (ByVal pidList As Long, _
        ByVal lpBuffer As String) As Long

Public Declare Function lstrcat Lib "kernel32" Alias "lstrcatA" _
        (ByVal lpString1 As String, ByVal _
        lpString2 As String) As Long

Public Type BrowseInfo
    hWndOwner      As Long
    pIDLRoot       As Long
    pszDisplayName As Long
    lpszTitle      As Long
    ulFlags        As Long
    lpfnCallback   As Long
    lParam         As Long
    iImage         As Long
End Type





