#pragma once
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "nanoev.hpp"
#include "NanoevClient.h"
#include "event_thread.h"
#include "CS_Protocol.h"

#define TIMEOUT_SECONDS 300

#define CLIENT_INPUT_BUFFER_SIZE 4096
#define CLIENT_OUTPUT_BUFFER_SIZE 4096

#define REGISTER_EXTERNAL_FUNC(ProtocolID, FuncName, ProtocolSize)  \
               { m_ProcessProtocolFuns[ProtocolID] = FuncName;      \
                 m_nProtocolSize[ProtocolID] = ProtocolSize; }

class ClientInfo
{
    int           nConnID;
    int           nStatus;
    nanoev_client ClientInfo;
};

class TcpServer
{
public:

    TcpServer();
    ~TcpServer();

    bool Init(EventThread* pThread, char szIP[16], int nPort, int nPingCycle);
    bool Uninit();

    void GetClientInfo(char* pData, size_t nSize, int nConnIndex, int nFrame);

private:
    static void InitConnection(nanoev_loop* pLoop, void* pContext, bool bRun);
    void InitConnection(nanoev_loop* pLoop);

    static void OnAccept(nanoev_event* pServerConn, int nStatus, nanoev_event* pNewClientConn);
    void OnAccept(nanoev_event* pServerConn, nanoev_event* pNewClientConn);

    static void* AllocUserData(void* pContext, void* pUserData);

    static void OnRead(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes);
    void OnRead(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes);

    static void OnWrite(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes);
    void OnWrite(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes);

    static void OnTimer(nanoev_event* pTimer);

private:
    EventThread*  m_pThread;
    nanoev_addr   m_LocalAddr;
    nanoev_loop*  m_pLoop;
    nanoev_event* m_pConn;

    std::vector<nanoev_client> m_pClientPool;

    typedef struct
    {
        nanoev_loop* loop;
    } GlobalData;

    typedef void (TcpServer::* PROCESS_PROTOCOL_FUNC)(char*, size_t, int, int);
    PROCESS_PROTOCOL_FUNC   m_ProcessProtocolFuns[c2s_connection_end];
    int                     m_nProtocolSize[c2s_connection_end];
};


#endif
