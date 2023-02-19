#pragma once
#include "pch.h"
#include "framework.h"
#pragma pack(push)
#pragma pack(1)
// 数据包类
class CPacket
{
public:
	CPacket(): m_Head(0), m_Lenth(0), m_Cmd(0), m_Sum(0)
	{

	}
	~CPacket()
	{

	}
	CPacket(const CPacket& pack)
	{
		this->m_Head = pack.m_Head;
		this->m_Lenth = pack.m_Lenth;
		this->m_Cmd = pack.m_Cmd;
		this->m_Date = pack.m_Date;
		this->m_Sum = pack.m_Sum;
		this->m_SendInfo = pack.m_SendInfo;

	}

	CPacket& operator=(const CPacket& pack)
	{
		this->m_Head = pack.m_Head;
		this->m_Lenth = pack.m_Lenth;
		this->m_Cmd = pack.m_Cmd;
		this->m_Date = pack.m_Date;
		this->m_Sum = pack.m_Sum;
		this->m_SendInfo = pack.m_SendInfo;
		return *this;
	}
	// 组包
	CPacket(int cmd, const char* data, size_t size)
	{
		this->m_Head = 0xfeff;
		this->m_Lenth = size + 4;
		this->m_Cmd = cmd;
		if (size > 0) {
			this->m_Date.resize(size);
			memcpy((void*)this->m_Date.c_str(), data, size);
			WORD sum = 0;
			for (int j = 0; j < size; ++j) {
				sum += *(BYTE*)(m_Date.c_str() + j) & 0xff;
			}
			this->m_Sum = sum;
		}
		else {
			this->m_Sum = 0;
		}

		FullSendInfo();
	
	}
	// 拆包	
	CPacket(const char* date, size_t & size)
	{
		size_t i = 0;
		for (; i < size; ++i)
		{
			if (*(WORD*)(date + i) == 0xfeff)
			{
				m_Head = 0xfeff;
				i += 2;
				break;
			}
		}
		if (i+4+2 >= size)
		{
			size = 0;
			return;
		}
		m_Lenth = *(DWORD*)(date + i); i += 4;
		if( i + m_Lenth >= size)
		{
			size = 0;
			return;
		}

		m_Cmd = *(WORD*)(date + i); i += 2;
		if (m_Lenth - 4 > 0)
		{
			m_Date.resize(m_Lenth - 4);
			memcpy((void*)m_Date.c_str(), date, m_Lenth - 4); i += m_Lenth - 4;
		}
		
		m_Sum = *(WORD*)(date + i); i += 2;
		WORD sum = 0;
		for (int j = 0; j < m_Lenth - 4; ++j) {
			sum += *(BYTE*)(m_Date.c_str() + j) & 0xff;
		}

		if (sum == m_Sum) size = i;

		size = 0;
	}
	// 将包数据填充到发送缓冲m_SendInfo
	bool FullSendInfo()
	{
		m_SendInfo.resize(m_Lenth + 6);
		const char* pTemp = m_SendInfo.c_str();
		*(WORD*)pTemp = m_Head;
		*(DWORD*)(pTemp + 2) = m_Lenth;
		*(WORD*)(pTemp + 2 + 4) = m_Cmd;

		if (m_Date.c_str() == 0) {
			if (*(WORD*)(m_SendInfo.c_str() + m_Lenth + 4) == m_Sum) return true;
			return false;
		}

		memcpy((void*)(pTemp+6+2) , m_Date.c_str(), m_Lenth - 4);
		*(WORD*)(pTemp + m_Lenth + 4) = m_Cmd;

		if (*(WORD*)(m_SendInfo.c_str() + m_Lenth + 4) == m_Sum) return true;
		return false;
	}

	// 包数据
	WORD m_Head;
	DWORD m_Lenth;
	WORD m_Cmd;
	std::string m_Date;
	WORD m_Sum;
	std::string m_SendInfo;
};
#pragma pack(pop)
// 鼠标类
struct CMouseEvent {
	CMouseEvent(){
		m_Button = -1;
		m_Action = -1;
		m_Location.x = 0;
		m_Location.y = 0;
	}

