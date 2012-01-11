#include "vcinpl.h"
#include "can_plugin.hpp"
#include <string.h>
#include <string>
#include <memory>

#define RX_FIFO_SIZE 512
#define TX_FIFO_SIZE 128

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------
void std_to_ixxat( const canmsg_t* m, CANMSG* ixxat_msg ) {
    ixxat_msg->dwTime = 0;
    ixxat_msg->dwMsgId = m->id;
    ixxat_msg->uMsgInfo.Bytes.bType  = CAN_MSGTYPE_DATA;
    ixxat_msg->uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(m->len,0,0,m->rtr,0);
    for ( int i = 0; i < m->len; ++i )
        ixxat_msg->abData[i] = m->data[i];
}

void ixxat_to_std( const CANMSG* ixxat_msg, canmsg_t* m ) {
    if( ixxat_msg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA ) {
        m->id = ixxat_msg->dwMsgId;
        m->rtr = ixxat_msg->uMsgInfo.Bits.rtr;
        m->len = ixxat_msg->uMsgInfo.Bits.dlc;
        for ( int i = 0; i < m->len; ++i )
            m->data[i] = ixxat_msg->abData[i];
    }
    else { // driver internals message, e.g, start, reset
        m->id = UINT_MAX;
        m->len = 0;
    }
}
//-----------------------------------------------------------------------------
// Plugin functions def
//-----------------------------------------------------------------------------
struct can_iface {
    HANDLE device;
    HANDLE ctrl_unit;
    HANDLE msg_channel;
};

F_0(const char*, canm_version)
{
    return "0.1";
}

F_0(const char*, canm_name)
{
    return "IXXAT USB-to-CAN";
}

F_0(const char*, canm_id_name)
{
    return "Device ID";
}

F_0(const char*, canm_param_name)
{
    return "Bitrate";
}

F_2(void, canm_params, char* buf, uint buf_sz )
{
    strncpy(buf, "1000K,500K,125K", buf_sz);
    buf[buf_sz-1] = '\0';
}

F_2(void, canm_ids, char* buf, uint buf_sz)
{
    std::string ids;
    HRESULT res;
    HANDLE        hEnum;   // enumerator handle
    VCIDEVICEINFO sInfo;   // device info
    // open the device list
    if( (res = vciEnumDeviceOpen(&hEnum)) != VCI_OK )
        buf[0] = '\0';
    while( (res = vciEnumDeviceNext(hEnum, &sInfo)) == VCI_OK ) {
        ids += std::string(sInfo.UniqueHardwareId.AsChar) + ',';
    }
    vciEnumDeviceClose(hEnum);

    if( ids.length() > 0 ) // remove last comma
        ids.resize( ids.length()-1 );
    strncpy( buf, ids.c_str(), buf_sz );
    buf[buf_sz-1]='\0';
}

F_3(can_iface_t, canm_make_iface, const char* id, const char* bitrate, uint* p_err)
{
    HANDLE        hEnum;   // enumerator handle
    VCIDEVICEINFO sInfo;   // device info
    std::string id_str(id);
    std::auto_ptr<can_iface> iface(new can_iface);

    // open device
    bool any_device = false;
    if( id_str == "*" )
        any_device = true;
    *p_err = vciEnumDeviceOpen(&hEnum);
    if(*p_err!=VCI_OK) return NULL;
    while( (*p_err = vciEnumDeviceNext(hEnum, &sInfo)) == VCI_OK ) {
        if( id_str == std::string(sInfo.UniqueHardwareId.AsChar) ||
            any_device == true ) {
            vciEnumDeviceClose(hEnum);
            *p_err = vciDeviceOpen( sInfo.VciObjectId, &iface->device );
            break;
        }
    }
    if( *p_err != VCI_OK ){
        vciDeviceClose(iface->device);
        return NULL;
    }

    // init device
    // .. bitrate regs selection
    UINT8 br0, br1;
    if( strcmp(bitrate, "125K") == 0 ) {
        br0 = CAN_BT0_125KB;
        br1 = CAN_BT1_125KB;
    }
    else
    if( strcmp(bitrate, "500K") == 0 ) {
        br0 = CAN_BT0_500KB;
        br1 = CAN_BT1_500KB;
    }
    else
    if( strcmp(bitrate, "1000K") == 0 ) {
        br0 = CAN_BT0_1000KB;
        br1 = CAN_BT1_1000KB;
    }
    else{
        vciDeviceClose(iface->device);
        *p_err = VCI_E_INVALIDARG;
        return NULL;
    }
    // .. init
    do {
        #define TRY(f) if( (*p_err = f) != VCI_OK ) break
        // create and initialize a message channel
        TRY(canChannelOpen( iface->device, 0, FALSE , &iface->msg_channel )); // 'FALSE' means 'not exclusive'
        // initialize the message channel
        UINT16 rx_fifo_sz  = RX_FIFO_SIZE;
        UINT16 rx_threshold = 1;
        UINT16 tx_fifo_sz  = TX_FIFO_SIZE;
        UINT16 tx_threshold = 1;
        TRY(canChannelInitialize( iface->msg_channel, rx_fifo_sz, rx_threshold,
                tx_fifo_sz, tx_threshold ));
        TRY(canChannelActivate( iface->msg_channel, TRUE ));
        // open the CAN controller
        TRY(canControlOpen( iface->device , 0, &iface->ctrl_unit ));

        if( (*p_err = canControlInitialize( iface->ctrl_unit, CAN_OPMODE_STANDARD,
                br0, br1 )) != VCI_OK ) {
            if( *p_err == VCI_E_ACCESSDENIED ) { // control unit already opened
                // get current bitrate. is it equal to desired one?
                CANLINESTATUS st;
                canControlGetStatus( iface->ctrl_unit, &st );
                if( st.bBtReg0 != br0 || st.bBtReg1 != br1 )
                    *p_err = VCI_E_BUSY;
                else {
                    *p_err = VCI_OK;
                }
                break; // control unit has been initialized by someone else
            }
            else // an error
                break;
        }
        // set the acceptance filter
        TRY(canControlSetAccFilter( iface->ctrl_unit, CAN_FILTER_STD,
                                     CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL ));
        // start the CAN controller
        TRY(canControlStart( iface->ctrl_unit, TRUE )); // TRUE -> start can controller
    }while(0);
    if( *p_err != VCI_OK ) {
        vciDeviceClose(iface->device);
        canChannelClose(iface->msg_channel);
        canControlClose(iface->ctrl_unit);
        return NULL;
    }
    // Now device is initialized, 
    can_iface_t res = new can_iface;
    *res = *iface; // iface is guarded by smart ptr and will be destroyed
    return res;
}

