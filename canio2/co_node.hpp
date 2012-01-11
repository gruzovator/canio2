/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     CANopen Node I/O
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef CO_NODE_HPP
#define CO_NODE_HPP

#include "node.hpp"
#include "triggers.hpp"
#include <map>
#include <algorithm>
#include <boost/lexical_cast.hpp>

CANIO_NS_START

//-----------------------------------------------------------------------------
struct CANopenError : Error {
    enum {
        ERR_OK =0 ,
        ERR_NODE_STATE_SETUP = 1,
        ERR_STR_TRANSACTION = 2,
        ERR_WRONG_DATA_SIZE = 0x06070010,
        ERR_WRITE_TO_RO  = 0x06010002,
        ERR_READ_FROM_WO = 0x06010001
    };

    std::string decode(int ev) const{
        std::string descr;
        switch(ev){
        case ERR_OK:
            descr = "No errors.";
            break;
        case ERR_NODE_STATE_SETUP:
            descr = "Can't setup desired state. Check device errors.";
            break;
        case ERR_STR_TRANSACTION:
            descr = "String Transaction error.";
            break;
        case 0x05030000:
            descr = "Toggle bit not alternated.";
            break;
        case 0x05040000:
            descr = "SDO protocol timed out.";
            break;
        case 0x05040001:
            descr = "Client/server command specifier not valid or unknown.";
            break;
        case 0x05040002:
            descr = "Invalid block size (block mode only).";
            break;
        case 0x05040003:
            descr = "Invalid sequence number (block mode only).";
            break;
        case 0x05040004:
            descr = "CRC error (block mode only).";
            break;
        case 0x05040005:
            descr = "Out of memory.";
            break;
        case 0x06010000:
            descr = "Unsupported access to an object.";
            break;
        case 0x06010001:
            descr = "Attempt to read a write only object.";
            break;
        case 0x06010002:
            descr = "Attempt to write a read only object.";
            break;
        case 0x06020000:
            descr = "Object does not exist in the object dictionary.";
            break;
        case 0x06040041:
            descr = "Object cannot be mapped to the PDO.";
            break;
        case 0x06040042:
            descr = "The number and length of the objects to be mapped would exceed PDO"
                    "length.";
            break;
        case 0x06040043:
            descr = "General parameter incompatibility reason.";
            break;
        case 0x06040047:
            descr = "General internal incompatibility in the device.";
            break;
        case 0x06060000:
            descr = "Access failed due to an hardware error.";
            break;
        case 0x06070010:
            descr = "Data type does not match, length of service parameter does not match.";
            break;
        case 0x06070012:
            descr = "Data type does not match, length of service parameter too high.";
            break;
        case 0x06070013:
            descr = "Data type does not match, length of service parameter too low.";
            break;
        case 0x06090011:
            descr = "Sub-index does not exist.";
            break;
        case 0x06090030:
            descr = "Value range of parameter exceeded (only for write access).";
            break;
        case 0x06090031:
            descr = "Value of parameter written too high.";
            break;
        case 0x06090032:
            descr = "Value of parameter written too low.";
            break;
        case 0x06090036:
            descr = "Maximum value is less than minimum value.";
            break;
        case 0x08000000:
            descr = "General error.";
            break;
        case 0x08000020:
            descr = "Data cannot be transferred or stored to the application.";
            break;
        case 0x08000021:
            descr = "Data cannot be transferred or stored to the application because of local"
                    "control.";
            break;
        case 0x08000022:
            descr = "Data cannot be transferred or stored to the application because of the"
                    "present device state.";
            break;
        case 0x08000023:
            descr = "Object dictionary dynamic generation fails or no object dictionary is"
                    "present (e.g. object dictionary is generated from file and generation fails"
                    "because of an file error).";
            break;
        default:
            std::ostringstream os;
            os << "Unknown. Code: " << ev ;
            descr = os.str();
        }
        return descr;
    }

    CANopenError(int ec):
        Error(std::string("CANopen error: ")+decode(ec)){}
};
typedef CANopenError CANopenException;

