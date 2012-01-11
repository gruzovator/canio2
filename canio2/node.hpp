/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Node abstraction for CANOpen and 'CANopen like' protocols (e.g. PMD)
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef NODE_HPP
#define NODE_HPP

#include "io_service.hpp"

CANIO_NS_START

/** Common error for IOClients*/
struct OperationTimeoutError : Error {
    OperationTimeoutError() : Error("Operation Timeout"){}
};
typedef OperationTimeoutError OperationTimeoutExcept;


/** Abstract class for CAN hw that compatible with CANopen 
    Actually, at the present this class only keeps node id 
    and monitors IOService errors.*/
class Node : public IOClient
{
public:
    /// This is a reasonable default value (in ms)
    static const uint DEFAULT_OPERATION_TIMEOUT = 1000;

public:
    Node(int id, IOService& io) : IOClient(io),my_id_(id),
        op_timeout_ms_(DEFAULT_OPERATION_TIMEOUT){
    }

    int id() const { return my_id_; }

    uint get_operation_timeout(){ return op_timeout_ms_; }
    void set_operation_timeout(uint ms){ op_timeout_ms_ = ms; }

protected:
    virtual void on_msg_(const canmsg_t& in) = 0;
    int my_id_;
    uint op_timeout_ms_;
};

CANIO_NS_END

#endif // NODE_HPP
