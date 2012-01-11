/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Triggers Subsystem
 ****************************************************************************
 @details   Triggers are used to handle async CAN I/O events
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef TRIGGERS_HPP
#define TRIGGERS_HPP
#include "io_service.hpp"
#include <set>

CANIO_NS_START

/** Base Trigger class
    Trigger/TriggersWaiter abstraction used to handle async CAN events. */
class Trigger 
{
public:
    Trigger():activ_(false){}

    Trigger(const Trigger&):activ_(false){}

    virtual ~Trigger(){}

    // returns true if msg activates the trigger
    bool activated(const canmsg_t& in) {
        if(activated_(in)){
            lock_t lock(mtx_);
            activ_ = true;
            return true;
        }
        return false;
    }

    bool activated() const {
        lock_t lock(mtx_);
        return activ_;
    }

    void reset() {
        lock_t lock(mtx_);
        activ_ = false;
    }

protected:
    Trigger& operator=( const Trigger& o ) {
        if(this!=&o) { this->reset(); }
        return *this;
    }

    virtual bool activated_( const canmsg_t& ) = 0;

private:
    mutable boost::mutex mtx_;
    bool activ_;
};

/** Trigger Activated by CAN messages with ids that pass filter.*/
class MsgIdTrigger : public Trigger
{
public:
    MsgIdTrigger(uint32_t code, uint32_t mask):
        code_(code&mask), mask_(mask){}
    MsgIdTrigger(uint32_t msg_id):
        code_(msg_id), mask_(0xFFFFFFFF){}
    MsgIdTrigger( const MsgIdTrigger& o ):
        code_(o.code_), mask_(o.mask_){}
    MsgIdTrigger& operator=( const MsgIdTrigger& o ){
        if(this!=&o){
            Trigger::operator =(o);
            code_ = o.code_;
            mask_ = o.mask_;
        }
        return *this;
    }

private:
    uint32_t code_;
    uint32_t mask_;
    virtual bool activated_(const canmsg_t& in){
        return (in.id & mask_) == code_;
    }
};

class MsgDataTrigger : public Trigger
{
public:
    MsgDataTrigger( uint32_t msg_id, uint64_t data_code, uint64_t data_mask ):
        msg_id_(msg_id), data_code_(data_code &data_mask ), data_mask_(data_mask){}
    MsgDataTrigger( uint32_t msg_id, uint64_t data):
        msg_id_(msg_id), data_code_(data), data_mask_(0xFFFFFFFFFFFFFFFFLL){}
    MsgDataTrigger( const MsgDataTrigger& o ):
        msg_id_(o.msg_id_), data_code_(o.data_code_),
        data_mask_(o.data_mask_){}
    MsgDataTrigger& operator=( const MsgDataTrigger& o ) {
        if(this!=&o){
            Trigger::operator =(o);
            msg_id_ = o.msg_id_;
            data_code_ = o.data_code_;
            data_mask_ = o.data_mask_;
        }
        return *this;
    }

private:
    uint32_t msg_id_;
    uint64_t data_code_;
    uint64_t data_mask_;
    virtual bool activated_(const canmsg_t & in){
        if( msg_id_ == in.id ){
            uint64_t* p_data = (uint64_t*)&in.data[0];
            return ((*p_data)&data_mask_) == data_code_;
        }
        return false;
    }
};

/** This class is to do wait or wait_any operations on triggers.
    Trigger/TriggersWaiter abstraction used to handle async CAN events.
    @code
        ....
        TriggersWaiter waiter(io_service);
        waiter.add(a_trigger);
        ... initiate operation that will activate the trigger
        if( waiter.wait(1000) == false ) { // Timeout
            ... handle timeout
        }
        ....
    @endcode
*/
class TriggersWaiter : public IOClient
{
private:
    boost::mutex mtx_;
    boost::condition_variable cond_var_;
 
   enum STATE {
        ST_BACKGROUND,
        ST_WAIT_ANY,
        ST_WAIT
    };

    int state_;
    typedef std::set<Trigger*> triggers_cont_t;
    triggers_cont_t not_triggered_items_;
    triggers_cont_t triggered_items_;

    virtual void on_msg_(const canmsg_t& in){
        lock_t lock(mtx_);
        // process 'in' message by all not yet triggered triggers
        triggers_cont_t::iterator it = not_triggered_items_.begin();
        while(it!=not_triggered_items_.end()){
            if( (*it)->activated(in) ){
                triggered_items_.insert(*it);
                not_triggered_items_.erase(it++);
            }
            else
                ++it;
        }
        //
        if( state_ == ST_WAIT ){
            if( not_triggered_items_.empty() ){
                cond_var_.notify_one();
            }
        }
        else
        if( state_ == ST_WAIT_ANY ){
            if(!triggered_items_.empty()){
                cond_var_.notify_one();
            }
        }
    }

public:
    TriggersWaiter( IOService& io ) : IOClient(io), state_(ST_BACKGROUND) {}
    ~TriggersWaiter(){}
    /** Adds trigger to internal triggers collection.
        The trigger will be removed from that collection when it will be activated
        or when TriggersWaiter will be reset
    */
    void add(Trigger& t){
        lock_t lock(mtx_);
        t.reset();
        not_triggered_items_.insert(&t);
    }

    /** Block until all triggers activated or timeout.
        In any case TriggersWaiter removes all activated triggers from internal collection.
        In 'timeout' case not activated triggers remain in internal collection.
        Call reset method to remove all triggers.
    */
    bool wait(uint timeout){
        lock_t lock(mtx_);
        if( not_triggered_items_.empty() ){
            triggered_items_.clear();
            return true;
        }
        state_ = ST_WAIT;
        bool res = true;
        while( !not_triggered_items_.empty() ){
            if(!cond_var_.timed_wait(lock, boost::posix_time::milliseconds(timeout))){
                res = false;
                break;
            }
        }
        triggered_items_.clear();
        state_ = ST_BACKGROUND;
        return res;
    }

    /** Block until any trigger activated or timeout.
        TriggersWaiter doesn't remove not activated triggers at function exit. */
    bool wait_any(uint timeout){
        lock_t lock(mtx_);
        if( !triggered_items_.empty() ){
            triggers_cont_t::iterator it = triggered_items_.begin();
            triggered_items_.erase(it);
            return true;
        }
        state_ = ST_WAIT_ANY;
        while( triggered_items_.empty() ){
            if(!cond_var_.timed_wait(lock, boost::posix_time::milliseconds(timeout))){
                state_ = ST_BACKGROUND;
                return false;
            }
        }
        state_ = ST_BACKGROUND;
        triggers_cont_t::iterator it = triggered_items_.begin();
        triggered_items_.erase(it);
        return true;
    }

    /// Clears handled triggers
    void reset() {
        lock_t lock(mtx_);
        triggered_items_.clear();
        not_triggered_items_.clear();
    }

};

CANIO_NS_END

#endif // TRIGGERS_HPP
