// Remote_Ctr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "Remote_Ctr.h"
#include<direct.h>
#include "ServSock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup") 
// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
// 文件结构体
struct FileInfo
{
    FileInfo() {
        memset(name, 0, sizeof(name));
        IsInvalid = false;
        IsFolder = false;
        HasFinished = false;
    }
    char name[260];
    bool IsInvalid; // 文件或文件夹是否有效 有（0） 无（1）
    bool IsFolder; // 是否为文件夹 是（1）否（0）
    bool HasFinished; // 是否为最后一个文件 是（1） 否（0）

}FILEINFO,*PFILEINFO;

void DumpInfo(const char* data, size_t size)
{
    std::string strDump;
    
    for (int i = 0; i < size; ++i) {
        char cBuf[4] = "";
        snprintf(cBuf,4, "%02X", data[i] & 0xff);
        strDump += cBuf;
    }
    OutputDebugStringA(strDump.c_str());

}

bool GetDiskInfo()
{
    std::string strInfo;
    int iIndex = 0;
    for (int i = 1; i <= 26; ++i) {
        if (_chdrive(i) == 0) {
            strInfo += 'A' + i - 1;
            ++iIndex;
        }

    }
    if (iIndex == 0) return false; 
    CPacket TempPkt(1,strInfo.c_str(), strInfo.size());
    //TODO: 错误处理
    TempPkt.FullSendInfo();
    DumpInfo(TempPkt.m_SendInfo.c_str(), TempPkt.m_SendInfo.size());
    //CServSock::GetServSockInstance()->SendInfo(TempPkt);

    return true;
}
// 获取本地文件信息
#include<io.h>
int GetDirectoryInfo() {
    std::string strPath;
    if (CServSock::GetServSockInstance()->GetDirectoryPath(strPath) == false) {
        OutputDebugString(_T("文件路径获取失败!\n"));
        return -1;
    }

    if (_chdir(strPath.c_str()) != 0) {
        FileInfo TempFile;
        memcpy(TempFile.name, strPath.c_str(), strPath.size());
        TempFile.IsInvalid = true;
        TempFile.HasFinished = true;
        CPacket TempPack(2, (char*)&TempFile, sizeof(TempFile));
        CServSock::GetServSockInstance()->SendInfo(TempPack);
		OutputDebugString(_T("无法访问文件! 权限不足 \n"));
		return -2;

   }
    
    _finddata_t fData;
    int fHandl = _findfirst("*", &fData);
    if (fHandl == -1) {
		OutputDebugString(_T("找不到任何文件\n"));
		return -3;
    }

    do 
    {
        FileInfo TempFile;
        memcpy(TempFile.name, fData.name, sizeof(fData.name));
        TempFile.IsFolder = ((fData.attrib & _A_SUBDIR) != 0);
		CPacket TempPack(2, (char*)&TempFile, sizeof(TempFile));
		CServSock::GetServSockInstance()->SendInfo(TempPack);

    } while (_findnext(fHandl, &fData) == 0);

	FileInfo TempFile;
	TempFile.IsInvalid = true;
	TempFile.HasFinished = true;
	CPacket TempPack(2, (char*)&TempFile, sizeof(TempFile));
	CServSock::GetServSockInstance()->SendInfo(TempPack);

    return 0;
}
// 运行文件
int RunFile() {
	std::string strPath;
	if (CServSock::GetServSockInstance()->GetDirectoryPath(strPath) == false) {
		OutputDebugString(_T("文件路径获取失败!\n"));
		return -1;
	}
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	CPacket TempPack(3, NULL, 0);
	CServSock::GetServSockInstance()->SendInfo(TempPack);
    return 0;
}
//下载文件
int DownloadFile() {
	std::string strPath;
	if (CServSock::GetServSockInstance()->GetDirectoryPath(strPath) == false) {
		OutputDebugString(_T("文件路径获取失败!\n"));
		return -1;
	}
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    if (err != 0) {
		CPacket TempPack(4, NULL, 0);
		CServSock::GetServSockInstance()->SendInfo(TempPack);
		OutputDebugString(_T("文件打开失败!\n"));
		return -2;
    }
    if (pFile != NULL) {
        // 将文件大小发送出去
        fseek(pFile, 0, SEEK_END);
        long long llRet = _ftelli64(pFile);
        fseek(pFile, 0, SEEK_SET);
		CPacket TempPack(4, (char*)&llRet, sizeof(long long));
		CServSock::GetServSockInstance()->SendInfo(TempPack);

        BYTE ucBuf[1024] = "";
        size_t ullRet = 0;
        do 
        {
            ullRet = fread(ucBuf, 1, 1024, pFile);
			CPacket TempPack(4, (char*)ucBuf, ullRet);
			CServSock::GetServSockInstance()->SendInfo(TempPack);

        } while (ullRet < 1024);
		fclose(pFile);
		return 0;

    }
	CPacket TempPack(4, NULL, 0);
	CServSock::GetServSockInstance()->SendInfo(TempPack);
    fclose(pFile);
    return -3;
}