	short m_Button; //0 - 左 ， 1 - 中 ， 2 - 右 ，4 - 无
	short m_Action; // 0 - 单击 ， 1 - 双击 ， 2 - 按下 ， 4 - up ;
	POINT m_Location;
};

class CServSock
{
public:
	static CServSock* GetServSockInstance()
	{
		if (m_PServSock == NULL)
		{
			m_PServSock = new CServSock();
		}
		return m_PServSock;
	}
	// 获取文件路径
	bool GetDirectoryPath (std::string & path) {
		if (m_packet.m_Cmd == 2) {
			path = m_packet.m_Date;
			return true;

		}
		return false;
	}
	BOOL InitSock()
	{
		// 新建套接字
		m_ServSock = socket(AF_INET, SOCK_STREAM, 0);
		if (m_ServSock == INVALID_SOCKET ) return false;
		
		m_addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		m_addrSrv.sin_family = AF_INET;
		m_addrSrv.sin_port = htons(9527);
		// 绑定套接字到本地 IP 地址，端口号 6000
		if (bind(m_ServSock, (SOCKADDR*)&m_addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) return false;
		// 开始监听
		if (listen(m_ServSock, 1) == SOCKET_ERROR) return false;
		
	}
#define  BUFSIZE 4096
	// 接受消息
	//TODO : 不断组包，包不能保存
	BOOL RecvInfo()
	{
		
		if (m_ServSock == INVALID_SOCKET || m_ClientSock == INVALID_SOCKET) return false;
		char* ucBuffer = new char[BUFSIZE];
		memset(ucBuffer, 0, BUFSIZE);
		int iIndex = 0;
		while (true)
		{
			size_t iRlen = recv(m_ClientSock, (char*)ucBuffer + iIndex, BUFSIZE - iIndex, 0);
			if (iRlen <= 0) {
				delete[]ucBuffer;
				return false;
			}
			iIndex += iRlen;
			m_packet = CPacket(ucBuffer,iRlen); //TODO: 有参构造函数报错
			if (iRlen == 0) return false;
			iIndex -= iRlen;
			memmove(ucBuffer, ucBuffer + iRlen, BUFSIZE - iRlen);

		}
	
	}
	// 发送消息
	BOOL SendInfo(const CPacket& pack)
	{
		if (m_ServSock == INVALID_SOCKET || m_ClientSock == INVALID_SOCKET) return false;
		
		int iRLen = send(m_ClientSock, (const char*)pack.m_SendInfo.c_str(), pack.m_SendInfo.size(), 0);
		if (iRLen <= 0) return false;


	}
	//获取鼠标类
	bool GetMouse(CMouseEvent* mouse) {
		if (m_packet.m_Cmd == 5) {
			memcpy(mouse, m_packet.m_Date.c_str(), m_packet.m_Date.size()); //TODO: 
			return true;
		}
		return false;
	}

private:
	CServSock* operator = (const CServSock&)
	{
	}
	CServSock(const CServSock&)
	{
	}
	CServSock() : m_ServSock(INVALID_SOCKET) , m_ClientSock(INVALID_SOCKET)
	{
		memset(&m_addrSrv, 0, sizeof(m_addrSrv));
		// 加载套接字库
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(1, 1);
		// 初始化套接字库
		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			exit(-1);
		}
		if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
		{
			WSACleanup();
			exit(-1);
		}
	}
	~CServSock()
	{
		closesocket(m_ServSock);
		WSACleanup();
	}

	static void Help_Xi_Gou()
	{
		if (m_PServSock != NULL) {
		
			delete m_PServSock;
			m_PServSock = NULL;
		}

	}

	// 用于程序结束时调用析构函数
	class CHelpRelease
	{
	public:
		CHelpRelease()
		{
			
			GetServSockInstance();
		}
		~CHelpRelease()
		{
			Help_Xi_Gou();
		}

	};

	CPacket m_packet;
	static CHelpRelease m_HelpRelease;
	SOCKET m_ServSock;
	SOCKET m_ClientSock;
	SOCKADDR_IN m_addrSrv;
	static CServSock* m_PServSock; // 静态成员变量：类内声明 类外初始化。
};

