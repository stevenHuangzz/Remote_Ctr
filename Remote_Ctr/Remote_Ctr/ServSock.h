#pragma once
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

	BOOL InitSock()
	{
		// �½��׽���
		m_ServSock = socket(AF_INET, SOCK_STREAM, 0);
		if (m_ServSock == INVALID_SOCKET ) return false;
		
		m_addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		m_addrSrv.sin_family = AF_INET;
		m_addrSrv.sin_port = htons(9527);
		// ���׽��ֵ����� IP ��ַ���˿ں� 6000
		if (bind(m_ServSock, (SOCKADDR*)&m_addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) return false;
		// ��ʼ����
		if (listen(m_ServSock, 1) == SOCKET_ERROR) return false;
		
	}

	// ������Ϣ
	BOOL RecvInfo();
	// ������Ϣ
	BOOL SendInfo();

private:
	CServSock& operator = (const CServSock&)
	{
	}
	CServSock(const CServSock&)
	{
	}
	CServSock()
	{
		// �����׽��ֿ�
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(1, 1);
		// ��ʼ���׽��ֿ�
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

	// ���ڳ������ʱ������������
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

	static CHelpRelease m_HelpRelease;
	SOCKET m_ServSock;
	SOCKET m_ClientSock;
	SOCKADDR_IN m_addrSrv;
	static CServSock* m_PServSock; // ��̬��Ա�������������� �����ʼ����
};

