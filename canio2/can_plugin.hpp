/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     CAN plugin (external module) defintions
 ****************************************************************************
 @details   That functions must be implemented in every plugin.\n
            Each F_X is a macros. First arg is return type,
            second - function name, others - params.\n
            Macros are expanded depending on 'CAN_PLUGIN' defintion.
            Attention! Define <b>CAN_PLUGIN</b> to build a plugin.
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef CANIO_PLUGIN_HPP
#define CANIO_PLUGIN_HPP

#include "canio_common.hpp"

/// Opaque pointer represents CAN iface handle
typedef struct can_iface *can_iface_t;

#ifndef CAN_PLUGIN

#define F_0(res_type, name)\
    res_type name();\
    typedef res_type (*name##_f)();

#define F_1(res_type, name, p)\
    res_type name(p);\
    typedef res_type (*name##_f)(p);

#define F_2(res_type, name, p1, p2)\
    res_type name(p1,p2);\
    typedef res_type (*name##_f)(p1,p2);

#define F_3(res_type, name, p1, p2, p3)\
    res_type name(p1,p2,p3);\
    typedef res_type (*name##_f)(p1,p2,p3);

#define F_4(res_type, name, p1, p2, p3, p4)\
    res_type name(p1,p2,p3, p4);\
    typedef res_type (*name##_f)(p1,p2,p3, p4);

#else

#ifdef _WIN32
#define PREFIX extern "C" __declspec(dllexport)
#else
#define PREFIX extern "C"
#endif

#define F_0(res_type, name)\
    PREFIX res_type name()

#define F_1(res_type, name, p)\
    PREFIX res_type name(p)

#define F_2(res_type, name, p1, p2)\
    PREFIX res_type name(p1,p2)

#define F_3(res_type, name, p1, p2, p3)\
    PREFIX res_type name(p1,p2,p3)

#define F_4(res_type, name, p1, p2, p3, p4)\
    PREFIX res_type name(p1,p2,p3,p4)

#endif

///@name Common functions
//@{
F_0(const char*, canm_version);
F_0(const char*, canm_name);
F_0(const char*, canm_id_name);
F_0(const char*, canm_param_name);
F_2(void, canm_params, char* buf, uint buf_sz);
F_2(void, canm_ids, char* buf, uint buf_sz);///< csv string of possible ids values
F_3(can_iface_t, canm_make_iface, const char* id, const char* param, uint* p_error);
F_3(void, canm_err, uint error, char* buf, uint buf_sz); ///< get common module error description
//@}
///@name Iface functions
//@{
F_1(void, can_del, can_iface_t); ///< Iface destructor
F_4(bool, can_send, can_iface_t, const canmsg_t&, uint timeout_ms, uint* p_error);
F_4(bool, can_recv, can_iface_t, canmsg_t*, uint timeout_ms, uint* p_error);
F_3(void, can_set_acc_filter, can_iface_t, uint code, uint mask); ///< to remove filter pass zero code and mask
F_3(void, can_add_id_filter, can_iface_t, uint code, uint mask);
F_3(void, can_rem_id_filter, can_iface_t, uint code , uint mask);
F_3(void, can_err, uint error, char* buf, uint buf_sz); ///< get iface error description
//@}

#ifndef CAN_PLUGIN
#undef F_0
#undef F_1
#undef F_2
#undef F_3
#undef F_4
#undef PREFIX
#endif

#endif // CANIO_PLUGIN_HPP
