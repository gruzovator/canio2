/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Common CANIO2 headers and types
 ****************************************************************************
 @details   
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdexcept>
#include <vector>

#if defined(_MSC_VER) && (_MSC_VER<1600)
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
#include <stdint.h>
#endif

typedef unsigned int uint;

///  CAN packet structure
typedef struct {
    uint32_t id; ///< 11 bit id
    uint8_t len; ///< data lenght 0-8
    uint8_t rtr; ///< RTR flag
    uint8_t data[8];
} canmsg_t;


#define CANIO_NS_START namespace CANIO2 {
#define CANIO_NS_END }

CANIO_NS_START

/// canio2 base error class
class Error{
public:
    Error( const std::string& info = "" ) : info_(info){}
    virtual ~Error(){}
    operator bool() const {
        return !info_.empty();
    }
    const std::string& what() const {return info_;}
    void clear() { info_.clear(); }
private:
    std::string info_;
};

typedef Error Exception;

CANIO_NS_END

// Boost
#ifndef CAN_PLUGIN
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
typedef boost::mutex::scoped_lock lock_t;
#define for_ BOOST_FOREACH
#endif

#endif // COMMON_HPP
