#pragma once
struct PROTOCOL_HEADER
{
    unsigned int uProtocolID;
};

enum GS_CLIENT_PROTOCOL
{
    s2c_connection_begin = 0,

    s2c_get_client_info_respond,

    s2c_connection_end,
};

enum CLIENT_GS_PROTOCOL
{
    c2s_connection_begin = 0,

    c2s_get_client_info_request,

    c2s_connection_end,
};

struct C2S_GET_CLIENT_INFO_REQUEST : PROTOCOL_HEADER
{

};

struct S2C_GET_CLIENT_INFO_RESPOND : PROTOCOL_HEADER
{

};