F_3(void, canm_err, uint error, char* buf, uint buf_sz){
    if(error) {
        if(error == VCI_E_NO_MORE_ITEMS) {
            strncpy(buf, "No such interface.", buf_sz);
        } else {
            vciFormatError( error, buf, buf_sz );
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
    vciDeviceClose(i->device);
    canChannelClose(i->msg_channel);
    canControlClose(i->ctrl_unit);
    delete i;
}
F_4(bool, can_send, can_iface_t i, const canmsg_t& m,
        uint timeout, uint* p_error)
{
    CANMSG ixxat_msg;
    std_to_ixxat( &m, &ixxat_msg );
    //if(timeout == 0) timeout = INFINITE;
    canChannelWaitTxEvent(i->msg_channel, 0); // should be called before, otherwise canChannelSendMessage won't wait
    HRESULT res = canChannelSendMessage( i->msg_channel, timeout, &ixxat_msg );
    /*
    // Next code shows how canChannelSendMessage works
    HRESULT res = canChannelPostMessage(i->msg_channel, &ixxat_msg );
    if( res == VCI_E_TXQUEUE_FULL ){
        canChannelWaitTxEvent(i->msg_channel, timeout);// this call always don't wait
        //if we don't call canChannelWaitTxEvent(i->msg_channel, 0) before
        res = canChannelPostMessage(i->msg_channel, &ixxat_msg );
    }
    */
    if( res == VCI_E_TIMEOUT ) {
        *p_error  = 0;
    }
    else {  
        *p_error = res;
    }
    return (res == VCI_OK);
}

F_4(bool, can_recv, can_iface_t i, canmsg_t* m,
        uint timeout, uint* p_error)
{
    HRESULT res;
    CANMSG ixxat_msg;
    if(timeout == 0) timeout = INFINITE;
    while( (res = canChannelReadMessage( i->msg_channel, timeout, &ixxat_msg )) == VCI_OK ) {
        ixxat_to_std( &ixxat_msg, m );
        if( m->id == UINT_MAX ) { // internal messages are skipped 
            continue;
        }
        else
            break;
    }
    if(res == VCI_E_TIMEOUT)
        *p_error = 0;
    else
        *p_error = res;
    return (res == VCI_OK);
}

F_3(void, can_set_acc_filter, can_iface_t i , uint code, uint mask)
{
    canControlStart(i->ctrl_unit,false);
    canControlSetAccFilter(i->ctrl_unit,false,code, mask);
    canControlStart(i->ctrl_unit,true);
}
F_3(void, can_add_id_filter, can_iface_t i, uint code, uint mask)
{
    canControlStart(i->ctrl_unit,false);
    canControlAddFilterIds(i->ctrl_unit,false, code, mask);
    canControlStart(i->ctrl_unit,true);
}
F_3(void, can_rem_id_filter, can_iface_t i, uint code, uint mask)
{
    canControlStart(i->ctrl_unit,false);
    canControlRemFilterIds(i->ctrl_unit,false, code, mask);
    canControlStart(i->ctrl_unit,true);
}

F_3(void, can_err, uint e, char* buf, uint buf_sz)
{
    if(e) {
        vciFormatError( e, buf, buf_sz );
        buf[buf_sz-1] = 0;
    }
    else
        buf[0] = 0;
}
