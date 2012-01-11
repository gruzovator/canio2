#include "can_plugin.hpp"
#include <windows.h>
#include <boost/scoped_ptr.hpp>
#include "CANMessage.h" 

struct can_iface {
    int socket;
};

F_0(const char*, canm_version)
{
    return "0.1";
}

F_0(const char*, canm_name)
{
    return "can_gate plugin";
}

F_0(const char*, canm_id_name)
{
    return "IP address";
}

F_0(const char*, canm_param_name)
{
    return "Port";
}

F_2(void, canm_params, char* buf, uint buf_sz )
{
    strncpy(buf, "7777", buf_sz);
    buf[buf_sz-1] = '\0';
}

F_2(void, canm_ids, char* buf, uint /*buf_sz*/)
{
    buf[0] = '\0';
}

F_3(can_iface_t, canm_make_iface, const char* ip, const char* port, uint* p_err)
{
    WSADATA info;
    *p_err = WSAStartup(MAKEWORD(2,0), &info);
    if(*p_err) {
        return NULL;
    }

    boost::scoped_ptr<can_iface> iface(new can_iface);
    iface->socket = socket(AF_INET,SOCK_STREAM,0);
    if( iface->socket == INVALID_SOCKET ){
        *p_err = WSAGetLastError();
        WSACleanup();
        return NULL;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);
    if( addr.sin_addr.s_addr == INADDR_NONE ){
        closesocket(iface->socket);
        *p_err = WSAGetLastError();
        WSACleanup();
        return NULL;
    }
    memset(&(addr.sin_zero), 0, 8);
    if(connect(iface->socket, (sockaddr *)&addr, sizeof(sockaddr))) {
        *p_err = WSAGetLastError();
        closesocket(iface->socket);
        WSACleanup();
        return NULL;
    }
    can_iface_t res = new can_iface(*iface);
    return res;
}

F_3(void, canm_err, uint error, char* buf, uint buf_sz){
    if(error) {
        if(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                0,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                buf, buf_sz, 0)==0){
            sprintf_s(buf,buf_sz-1,"Unknown error (0x%X)",error );
        }
        buf[buf_sz-1] = 0;
    }
    else
        buf[0] = 0;
}

//-----------------------------------------------------------------------------
// CAN Iface
//-----------------------------------------------------------------------------
F_1(void, can_del, can_iface_t i)
{
    closesocket(i->socket);
    WSACleanup();
    delete i;
}

F_4(bool, can_send, can_iface_t i, const canmsg_t& m,
        uint, uint* p_err)
{
    TCanMessage msg(m.id,0,m.rtr,m.len,m.data);
    if( ::send(i->socket, reinterpret_cast<char*>(&msg), sizeof TCanMessage, 0) == SOCKET_ERROR ){
        *p_err = WSAGetLastError();
        return false;
    }
    return true;
}

F_4(bool, can_recv, can_iface_t i, canmsg_t* m,
        uint timeout, uint* p_err)
{
    TCanMessage msg;
    fd_set rsd;
    FD_ZERO( &rsd );
    FD_SET( i->socket, &rsd );
    struct timeval tv;
    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout%1000)*1000;
    int res = ::select(0, &rsd, (fd_set*) 0, (fd_set*) 0, &tv);
    if( res == 0 ){ // timeout
        return false;
    }
    else
    if( res == SOCKET_ERROR ) {
        *p_err = WSAGetLastError();
    }
    else{
        res = ::recv( i->socket, reinterpret_cast<char*>(&msg), sizeof TCanMessage, 0 /*MSG_WAITALL*/ );
        m->id = msg.Arbitration();
        m->len = msg.m_nDataSize;
        m->rtr = msg.m_nRemoteRequest;
        memcpy( m->data, msg.m_aData, 8 );
        if(res == SOCKET_ERROR){
            *p_err = WSAGetLastError();
        }
    }
    return *p_err == 0;
}

F_3(void, can_set_acc_filter, can_iface_t , uint , uint )
{
    // can_gate doesn't support filters
}
F_3(void, can_add_id_filter, can_iface_t , uint , uint )
{
}
F_3(void, can_rem_id_filter, can_iface_t , uint , uint )
{
}

F_3(void, can_err, uint e, char* buf, uint buf_sz)
{
    canm_err(e,buf,buf_sz);
}
