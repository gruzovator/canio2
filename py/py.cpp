/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Python Library
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
#include <iostream>
#include <iomanip>

//-----------------------------------------------------------------------------
// Utils & Wrappers
//-----------------------------------------------------------------------------
struct CANMsg : canmsg_t, wrapper<canmsg_t> {

    CANMsg(){}
    CANMsg(const canmsg_t& o):canmsg_t(o){}

    template<typename T, size_t OFFSET>
    void set_data( T d ){
        memcpy(&data[OFFSET], &d, sizeof(T));
    }

    template<typename T, size_t OFFSET>
    T get_data(){
        T res;
        memcpy(&res, &data[OFFSET], sizeof(T));
        return res;
    }
};

std::ostream& operator<<( std::ostream& os, const CANMsg& msg ){
    os << "ID:" << std::hex <<std::showbase << (int)msg.id
       << " L:" << std::noshowbase << (int)msg.len;
    if(msg.rtr){
        os << " RTR: ON";
    }
    os << " D:" << std::setfill('0');
    for( int i=0; i<8 && i<msg.len; ++i )
        os << std::setw(2) << (int)msg.data[i] << ' ';
    return os;
}

class ScopedGILRelease {
public:
    ScopedGILRelease() {
        gstate_ = PyGILState_Ensure(); }
    ~ScopedGILRelease() {
        PyGILState_Release(gstate_);
    }
private:
    PyGILState_STATE gstate_;
};

struct TriggersWaiterWrapper : TriggersWaiter {
    TriggersWaiterWrapper( IOService& io ):TriggersWaiter(io){}
    bool wait(uint timeout){
        bool res;
        Py_BEGIN_ALLOW_THREADS;
        res = TriggersWaiter::wait(timeout);
        Py_END_ALLOW_THREADS;
        return res;
    }
    bool wait_any(uint timeout){
        bool res;
        Py_BEGIN_ALLOW_THREADS;
        res = TriggersWaiter::wait_any(timeout);
        Py_END_ALLOW_THREADS;
        return res;
    }
};

// At the present lib doesn't export  IOClient or Triggers (for usign as base clas)
// cause sucsessors of IOClient and Trigger are called from other thread.
// This lead to to some problems. So lib just supply ready Triggers.


//struct IOClientWrapper : IOClient, wrapper<IOClient>
//{
//    IOClientWrapper(IOService& io) : IOClient(io),wrapper<IOClient>(){}

//    virtual void on_msg( const canmsg_t& in ) {
//        override f = this->get_override("on_msg");
//        if(f){
//            ScopedGILRelease r;
//            f(in);
//        }
//    }
//};

//struct TriggerWrapper : Trigger, wrapper<Trigger>
//{
//    virtual bool activated_(const canmsg_t& in ) {
//        override f = this->get_override("activated_");
//        if(f){
//            ScopedGILRelease r;
//            return f(in);
//        }
//        return Trigger::activated_(in);
//    }
//};

//-----------------------------------------------------------------------------
// Exceptions translators
//-----------------------------------------------------------------------------
void canio_ex_translator(const Exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what().c_str());
}

void can_module_ex_translator(const CANModuleException& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what().c_str());
}

void can_iface_ex_translator(const IOException& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what().c_str());
}
void timeout_ex_translator(const OperationTimeoutExcept& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what().c_str());
}

