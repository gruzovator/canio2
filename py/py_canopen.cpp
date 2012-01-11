/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     CANopen part export
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable: 4100 )// too many 'unreferenced formal parameter' warnings
#endif
#include "py.hpp"

//-----------------------------------------------------------------------------
// Utils & Wrappers
//-----------------------------------------------------------------------------
void co_ex_translator(const CANopenException& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what().c_str());
}

//-----------------------------------------------------------------------------
// CANopen lib part export
//-----------------------------------------------------------------------------
void export_canopen()
{
    register_exception_translator<CANopenException>(co_ex_translator);
    class_<CANopenNode, boost::noncopyable> co_node("CANopenNode", init<int, IOService&>()[with_custodian_and_ward<1,3>()]);
    scope CO_SCOPE = co_node;

    enum_<CANopenNode::NODE_STATES>("STATES")
            .value("INITIALIZATION", CANopenNode::NODE_ST_INITIALIZATION)
            .value("DISCONNECTED", CANopenNode::NODE_ST_DISCONNECTED)
            .value("CONNECTING", CANopenNode::NODE_ST_CONNECTING)
            .value("PREPARING", CANopenNode::NODE_ST_PREPARING)
            .value("STOPPED", CANopenNode::NODE_ST_STOPPED)
            .value("OPERATIONAL", CANopenNode::NODE_ST_OPERATIONAL)
            .value("PRE_OPERATIONAL", CANopenNode::NODE_ST_PREOPERATIONAL)
            ;

    enum_<CANopenNode::NODE_COMMANDS>("CMDS")
            .value("START", CANopenNode::NODE_CMD_START)
            .value("STOP", CANopenNode::NODE_CMD_STOP)
            .value("ENTER_PRE_OP", CANopenNode::NODE_CMD_ENTER_PRE_OP)
            .value("RESET", CANopenNode::NODE_CMD_RESET_NODE)
            .value("RESET_COMMUNICATION", CANopenNode::NODE_CMD_RESET_COMMUNICATION)
            ;
#define EXPORT_CO_OPERATION_(n)\
    void (CANopenNode::*ws##n)(uint16_t, uint8_t, const int##n##_t& ) = &CANopenNode::write<int##n##_t>;\
    void (CANopenNode::*wu##n)(uint16_t, uint8_t, const uint##n##_t& ) = &CANopenNode::write<uint##n##_t>;\
    int##n##_t (CANopenNode::*rs##n)(uint16_t, uint8_t) = &CANopenNode::read<int##n##_t>;\
    uint##n##_t (CANopenNode::*ru##n)(uint16_t, uint8_t) = &CANopenNode::read<uint##n##_t>;\
    void (CANopenNode::*ws_##n)(uint16_t, const int##n##_t& ) = &CANopenNode::write<int##n##_t>;\
    void (CANopenNode::*wu_##n)(uint16_t, const uint##n##_t& ) = &CANopenNode::write<uint##n##_t>;\
    int##n##_t (CANopenNode::*rs_##n)(uint16_t) = &CANopenNode::read<int##n##_t>;\
    uint##n##_t (CANopenNode::*ru_##n)(uint16_t) = &CANopenNode::read<uint##n##_t>;\
    co_node.def("ws"#n, ws##n);\
    co_node.def("wu"#n, wu##n);\
    co_node.def("rs"#n, rs##n);\
    co_node.def("ru"#n, ru##n);\
    co_node.def("ws"#n, ws_##n);\
    co_node.def("wu"#n, wu_##n);\
    co_node.def("rs"#n, rs_##n);\
    co_node.def("ru"#n, ru_##n)

    std::string (CANopenNode::*rstr)(uint16_t, uint8_t) = &CANopenNode::read;
    std::string (CANopenNode::*rstr_)(uint16_t) = &CANopenNode::read;
    co_node.def("rstr", rstr).def("rstr",rstr_);

    EXPORT_CO_OPERATION_(8);
    EXPORT_CO_OPERATION_(16);
    EXPORT_CO_OPERATION_(32);

    void (CANopenNode::*wf)(uint16_t, uint8_t, const float&) = &CANopenNode::write<float>;
    void (CANopenNode::*wf_)(uint16_t, const float&) = &CANopenNode::write<float>;
    float (CANopenNode::*rf)(uint16_t, uint8_t) = &CANopenNode::read<float>;
    float (CANopenNode::*rf_)(uint16_t) = &CANopenNode::read<float>;
    co_node.def("wf", wf).def("wf", wf_).def("rf", rf).def("rf", rf_);

    co_node.def("send_cmd", &CANopenNode::send_cmd)
            .def("reset", &CANopenNode::reset)
            .add_property("id", &CANopenNode::id)
            .add_property("operation_timeout", &CANopenNode::get_operation_timeout, &CANopenNode::set_operation_timeout)
            .add_property("state", &CANopenNode::get_state, &CANopenNode::try_set_state)
            ;

    //class_<CANopenNode::BootTrg, bases<Trigger>, boost::noncopyable>("BootTrg", init<int>());
}

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

