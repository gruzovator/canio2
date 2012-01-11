/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     CAN IO Service
 ****************************************************************************
 @details   IOService and IOClient make Observer pattern for CAN iface 
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef IO_SERVICE_HPP
#define IO_SERVICE_HPP
#include "can_module.hpp"

CANIO_NS_START

class IOClient;

/** IOService is a gate to low level CAN I/O.
    It has receiving thread, that receives CAN packets
    and then notifies subscribers - IOClients.*/
class IOService : boost::noncopyable
{
    // These timouts are just low level TX/RX timeouts
    // High level operations timouts are defined in high level protocols
    static const int RX_TIMEOUT = 500;
    static const int TX_TIMEOUT = 500;
public:
    IOService(){}
    IOService( can_iface_sp_t i ):can_iface_(i) {
        if(can_iface_)
            rx_thr_ = boost::thread(&IOService::rx_thread,this);
        //std::cout << "iosrv ctor\n";
    }

    virtual ~IOService(){
        if(can_iface_) {
            rx_thr_.interrupt();
            rx_thr_.join();
        }
        //std::cout << "iosrv dtor\n";
    }

    void reset( can_iface_sp_t i ){
        rx_thr_.interrupt();
        rx_thr_.join();
        lock_t lock(send_mtx_);
        error_.clear();
        can_iface_ = i;
        if(can_iface_)
            rx_thr_ = boost::thread(&IOService::rx_thread,this);
    }

    /// @throw IOException
    void send( const canmsg_t& out ){
        lock_t lock(send_mtx_);
        if(error_){
            throw IOException(error_.what());
        }
        if(!can_iface_){
            throw IOException("CAN Interface is not set.");
        }
        if( can_iface_->send(out, TX_TIMEOUT) == false ) {
            throw IOException("CAN message TX timeout.");
        }
    }

    void add_client( IOClient& c );

    void rem_client( IOClient& c ){
        lock_t lock(clients_mtx_);
        clients_.erase(
                    std::remove(clients_.begin(), clients_.end(),&c),
                    clients_.end());
    }

private:
    can_iface_sp_t can_iface_;

    boost::mutex send_mtx_;
    boost::mutex clients_mtx_;
    IOError error_;

    std::vector<IOClient*> clients_;

    bool is_running_;
    boost::thread rx_thr_;
    void rx_thread();
};

/** Base (abstract) class for all IOService users 
    That class not only handles IOService interaction, 
    but also keeps an io error state. */
class IOClient : boost::noncopyable
{
public:
    IOClient( IOService& io ): my_io_service_(io){
        my_io_service_.add_client(*this);
    }

    virtual ~IOClient(){
        try {
            my_io_service_.rem_client(*this);
        }catch( Exception& ){}
    }

    // if process_msg receives error, this is a critical error, IOService is not operational.
    void process_msg(const canmsg_t& in, const IOError& e){
        if(e) {
            lock_t lock(io_service_err_mtx_);
            io_service_err_ = e;
        }
        else {
            on_msg_(in);
        }
    }

    IOError io_error() {
        lock_t lock(io_service_err_mtx_);
        return  io_service_err_;
    }
    
    void clear_io_error() {
        lock_t lock(io_service_err_mtx_);
        io_service_err_.clear();
    }

protected:
    IOService& my_io_service_;

    /// Successors must implement CAN message processing
    virtual void on_msg_( const canmsg_t&) = 0;

private:
    IOError io_service_err_;
    boost::mutex io_service_err_mtx_;
};

inline void IOService::add_client( IOClient& c )
{
    lock_t lock(clients_mtx_);
    if(error_){
        throw error_;
    }
    c.clear_io_error();
    clients_.push_back(&c);
}

inline void IOService::rx_thread()
{
    // resets clients on start (cause rx thread may be restarted arter IOService reset)
    lock_t lock(clients_mtx_);
    for_(IOClient* c, clients_ ){
        c->clear_io_error();
    }
    lock.unlock();
    canmsg_t in;
    IOError e;
    std::vector<IOClient*> clients_copy_;
    try {
        while( !boost::this_thread::interruption_requested() ){
            if(can_iface_->recv(in,RX_TIMEOUT)){
                lock.lock();
                clients_copy_ = clients_;
                lock.unlock();
                for_(IOClient* c, clients_copy_ ){
                    c->process_msg(in,e);
                }
                //TMP
                //static size_t i = 0;
                //fprintf(stdout, "r:%d\n", ++i);
            }
        }
    }
    catch( const IOException& ex ){
        lock_t lock1(clients_mtx_);
        clients_copy_ = clients_;
        lock_t lock2(send_mtx_);
        error_ = IOError(ex.what());
        lock1.unlock();
        lock2.unlock();
        for_(IOClient* c, clients_copy_ ){
            c->process_msg(in, error_);
        }
    }
}

CANIO_NS_END

#endif // IO_SERVICE_HPP
