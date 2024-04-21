#pragma once
#include "stdafx.h"
#include "nanoev.h"
#include "def.h"

enum ClientSocketState
{
    esssInvalid = -1,

    ecssInit = 0,
    ecssSend,
    ecssRecv,

    ecssTotal
};

typedef struct
{
    int            nState;
    unsigned char* pszOutBuf;
    unsigned int   nOutBufCapacity;
    unsigned int   nOutBufSize;
    unsigned int   nOutBufSent;
    unsigned char* pszInBuf;
    unsigned int   nInBufCapacity;
    unsigned int   nInBufSize;
    nanoev_event*  pClientConn;
    nanoev_event*  pTimer;
    void*          pClass;
} nanoev_client;

static nanoev_client* ClientNew(nanoev_loop* pLoop)
{
    nanoev_client* pClient = (nanoev_client*)malloc(sizeof(nanoev_client));

    if (pClient)
    {
        pClient->nState          = ecssInit;
        pClient->pszOutBuf       = NULL;
        pClient->nOutBufCapacity = 0;
        pClient->nOutBufSize     = 0;
        pClient->nOutBufSent     = 0;
        pClient->pszInBuf        = NULL;
        pClient->nInBufCapacity  = 0;
        pClient->nInBufSize      = 0;
        pClient->pClientConn     = NULL;
        pClient->pClass          = NULL;
        pClient->pTimer          = nanoev_event_new(nanoev_event_timer, pLoop, pClient);
        LOG_PROCESS_ERROR(pClient->pTimer);
    }

Exit0:
    return pClient;
}

static void ClientFree(nanoev_client* pClient)
{
    free(pClient->pszOutBuf);
    free(pClient->pszInBuf);
    nanoev_event_free(pClient->pTimer);
    free(pClient);
}

static int EnsureInBuf(nanoev_client* pClient, unsigned int nCapacity)
{
    int   nResult = 0;
    void* pNewBuf;
    if (pClient->nInBufCapacity < nCapacity)
    {
        pNewBuf = realloc(pClient->pszInBuf, nCapacity);
        LOG_PROCESS_ERROR(pNewBuf);

        pClient->pszInBuf = (unsigned char*)pNewBuf;
        pClient->nInBufCapacity = nCapacity;
    }

    nResult = 1;
Exit0:
    return nResult;
}

static int GetRemainSize(nanoev_client* pClient)
{
    unsigned int nTotal = 0;

    if (pClient->nInBufSize < sizeof(unsigned int))
    {
        nTotal = sizeof(unsigned int);
    }
    else
    {
        nTotal = sizeof(unsigned int) + *((unsigned int*)pClient->pszInBuf);
    }

    return nTotal - pClient->nInBufSize;
}

static int WriteToBuf(nanoev_client* pClient, const unsigned char* pszMsg)
{
    unsigned int nLen = 0;
    unsigned int nRequiredCapacity = 0;

    LOG_PROCESS_ERROR(pClient);
    LOG_PROCESS_ERROR(pszMsg);

    nLen = (unsigned int)strlen((const char*)pszMsg) + 1;
    nRequiredCapacity = sizeof(unsigned int) + nLen;

    if (pClient->nOutBufCapacity < nRequiredCapacity)
    {
        void* pszNewBuf = realloc(pClient->pszOutBuf, nRequiredCapacity);
        LOG_PROCESS_ERROR(pszNewBuf);

        pClient->pszOutBuf = (unsigned char*)pszNewBuf;
        pClient->nOutBufCapacity = nRequiredCapacity;
    }

    memcpy(pClient->pszOutBuf + pClient->nOutBufSize, pszMsg, nLen);
    pClient->nOutBufSize += nLen;

Exit0:
    return 0;
}