//-----------------------------------------------------------------------------
class CANopenNode: public Node
{
private:
    enum COMMUNICATION_STATE {
        CST_IDLE = -1,
        CST_WAIT_NODE_STATE,
        CST_SDO_DOWNLOAD_EXP,
        CST_SDO_DOWNLOAD,
        CST_SDO_UPLOAD_EXP,
        CST_SDO_UPLOAD
    };
    int comm_state_;
    boost::mutex mtx_;
    boost::condition_variable rx_cond_var_;
    canmsg_t tx_;
    canmsg_t rx_;
    uint32_t state_reply_id_;
    uint32_t sdo_reply_id_;
    void do_transaction(int comm_state) {
        lock_t lock(mtx_);
        comm_state_ = comm_state;
        lock.unlock();
        my_io_service_.send(tx_);
        lock.lock();
        while(comm_state_!=CST_IDLE){
            if(!rx_cond_var_.timed_wait(lock, boost::posix_time::milliseconds(op_timeout_ms_))){
                throw OperationTimeoutExcept();
            }
        }
    }

public:
    CANopenNode(int id, IOService& io) :
        Node(id,io), comm_state_(CST_IDLE)
    {
        state_reply_id_ = 0x700 + id;
        sdo_reply_id_ = 0x580 + id;
    }

    typedef enum NODE_STATES {
        NODE_ST_INITIALIZATION = 0,
        NODE_ST_DISCONNECTED =1,    //* - if node support extended boot-up
        NODE_ST_CONNECTING = 2,     //*
        NODE_ST_PREPARING = 3,      //*
        NODE_ST_STOPPED = 4,
        NODE_ST_OPERATIONAL = 5,
        NODE_ST_PREOPERATIONAL = 127
    } state_t;

    static const char* state_name( state_t state ) {
        static const char* STATES_NAMES[] = {
            "Initialization", "Disconnected", "Connecting",
            "Preparing", "Stopped", "Operational" };
        if( state >= NODE_ST_INITIALIZATION && state <= NODE_ST_OPERATIONAL )
            return  STATES_NAMES[state];
        if( state == NODE_ST_PREOPERATIONAL )
            return  "Pre-operational";
        return "Unknown";
    }

    typedef enum NODE_COMMANDS {
        NODE_CMD_NOP = 0,
        NODE_CMD_START = 1,
        NODE_CMD_STOP = 2,
        NODE_CMD_ENTER_PRE_OP = 0x80,
        NODE_CMD_RESET_NODE = 0x81,
        NODE_CMD_RESET_COMMUNICATION = 0x82
    } cmd_t;

    /// sends NMT commands (see enum NODE_COMMANDS)
    void send_cmd( cmd_t cmd ) {
        static canmsg_t cmd_msg = {0x0, 0x2, 0x0, {0}};
        cmd_msg.data[0] = (int)cmd;
        cmd_msg.data[1] = my_id_;
        this->my_io_service_.send(cmd_msg);
    }

    /** Tries setup new node state.
        The new state is not guaranteed,e.g. cause of node errors.
        Check node state after this operation. 
        Attention! Node state switch time is different for differenet
        HW. It's not zero. So, do hw specific delay before 
        new node state checking */ 
    void try_set_state( state_t st ) {
        cmd_t cmd;
        switch(st){
        case NODE_ST_INITIALIZATION:
            cmd = NODE_CMD_RESET_NODE;
            break;
        case NODE_ST_STOPPED:
            cmd = NODE_CMD_STOP;
            break;
        case NODE_ST_PREOPERATIONAL:
            cmd = NODE_CMD_ENTER_PRE_OP;
            break;
        case NODE_ST_OPERATIONAL:
            cmd = NODE_CMD_START;
            break;
        default:
            cmd = NODE_CMD_NOP;
        }
        send_cmd(cmd);
        // different devices have different state switch time
        // so, user should do device specific delay to wait new state
    }

