#include "TcpServer.h"

TcpServer::TcpServer()
{
    m_pConn = NULL;
    m_pLoop = NULL;
    m_pThread = NULL;

    m_LocalAddr = {};

    REGISTER_EXTERNAL_FUNC(c2s_get_client_info_request, &TcpServer::GetClientInfo, sizeof(C2S_GET_CLIENT_INFO_REQUEST));
}

TcpServer::~TcpServer()
{

}

bool TcpServer::Init(EventThread* pThread, char szIP[16], int nPort, int nPingCycle)
{
    bool bResult = false;

    LOG_PROCESS_ERROR(pThread);
    m_pThread = pThread;
    m_pThread->queueTask(new SimpleEventTask(InitConnection, this));

    nanoev_addr_init(&m_LocalAddr, AF_INET, szIP, nPort);

    printf("Server Init\n");

    bResult = true;
Exit0:
    return bResult;
}

bool TcpServer::Uninit()
{
    return true;
}

void TcpServer::GetClientInfo(char* pData, size_t nSize, int nConnIndex, int nFrame)
{

}

void TcpServer::InitConnection(nanoev_loop* pLoop, void* pContext, bool bRun)
{
    TcpServer* pSelf = (TcpServer*)pContext;

    if (bRun)
    {
        pSelf->InitConnection(pLoop);
    }
}

void TcpServer::InitConnection(nanoev_loop* pLoop)
{
    int nRetCode = 0;

    m_pLoop = pLoop;

    m_pConn = nanoev_event_new(nanoev_event_tcp, pLoop, this);
    LOG_PROCESS_ERROR(m_pConn);

    printf("Init Server Connection\n");

    nRetCode = nanoev_tcp_listen(m_pConn, &m_LocalAddr, 0);
    LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

    nRetCode = nanoev_tcp_accept(m_pConn, OnAccept, AllocUserData);
    LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

Exit0:
    return;
}

void TcpServer::OnAccept(nanoev_event* pServerConn, int nStatus, nanoev_event* pNewClientConn)
{
    TcpServer* pSelf = NULL;

    LOG_PROCESS_ERROR(pServerConn);

    LOG_PROCESS_ERROR(nStatus == NANOEV_SUCCESS);

    pSelf = (TcpServer*)nanoev_event_userdata(pServerConn);
    LOG_PROCESS_ERROR(pSelf);

    pSelf->OnAccept(pServerConn, pNewClientConn);
Exit0:
    return;
}

void TcpServer::OnAccept(nanoev_event* pServerCon, nanoev_event* pNewClientConn)
{
    int            nRetCode = 0;
    unsigned short nPort = 0;
    nanoev_client* pClient = NULL;
    char           szClientIP[46];
    nanoev_addr    ClientAddr;
    nanoev_timeval TimeOut;

    LOG_PROCESS_ERROR(pServerCon);
    LOG_PROCESS_ERROR(pNewClientConn);

    printf("Accept new client");

    pClient = (nanoev_client*)nanoev_event_userdata(pNewClientConn);
    pClient->pClientConn = pNewClientConn;
    pClient->pClass = this;

    nRetCode = nanoev_tcp_addr(pNewClientConn, 0, &ClientAddr);
    if (nRetCode != NANOEV_SUCCESS)
    {
        printf("nanoev_tcp_addr failed, code = %u\n", nRetCode);
        goto Exit0;
    }

    nanoev_addr_get_ip(&ClientAddr, szClientIP, sizeof(szClientIP));
    nanoev_addr_get_port(&ClientAddr, &nPort);
    printf("Client %s:%d connected\n", szClientIP, (int)nPort);

    nRetCode = EnsureInBuf(pClient, CLIENT_INPUT_BUFFER_SIZE);
    LOG_PROCESS_ERROR(nRetCode);

    nRetCode = nanoev_tcp_read(pNewClientConn, pClient->pszInBuf, pClient->nInBufCapacity, OnRead);
    LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

    TimeOut.tv_sec = TIMEOUT_SECONDS;
    TimeOut.tv_usec = 0;

    //nRetCode = nanoev_timer_add(pClient->pTimer, TimeOut, 0, OnTimer);
    //LOG_PROCESS_ERROR (nRetCode == NANOEV_SUCCESS);

    pClient->nState = ecssRecv;

Exit0:
    return;
}

void TcpServer::OnRead(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes)
{
    nanoev_client* pClient = NULL;
    TcpServer* pSelf = NULL;

    LOG_PROCESS_ERROR(pClientConn);

    LOG_PROCESS_ERROR(nStatus == NANOEV_SUCCESS);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);

    pSelf = (TcpServer*)(pClient->pClass);
    LOG_PROCESS_ERROR(pSelf);

    pSelf->OnRead(pClientConn, pBuf, nBytes);
Exit0:
    return;
}

