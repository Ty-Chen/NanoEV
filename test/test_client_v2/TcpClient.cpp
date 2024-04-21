#include "TcpClient.h"

TcpClient::TcpClient()
{
    m_pConn = NULL;
    m_pLoop = NULL;
    m_pThread = NULL;

    m_ServerAddr = {};
}

TcpClient::~TcpClient()
{
}

bool TcpClient::Init(EventThread* pThread, char szIP[16], int nPort, int nPingCycle)
{
    bool bResult  = false;
    int  nRetCode = 0;

    nRetCode = nanoev_addr_init(&m_ServerAddr, AF_INET, szIP, nPort);
    LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

    m_pThread = pThread;
    m_pThread->queueTask(new SimpleEventTask(InitConnection, this));

    printf("TcpClient Init\n");

    bResult = true;
Exit0:
    return bResult;
}

bool TcpClient::Uninit()
{
    bool bResult = false;

    bResult = true;
    return bResult;
}

bool TcpClient::DoConnectRequest()
{
    bool bResult = false;
    int  nRetCode = 0;

    printf("Connect request");

    m_pThread->queueTask(new SimpleEventTask(Connect, this));

    bResult = true;
    return bResult;
}

bool TcpClient::OnConnectRespond(void* pBuf)
{
    bool bResult = false;

    printf("Connected successful\n");

    bResult = true;
    return bResult;
}

void TcpClient::InitConnection(nanoev_loop* pLoop, void* pCtx, bool bRun)
{
    TcpClient* self = (TcpClient*)pCtx;

    if (bRun)
    {
        self->InitConnection(pLoop);
    }
}

void TcpClient::InitConnection(nanoev_loop* pLoop)
{
    int            nRetCode = 0;
    nanoev_client* pClient = NULL;

    m_pLoop = pLoop;

    pClient = ClientNew(pLoop);
    LOG_PROCESS_ERROR(pClient);

    m_pConn = nanoev_event_new(nanoev_event_tcp, pLoop, pClient);
    LOG_PROCESS_ERROR(m_pConn);

    printf("Init Connection\n");

Exit0:
    return;
}

void TcpClient::Connect(nanoev_loop* pLoop, void* pCtx, bool bRun)
{
    TcpClient* pSelf = (TcpClient*)pCtx;

    if (bRun)
    {
        pSelf->Connect();
    }
}

void TcpClient::Connect()
{
    int nRetCode = 0;

    nRetCode = nanoev_tcp_connect(m_pConn, &m_ServerAddr, OnConnect);
    LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

Exit0:
    return;
}

void TcpClient::OnConnect(nanoev_event* pClientConn, int nStatus)
{
    int            nRetCode = 0;
    nanoev_client* pClient = NULL;
    TcpClient* pSelf = NULL;

    LOG_PROCESS_ERROR(pClientConn);
    LOG_PROCESS_ERROR(nStatus == 0);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);

    pSelf = (TcpClient*)pClient->pClass;
    pSelf->OnConnect(pClientConn);

Exit0:
    return;
}

void TcpClient::OnConnect(nanoev_event* pClientConn)
{
    int            nRetCode = 0;
    nanoev_client* pClient = NULL;
    const char*    pszMsg = "Hello, World! Request for client info\n";
    unsigned char* puszMsg;

    puszMsg = (unsigned char*)malloc(strlen(pszMsg) + 1);
    LOG_PROCESS_ERROR(puszMsg);

    memcpy(puszMsg, pszMsg, strlen(pszMsg) + 1);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);
    LOG_PROCESS_ERROR(pClient->nState == ecssInit);

    nRetCode = WriteToBuf(pClient, puszMsg);
    LOG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = nanoev_tcp_write(pClientConn, pClient->pszOutBuf, pClient->nOutBufSize, OnWrite);
    LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

Exit0:
    return;
}

void TcpClient::OnWrite(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes)
{
    int              nRetCode = 0;
    nanoev_client*   pClient  = NULL;
    TcpClient*       pSelf    = NULL;

    LOG_PROCESS_ERROR(pClientConn);
    LOG_PROCESS_ERROR(nStatus == 0);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);

    pSelf = (TcpClient*)pClient->pClass;
    pSelf->OnWrite(pClientConn, pBuf, nBytes);

Exit0:
    return;
}

void TcpClient::OnWrite(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes)
{
    int              nRetCode = 0;
    nanoev_client* pClient = NULL;

    LOG_PROCESS_ERROR(nBytes);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);

    pClient->nOutBufSent += nBytes;

    if (pClient->nOutBufSent < pClient->nOutBufSize)
    {
        printf("keep write\n");
        nRetCode = nanoev_tcp_write(pClientConn, pClient->pszOutBuf + pClient->nOutBufSent, pClient->nOutBufSize - pClient->nOutBufSent, OnWrite);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);
    }
    else
    {
        pClient->nInBufSize = 0;
        nRetCode = EnsureInBuf(pClient, CLIENT_OUTPUT_BUFFER_SIZE);
        LOG_PROCESS_ERROR(nRetCode);

        nRetCode = nanoev_tcp_read(pClientConn, pClient->pszInBuf, pClient->nInBufCapacity, OnRead);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);

        pClient->nState = ecssRecv;
    }

Exit0:
    return;
}

void TcpClient::OnRead(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes)
{
    int              nRetCode = 0;
    nanoev_client* pClient = NULL;
    TcpClient* pSelf = NULL;

    LOG_PROCESS_ERROR(pClientConn);
    LOG_PROCESS_ERROR(nStatus == 0);

    printf("On Read\n");

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient);

    pSelf = (TcpClient*)pClient->pClass;
    pSelf->OnRead(pClientConn, pBuf, nBytes);

Exit0:
    return;
}

void TcpClient::OnRead(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes)
{
    int              nRetCode = 0;
    nanoev_client* pClient = NULL;
    char             szMsg[4096];

    LOG_PROCESS_ERROR(pBuf);
    LOG_PROCESS_ERROR(nBytes);

    memcpy(szMsg, pBuf, nBytes);

    printf("Read %s\n", (char*)pBuf);

    OnConnectRespond(szMsg);

    pClient = (nanoev_client*)nanoev_event_userdata(pClientConn);
    LOG_PROCESS_ERROR(pClient->nState == ecssRecv);

    pClient->nInBufSize += nBytes;

    nBytes = GetRemainSize(pClient);
    if (nBytes > 0)
    {
        nRetCode = EnsureInBuf(pClient, pClient->nInBufSize + nBytes);
        LOG_PROCESS_ERROR(nRetCode);

        nRetCode = nanoev_tcp_read(pClientConn, pClient->pszInBuf + pClient->nInBufSize, nBytes, OnRead);
        LOG_PROCESS_ERROR(nRetCode == NANOEV_SUCCESS);
    }
    else
    {
        LOG_PROCESS_ERROR(pClient->pszInBuf);
        LOG_PROCESS_ERROR(pClient->nInBufSize);
        printf("Server return %u nBytes : %s\n", pClient->nInBufSize, pClient->pszInBuf + sizeof(unsigned int));
    }

Exit0:
    return;
}
