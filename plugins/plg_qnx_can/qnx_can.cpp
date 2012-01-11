/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief		QNX CAN plugin for CANIO2 lib
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include "can_plugin.hpp"
#include "CanMessage.h"

struct can_iface {
	can_iface() : fd(0){}
	can_iface( int f ) : fd(f){}
	int fd;
};

F_0(const char*, canm_version)
{
    return "0.1";
}

F_0(const char*, canm_name)
{
    return "QNX CAN Resource Manager";
}

F_0(const char*, canm_id_name)
{
    return "CAN iface path";
}

F_0(const char*, canm_param_name)
{
    return "Bitrate (not implemented)";
}

F_2(void, canm_params, char* buf, uint /*buf_sz*/ )
{
    buf[0] = '\0';
}

F_2(void, canm_ids, char* buf, uint /*buf_sz*/)
{
    buf[0] = '\0';
}

F_3(can_iface_t, canm_make_iface, const char* path, const char* /*param*/, uint* p_err)
{
	can_iface_t iface = 0;
	int fd = open( path, O_RDWR | O_APPEND );
	if(fd < 0) {
		*p_err = errno;
	}
	else {
		iface = new can_iface(fd);
	}
	return iface;
}

F_3(void, canm_err, uint error, char* buf, uint buf_sz){
    if(error) {
    	strncpy( buf, strerror(error), buf_sz );
        buf[buf_sz-1] = 0;
    }
    else
        buf[0] = 0;
}

F_1(void, can_del, can_iface_t i)
{
	close(i->fd);
    delete i;
}

F_4(bool, can_send, can_iface_t i, const canmsg_t& m,
        uint timeout, uint* p_error)
{
	TCanMessage msg(m.id,0,m.rtr,m.len,m.data);
	int res = write( i->fd, &msg, sizeof(msg));
	if( res == -1 )
		*p_error = errno;
	return res != -1;
}

F_4(bool, can_recv, can_iface_t i, canmsg_t* m,
        uint timeout, uint* p_error)
{
	fd_set rfd;
	FD_ZERO( &rfd );
	FD_SET( i->fd, &rfd );
	struct timeval tv;
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;
	TCanMessage msg;
	int res = select((i->fd)+1,&rfd, 0, 0, &tv);
	if( res > 0 ){
		res = read( i->fd, &msg, sizeof(msg) );
		if( res == sizeof(msg) ) {
			m->id = msg.Arbitration();
			m->len = msg.m_nDataSize;
			m->rtr = msg.m_nRemoteRequest;
			memcpy(	m->data, msg.m_aData, 8 );
		}
		else
		if( res < 0 ) { // Error
			*p_error = errno;
		}
		//TODO res > 0 but < sizeof msg Assert? Exception?
	}
	else
	if(res == 0) { // timeout
		FD_SET( i->fd, &rfd );
	}
	else { // select call error
		*p_error = errno;
	}
	return res>0;
}

F_3(void, can_set_acc_filter, can_iface_t , uint , uint )
{
	// this plugin doesn't support filters
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