void TcpServer::OnRead(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes)
{
    int              nRetCode    = 0;
    int              nRemainSize = 0;
    nanoev_client*   pClient     = NULL;
    PROTOCOL_HEADER* pHeader     = (PROTOCOL_HEADER*)pBuf;
    nanoev_timeval   TimeOut;
    const char*      pszMsg      = "\n Client name = ty\n Client ID = 1\n ";
    unsigned char*   puszMsg;

    puszMsg = (unsigned char*)malloc(strlen(pszMsg) + 1);
    LOG_PROCESS_ERROR(puszMsg);

    memcpy(puszMsg, pszMsg, strlen(pszMsg) + 1);

    LOG_PROCESS_ERROR(pClientConn);
    LOG_PROCESS_ERROR(pBuf);
    LOG_PROCESS_ERROR(nBytes > 0);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);
    //LOG_PROCESS_ERROR(pClient->nState == ecssRecv);

    //nanoev_timer_del(pClient->pTimer);

    printf("Read successful, content = %s", (char*)pBuf);

    pClient->nInBufSize += nBytes;
    //nRemainSize = GetRemainSize(pClient);

    if (nRemainSize > 0)
    {
        nRetCode = EnsureInBuf(pClient, pClient->nInBufSize + nRemainSize);
        LOG_PROCESS_ERROR(nRetCode);

        nRetCode = nanoev_tcp_read(pClientConn, pClient->pszInBuf + pClient->nInBufSize, nRemainSize, OnRead);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        TimeOut.tv_sec = TIMEOUT_SECONDS;
        TimeOut.tv_usec = 0;

        //nRetCode = nanoev_timer_add(pClient->pTimer, TimeOut, 0, OnTimer);
        //LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);
    }
    else
    {
        LOG_PROCESS_ERROR(pClient->pszInBuf);
        LOG_PROCESS_ERROR(pClient->nInBufSize);
        nRetCode = WriteToBuf(pClient, puszMsg);
        LOG_PROCESS_ERROR(nRetCode == 0);

        pClient->nOutBufSent = 0;
        nRetCode = nanoev_tcp_write(pClientConn, puszMsg, pClient->nOutBufSize, OnWrite);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        TimeOut.tv_sec = TIMEOUT_SECONDS;
        TimeOut.tv_usec = 0;
        //nRetCode = nanoev_timer_add(pClient->pTimer, TimeOut, 0, OnTimer);
        //LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        pClient->nState = ecssSend;
    }

Exit0:
    return;
}

void TcpServer::OnWrite(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes)
{
    nanoev_client* pClient = NULL;
    TcpServer*     pSelf   = NULL;

    LOG_PROCESS_ERROR(pClientConn);

    LOG_PROCESS_ERROR(nStatus == NANOEV_SUCCESS);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);

    pSelf = (TcpServer*)(pClient->pClass);
    LOG_PROCESS_ERROR(pSelf);

    pSelf->OnWrite(pClientConn, pBuf, nBytes);
Exit0:
    return;
}

void TcpServer::OnWrite(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes)
{
    int              nRetCode    = 0;
    int              nRemainSize = 0;
    nanoev_client*   pClient     = NULL;
    PROTOCOL_HEADER* pHeader     = (PROTOCOL_HEADER*)pBuf;
    nanoev_timeval   TimeOut;

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient->nState == ecssSend);

    //nanoev_timer_del(pClient->pTimer);

    LOG_PROCESS_ERROR(nBytes);

    pClient->nOutBufSent += nBytes;

    if (pClient->nOutBufSent < pClient->nOutBufSize)
    {
        nRetCode = nanoev_tcp_write(
            pClientConn, pClient->pszOutBuf + pClient->nOutBufSent,
            pClient->nOutBufSize - pClient->nOutBufSent, OnWrite
        );
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        TimeOut.tv_sec = TIMEOUT_SECONDS;
        TimeOut.tv_usec = 0;
        nRetCode = nanoev_timer_add(pClient->pTimer, TimeOut, 0, OnTimer);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);
    }
    else
    {
        pClient->nInBufSize  = 0;
        pClient->nOutBufSize = 0;
        pClient->nOutBufSent = 0;

        nRetCode = nanoev_tcp_read(pClientConn, pClient->pszInBuf, pClient->nInBufCapacity, OnRead);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        TimeOut.tv_sec = TIMEOUT_SECONDS;
        TimeOut.tv_usec = 0;
        //nRetCode = nanoev_timer_add(pClient->pTimer, TimeOut, 0, OnTimer);
        //LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        pClient->nState = ecssRecv;
    }

Exit0:
    return;
}

void TcpServer::OnTimer(nanoev_event* pTimer)
{
    nanoev_client* pClient = (nanoev_client*)nanoev_event_userdata(pTimer);

    printf("TCP Reading/Writing Timeout\n");

    nanoev_event_free(pClient->pClientConn);
    ClientFree(pClient);

    return;
}

void* TcpServer::AllocUserData(void* pContext, void* pUserData)
{
    void* pResult       = NULL;
    GlobalData* pGlobal = (GlobalData*)pContext;

    LOG_PROCESS_ERROR(pGlobal);

    if (!pUserData)
    {
        pResult = ClientNew(pGlobal->loop);
    }
    else
    {
        ClientFree((nanoev_client*)pUserData);
    }

Exit0:
    return pResult;
}
