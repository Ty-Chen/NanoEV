#pragma once
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "nanoev.hpp"
#include "NanoevClient.h"
#include "event_thread.h"
#include "CS_Protocol.h"

#define TIMEOUT_SECONDS 300

#define CLIENT_INPUT_BUFFER_SIZE 4096
#define CLIENT_OUTPUT_BUFFER_SIZE 4096

class TcpClient
{
public:
    TcpClient();
    ~TcpClient();

    bool Init(EventThread* pThread, char szIP[16], int nPort, int nPingCycle);
    bool Uninit();

    bool DoConnectRequest();
    bool OnConnectRespond(void* pBuf);

private:
    static void InitConnection(nanoev_loop* pLoop, void* pCtx, bool bRun);
    void InitConnection(nanoev_loop* pLoop);

    static void Connect(nanoev_loop* pLoop, void* pCtx, bool bRun);
    void Connect();

    static void OnConnect(nanoev_event* pClientConn, int nStatus);
    void OnConnect(nanoev_event* pClientConn);

    static void OnRead(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes);
    void OnRead(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes);

    static void OnWrite(nanoev_event* pClientConn, int nStatus, void* pBuf, unsigned int nBytes);
    void OnWrite(nanoev_event* pClientConn, void* pBuf, unsigned int nBytes);

    static void OnTimer(nanoev_event* pTimer);

private:
    EventThread*    m_pThread;
    nanoev_addr     m_ServerAddr;
    nanoev_loop*    m_pLoop;
    nanoev_event*   m_pConn;
};

#endif