int MouseMove() {
    CMouseEvent MyMouse;
    if (CServSock::GetServSockInstance()->GetMouse(&MyMouse) == false) {
		OutputDebugString(_T("鼠标信息获取失败!\n"));
		return -1;
    }

    WORD Flag = 0;

    switch (MyMouse.m_Button)
    {
    case 0: Flag = 0; // 左
        break;          
    case 1: Flag = 1;// 中
        break;
    case 2: Flag = 2;// 右
        break;
    case 4: Flag = 4; //无（纯运动）
        break;
    default:
        break;
    }
    if (Flag != 4) SetCursorPos(MyMouse.m_Location.x, MyMouse.m_Location.y);

    switch (MyMouse.m_Action)
    {
	case 0: Flag |= 0x00; // 单击
		break;
	case 1: Flag = 0x10;// 双击
		break;
	case 2: Flag = 0x20;// 按下
		break;
	case 4: Flag = 0x40; //up
		break;
    default:
        break;
    }

    switch (Flag)
    {
    case 0x10: mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
               mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
	case 0x00: mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		       mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case 0x20: mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
        break;
    case 0x40:mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case 0x11:mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
              mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
	case 0x01:mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		      mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case 0x21:mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
        break;
    case 0x41: mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        break;
	case 0x12:mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
              mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
	case 0x02:mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		      mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0x22:mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0x42:mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
    case 0x04:mouse_event(MOUSEEVENTF_MOVE, MyMouse.m_Location.x, MyMouse.m_Location.y,  0, GetMessageExtraInfo()); // 纯移动
        break;
    default:
        break;
    }

	CPacket TempPack(5, NULL, 0);
	CServSock::GetServSockInstance()->SendInfo(TempPack);
    return 0;
}
// 获取和发送屏幕图片
#include<atlimage.h> 
int SendScrean() {
    CImage MyStream;
    HDC hStream = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hStream, BITSPIXEL);
    int nHetght = GetDeviceCaps(hStream, VERTRES);
    int nWidth = GetDeviceCaps(hStream, HORZRES);
    MyStream.Create(nWidth, nHetght, nBitPerPixel);
    BitBlt(MyStream.GetDC(), 0, 0, 1920, 1080, hStream, 0, 0, SRCCOPY);
    ReleaseDC(NULL,hStream);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);

    return 0;
}

//锁机
#include "LockDlg.h"
CLockDlg myLockDlg; //锁屏窗口
unsigned int ThreadID; //线程ID

unsigned  __stdcall LockThread(void* arg) {

    TRACE("threadID:%d\n", GetCurrentThreadId());
	RECT ClipRect;
	myLockDlg.Create(IDD_DIALOG_INFO, NULL);
	myLockDlg.ShowWindow(SW_SHOWNORMAL);
	//将窗口置顶
	myLockDlg.SetWindowPos(&(myLockDlg.wndTopMost), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	//将窗口设置成全屏
	ClipRect.left = 0;
	ClipRect.top = 0;
	ClipRect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	ClipRect.bottom = 1.1 * GetSystemMetrics(SM_CYFULLSCREEN);
	myLockDlg.MoveWindow(&ClipRect);
	// 隐藏鼠标
	ShowCursor(false);
	// 限制鼠标的移动
	ClipRect.left = 0;
	ClipRect.top = 0;
	ClipRect.right = 1;
	ClipRect.bottom = 1;
	ClipCursor(&ClipRect);
	//隐藏WND 任务栏
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	//消息循环。 消息循环是对话窗的基础，消息循环中的消息依赖于对应的线程。
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_KEYDOWN) {
			TRACE("msg:%08X wparam:%08X  lparam: %08X \n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x1b) {
				break;
			}
		}

	}
	// 显示鼠标
	ShowCursor(true);
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
	myLockDlg.DestroyWindow();
    _endthreadex(0);
    return 0;

}

int LockMachine() {
    if (myLockDlg.m_hWnd == NULL || myLockDlg.m_hWnd == INVALID_HANDLE_VALUE) {
		_beginthreadex(NULL, 0, LockThread, NULL, 0, &ThreadID);
		TRACE("threadID:%d\n", ThreadID);

    }
	CPacket TempPack(7, NULL, 0);
	CServSock::GetServSockInstance()->SendInfo(TempPack);
    return 0;
}

// 解锁
int UnLockMachine() {
    //向另外一个线程发送消息只能用这个函数消息泵才能收到
    PostThreadMessage(ThreadID, WM_KEYDOWN, 0x1b, 0x010001); 
	CPacket TempPack(8, NULL, 0);
	CServSock::GetServSockInstance()->SendInfo(TempPack);
	return 0;
    
}


int main()  
{
    int nRetCode = 0;    

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            int cmd = 7;
            switch (cmd)
            {
            case 1:GetDiskInfo();
                break;
            case 2:GetDirectoryInfo();
                break;
            case 3:RunFile();
                break;
            case 4:DownloadFile();
                break;
            case 5:MouseMove();
                break;
            case 6:SendScrean();
                break;
            case 7:LockMachine();
                
                break;
            case 8:UnLockMachine();
                break;
            default:
                break;
            }

			/*if (myLockDlg.m_hWnd != NULL && myLockDlg.m_hWnd != INVALID_HANDLE_VALUE) {
                Sleep(100);

			}*/
            while (true);
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
