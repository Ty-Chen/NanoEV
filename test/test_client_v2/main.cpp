﻿#include "TcpClient.h"

int main()
{
    int             nResult         = 0;
    bool            bRetCode        = false;
    bool            bConnected      = false;
    char            LocalIP[]       = "127.0.0.1";
    EventThread     m_EventThread;
    TcpClient       m_Client;

    bRetCode = nanoev_init();
    LOG_PROCESS_ERROR(bRetCode == NANOEV_SUCCESS);

    bRetCode = m_EventThread.start();
    LOG_PROCESS_ERROR(bRetCode);

    bRetCode = m_Client.Init(&m_EventThread, LocalIP, 7001, 5);
    LOG_PROCESS_ERROR(bRetCode);
                     
    while (true)
    {
        if (!bConnected)
        {
            m_Client.DoConnectRequest();
            bConnected = true;
        }    
        Sleep(1000);
    }

    nResult = 1;
Exit0:
    m_EventThread.stop();

    return nResult;
}