//-----------------------------------------------------------------------------
// CANIO lib
//-----------------------------------------------------------------------------
BOOST_PYTHON_MODULE(canio2)
{
    PyEval_InitThreads(); // cause this lib uses GIL locks to use python callbacks
    register_exception_translator<Exception>(canio_ex_translator);
    register_exception_translator<CANModuleException>(can_module_ex_translator);
    register_exception_translator<IOException>(can_iface_ex_translator);
    register_exception_translator<OperationTimeoutExcept>(timeout_ex_translator);

//    class_<CANIOError>("Error", init<std::string>())
//            .def(init<>())
//            .def(operator!(self))
//            .def("what", &Error::what, return_value_policy<copy_const_reference>() )
//            ;

    class_<CANMsg>("CANMsg")
            .def_readwrite("id",&CANMsg::id)
            .def_readwrite("rtr",&CANMsg::rtr)
            .def_readwrite("len",&CANMsg::len)
            .add_property("data",&CANMsg::get_data<uint64_t,0>,
                                &CANMsg::set_data<uint64_t,0>)
            .add_property("data_ldw",&CANMsg::get_data<uint32_t,0>,
                                &CANMsg::set_data<uint32_t,0>)
            .add_property("data_hdw",&CANMsg::get_data<uint32_t,4>,
                                &CANMsg::set_data<uint32_t,4>)
            .add_property("data0",&CANMsg::get_data<uint8_t,0>,
                                &CANMsg::set_data<uint8_t,0>)
            .add_property("data1",&CANMsg::get_data<uint8_t,1>,
                                &CANMsg::set_data<uint8_t,1>)
            .add_property("data2",&CANMsg::get_data<uint8_t,2>,
                                &CANMsg::set_data<uint8_t,2>)
            .add_property("data3",&CANMsg::get_data<uint8_t,3>,
                                &CANMsg::set_data<uint8_t,3>)
            .add_property("data4",&CANMsg::get_data<uint8_t,4>,
                                &CANMsg::set_data<uint8_t,4>)
            .add_property("data5",&CANMsg::get_data<uint8_t,5>,
                                &CANMsg::set_data<uint8_t,5>)
            .add_property("data6",&CANMsg::get_data<uint8_t,6>,
                                &CANMsg::set_data<uint8_t,6>)
            .add_property("data7",&CANMsg::get_data<uint8_t,7>,
                          &CANMsg::set_data<uint8_t,7>)
            .def(self_ns::str(self_ns::self))
            ;

    class_< std::vector<std::string> >("StringList")
        .def(vector_indexing_suite<std::vector<std::string> >());

    class_<CANIface, can_iface_sp_t, boost::noncopyable > can_iface("CANIface", no_init);

    class_<CANModule, can_module_sp_t, boost::noncopyable >("CANModule", no_init)
            .def("name", &CANModule::name)
            .def("id_name", &CANModule::id_name)
            .def("param_name", &CANModule::param_name)
            .def("params", &CANModule::params)
            .def("ids", &CANModule::ids)
            .def("make_iface", &CANModule::make_iface)
            ;

    class_< std::vector<can_module_sp_t> >("ModulesList")
        .def(vector_indexing_suite<std::vector<can_module_sp_t>, true >());

    def("get_modules", GetModules);
    def("make_module", MakeModule);

    can_iface
            .def("id", &CANIface::id)
            .def("module", &CANIface::module)
            .def("send", &CANIface::send)
            .def("recv", &CANIface::recv)
            ;

    class_<IOService,boost::noncopyable>("IOService", init<can_iface_sp_t>())
            .def("reset", &IOService::reset)
            .def("send", &IOService::send)
            ;

//    class_<IOClientWrapper, boost::noncopyable>("IOClient", init<IOService&>()[with_custodian_and_ward_postcall<1,2>()])
//            .def("on_msg", &IOClientWrapper::on_msg)
//            .def("io_error", &IOClient::io_error)
//            .def("clear_io_error", &IOClient::clear_io_error);

    bool (Trigger::*p_activated_getter)() const = &Trigger::activated;
    class_<Trigger, boost::noncopyable>("Trigger", no_init)
            .add_property("activated", p_activated_getter);

    class_<MsgIdTrigger, bases<Trigger> >("MsgIdTrigger",init<uint32_t, uint32_t>())
            .def(init<uint32_t>());

    class_<MsgDataTrigger, bases<Trigger> >("MsgDataTrigger",init<uint32_t, uint64_t, uint64_t>())
            .def(init<uint32_t, uint64_t>());

    class_<TriggersWaiterWrapper, boost::noncopyable>("TriggersWaiter", init<IOService&>()[with_custodian_and_ward_postcall<1,2>()])
            .def("add", &TriggersWaiter::add)
            .def("reset", &TriggersWaiter::reset)
            .def("wait", &TriggersWaiterWrapper::wait)
            .def("wait_any", &TriggersWaiterWrapper::wait_any)
            ;

    export_pmd();
    export_canopen();
}

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