    state_t get_state(){
        tx_.id = 0x700 + my_id_;
        tx_.len = 0x1;
        tx_.rtr = 0x1;
        do_transaction(CST_WAIT_NODE_STATE);
        return (state_t)(rx_.data[0] & 0x7F); // & 0x7F - skip toggle bit
    }

    /// Resets node (Sends 'reset' and wait 'timeout'). Returns wait result.
    bool reset( uint timeout ) {
        struct BootTrg : Trigger
        {
            BootTrg( int node_id):event_id_(0x700 + node_id){}
            uint32_t event_id_;
            virtual bool activated_(const canmsg_t& in) {
                return ( in.id == event_id_ && in.len == 1 && in.data[0] == 0 );
            }
        };
        BootTrg boot_trg(my_id_);
        TriggersWaiter waiter(this->my_io_service_);
        waiter.add(boot_trg);
        send_cmd(NODE_CMD_RESET_NODE);
        return waiter.wait(timeout);
    }

    /** @name CANopen SDO Download/Upload 
        Next functions do read/write of node's object dictionary.
        @param idx object index
        @param sub_idx object sub index 
        @throw CANopenException
        @throw IOError */
    //@{
    template< typename T >
    void write(uint16_t idx, uint8_t sub_idx, const T& v) {
        static const uint8_t data0[] = {0x2F, 0x2B, 0x27, 0x23};
        tx_.id = 0x600 + my_id_;
        tx_.rtr = 0;
        tx_.len = 8;
        if(sizeof(T)<=4){
            tx_.data[0] = data0[sizeof(T)-1];
        }
        else
            assert(0);
        memcpy( &tx_.data[1], &idx, 2);
        tx_.data[3] = sub_idx;
        memcpy( &tx_.data[4], &v, sizeof(T) );
        do_transaction(CST_SDO_DOWNLOAD_EXP);
        if(rx_.data[0] != 0x60 ){ //0x60 means success
            int err = 0;
            memcpy(&err, &rx_.data[4], 4);
            throw CANopenException(err);
        }
    }
    void write(uint16_t idx, uint8_t sub_idx, const std::string& v ){
        tx_.id = 0x600 + my_id_;
        tx_.rtr = 0;
        tx_.len = 8;
        memcpy( &tx_.data[1], &idx, 2);
        tx_.data[3] = sub_idx;
        const size_t str_size = v.length();
        size_t n = str_size;
        if(n<=4){
            tx_.data[0] = 0x23 + ((4-n)<<2);
            memcpy(&tx_.data[4], v.c_str(), n);
        }
        else {
            tx_.data[0] = 0x21;
            memcpy( &tx_.data[4], &n, 4 );
        }
        do_transaction(CST_SDO_DOWNLOAD_EXP);
        if(rx_.data[0] == 0x80 ){ //0x80 means error
            int err = 0;
            memcpy(&err, &rx_.data[4], 4);
            throw CANopenException(err);
        }
        if(n<=4) return;
        const char* p = v.c_str();
        if(n>7){
            tx_.data[0] = 0;
            n = 7;
        }
        else
            tx_.data[0] = ((7-n)<<1) + 1;
        bool toggle_bit;
        while(1){
            memcpy(&tx_.data[1], p, n);
            p+=n;
            do_transaction(CST_SDO_DOWNLOAD);
            if( rx_.data[0] != (tx_.data[0] & 0x1F)+0x20 )
                throw CANopenException(CANopenError::ERR_STR_TRANSACTION);
            n = str_size - (p-v.c_str());
            if(n==0) break;
            toggle_bit = ((tx_.data[0] & 0x10)>0);
            if(n>7){
                tx_.data[0] = toggle_bit ? 0 : 0x10;
                n = 7;
            }
            else{
                tx_.data[0] = ((7-n)<<1) + 1;
                if(!toggle_bit) tx_.data[0] += 0x10;
            }
        }
    }
    
    /// Short version of write, for object with zero sub index 
    template< typename T >
    void write(uint16_t idx, const T& v) {write(idx, 0, v);}

