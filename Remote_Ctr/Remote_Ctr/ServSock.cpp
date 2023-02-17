#include "pch.h"
#include "ServSock.h"

CServSock* CServSock::m_PServSock = NULL; // 类外初始化要加类型说明符

CServSock::CHelpRelease CServSock::m_HelpRelease; 