    template< typename T >
    T read(uint16_t idx, uint8_t sub_idx) {
        tx_.id = 0x600 + my_id_;
        tx_.rtr = 0;
        tx_.len = 8;
        tx_.data[0] = 0x40;
        memcpy( &tx_.data[1], &idx, 2);
        tx_.data[3] = sub_idx;
        memset(&tx_.data[4], 0, 4);
        do_transaction(CST_SDO_UPLOAD_EXP);
        if(rx_.data[0] == 0x80 ){ //0x80 means error
            int err = 0;
            memcpy(&err, &rx_.data[4], 4);
            throw CANopenException(err);
        }

        static const uint8_t data0[] = {0x4F, 0x4B, 0x47, 0x43};
        T res;
        if( sizeof(T)<=4 ){
            if(data0[sizeof(T)-1] != rx_.data[0] ){
                throw CANopenException(CANopenError::ERR_WRONG_DATA_SIZE);
            }
            else {
                memcpy(&res, &rx_.data[4], sizeof(T));
            }
        }
        else {
            assert(0);
        }
        return res;
    }
    
    std::string read(uint16_t idx, uint8_t sub_idx){
        std::string res;
        tx_.id = 0x600 + my_id_;
        tx_.rtr = 0;
        tx_.len = 8;
        tx_.data[0] = 0x40;
        memcpy( &tx_.data[1], &idx, 2);
        tx_.data[3] = sub_idx;
        memset(&tx_.data[4], 0, 4);
        do_transaction(CST_SDO_UPLOAD_EXP);
        if(rx_.data[0] == 0x80 ){ //0x80 means error
            int err = 0;
            memcpy(&err, &rx_.data[4], 4);
            throw CANopenException(err);
        }
        // first reply contains short string or length
        if( (rx_.data[0] & 0x3) == 0x3 ){ //short string (<=4 bytes)
            res.append((char*)&rx_.data[4],4);
            return res;
        }
        size_t str_size;
        memcpy(&str_size, rx_.data+4,4);
        res.reserve(str_size);
        tx_.data[0] = 0x60;
        bool toggle_bit;
        do {
            do_transaction(CST_SDO_UPLOAD);
            if(rx_.data[0] == 0x80 ){ //0x80 means error
                int err = 0;
                memcpy(&err, &rx_.data[4], 4);
                throw CANopenException(err);
            }
            toggle_bit = ((tx_.data[0]>>4)&0x1);
            if( toggle_bit != ((rx_.data[0]>>4)&0x1) )
                throw CANopenException(CANopenError::ERR_STR_TRANSACTION);
            size_t n = 0x7 -((rx_.data[0]>>1)&0x7);
            res.append((char*)&rx_.data[1],n);
            if( toggle_bit )
                tx_.data[0] = 0x60;
            else
                tx_.data[0] = 0x70;
        } while( (rx_.data[0] & 0x1) == 0 );
        return res;
    }
    
    /// Short version of read, for object with zero sub index 
    template< typename T >
    T read(uint16_t idx) { return read<T>(idx, 0);}
    std::string read(uint16_t idx) { return read(idx, 0); }
    //}@

private:
    virtual void on_msg_( const canmsg_t & in ){
        lock_t lock(mtx_);
        if(comm_state_ == CST_IDLE) return;
        if(comm_state_ == CST_WAIT_NODE_STATE && in.id != state_reply_id_ )
            return;
        if(comm_state_ >= CST_SDO_DOWNLOAD_EXP && comm_state_ <= CST_SDO_UPLOAD
                && in.id != sdo_reply_id_ )
            return;
        if((comm_state_ == CST_SDO_DOWNLOAD_EXP ||comm_state_ == CST_SDO_UPLOAD_EXP)
                && memcmp(&in.data[1],&tx_.data[1],3)!=0 /*cmp idx, sub_idx*/)
            return;
        rx_ = in;
        comm_state_ = CST_IDLE;
        rx_cond_var_.notify_one();
    }
};

CANIO_NS_END

#endif // CO_NODE_HPP
