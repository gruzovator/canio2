/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file 
 ****************************************************************************
 @brief     CAN external module
 ****************************************************************************
 @details   External module is a dynamic link library.
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef CAN_EXT_MODULE_HPP_
#define CAN_EXT_MODULE_HPP_

#include "can_module.hpp"
#include "can_plugin.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "detail/logger.hpp"

#ifdef _WIN32
#include <windows.h>
#define CAN_PLUGIN_FILE_EXT_ ".dll"
#define LOAD_LIB_(file) LoadLibraryA(file)
#define GET_PROC_ADDR_(lib,name) GetProcAddress(lib,name)
#define UNLOAD_LIB_(lib) FreeLibrary(lib)
#define LIB_LOAD_ERROR_() GetLastError()
#define LIB_HANDLE_ HINSTANCE
#endif
#ifdef __QNXNTO__
#include <dlfcn.h>
#define CAN_PLUGIN_FILE_EXT_ ".so"
#define LOAD_LIB_(file) dlopen(file, RTLD_NOW)
#define GET_PROC_ADDR_(lib,name) dlsym(lib,name)
#define UNLOAD_LIB_(lib) dlclose(lib)
#define LIB_LOAD_ERROR_() dlerror()
#define LIB_HANDLE_ void*
#endif

CANIO_NS_START

namespace details {

// Utility
inline strlist_t csv_to_strlist( const char* csv_str )
{
    strlist_t items;
    if( strlen(csv_str)!=0 ) {
        boost::split( items, csv_str, boost::is_punct() );
    }
    return items;
}

// CANExtModule is CANModule that is loaded from dll (external module)
class CANExtModule : public boost::enable_shared_from_this<CANExtModule>, public CANModule
{
    using boost::enable_shared_from_this<CANExtModule>::shared_from_this;
private:
    std::string name_;
    std::string id_name_;
    std::string param_name_;

    LIB_HANDLE_ lib_handle_;
    // pointers to functions from dll
    // .. module functions
    canm_name_f canm_name;
    canm_params_f canm_params;
    canm_ids_f canm_ids;    
    canm_make_iface_f canm_make_iface;
    canm_err_f canm_err;
    // .. iface functions
    can_del_f can_del;
    can_recv_f can_recv;
    can_send_f can_send;
    can_set_acc_filter_f can_set_acc_filter;
    can_add_id_filter_f can_add_id_filter;
    can_rem_id_filter_f can_rem_id_filter;
    can_err_f can_err;

private:
    struct Iface : CANIface {
        //
        boost::shared_ptr<CANExtModule> my_module_; // ref to module to keep it loaded
        std::string id_string_;
        can_iface_t iface_;

        //
        Iface( boost::shared_ptr<CANExtModule> module, 
                const std::string& iface_id, can_iface_t i ):
            my_module_(module),
            id_string_(iface_id),
            iface_(i){}

        ~Iface(){ my_module_->can_del(iface_); }
    
        virtual bool send(const canmsg_t& m, uint timeout_ms ) {
            uint e;
            bool res = my_module_->can_send(iface_, m, timeout_ms, &e);
            if(res == false && e>0 ) {
                char buf[1024];
                my_module_->can_err(e,buf,sizeof buf);
                LOG_DBG_(buf);
                throw IOException(std::string("TX Error: ") + buf );
            }
            return res;
        }

        virtual bool recv(canmsg_t& m, uint timeout_ms) {
            uint e = 0;
            bool res = my_module_->can_recv(iface_, &m, timeout_ms, &e);
            if(res == false && e>0 ) {
                char buf[1024];
                my_module_->can_err(e,buf,sizeof buf);
                LOG_DBG_(buf);
                throw IOException(std::string("RX Error: ") + buf );
            }
            return res;
        }
    
        virtual void set_acc_filter(uint code, uint mask){
            my_module_->can_set_acc_filter(iface_,code,mask);
        }
        virtual void add_id_filter(uint code, uint mask){
            my_module_->can_add_id_filter(iface_,code,mask);
        }
        virtual void rem_id_filter(uint code, uint mask){
            my_module_->can_rem_id_filter(iface_,code,mask);
        }
    
        virtual std::string id() const {
            return id_string_;
        }
    
        can_module_sp_t module() const {
            return my_module_;
        }
    };

public:
    CANExtModule( const std::string& dll_name ){
        namespace fs = boost::filesystem;
        fs::path file(dll_name);
        if( !fs::exists(file) ){
            throw CANModuleException(std::string("There is no file ") + dll_name);
        }
        LOG_DBG_("Trying to load lib: " << dll_name );
        lib_handle_ = LOAD_LIB_(dll_name.c_str());
        if(!lib_handle_){
            std::ostringstream error;
            error << "Can't load lib: " << dll_name << ". Error: " << LIB_LOAD_ERROR_();
            throw CANModuleException(error.str());
        }
        //------------------
        // Methods mapping
        //------------------
        struct InvalidLibIface : CANModuleException {
            InvalidLibIface(const std::string& dll_name, const std::string& f):
                CANModuleException(std::string("Lib ") + dll_name +" has wrong iface. "
                    "It doesn't export function: " + f){}
        };
    
        #define LOAD_F_(f)\
        f = (f##_f)GET_PROC_ADDR_(lib_handle_, #f);\
        if(!f){\
            throw InvalidLibIface(dll_name,#f);\
        }
        #define LOAD_M_(m)\
            LOAD_F_(m)
    
        canm_version_f canm_version;
        LOAD_F_(canm_version);
        LOG_DBG_("Plugin Lib Version: " << canm_version() );
    
        LOAD_M_(canm_name);
        name_ = canm_name();
    
        canm_id_name_f canm_id_name;
        LOAD_F_(canm_id_name);
        id_name_ = canm_id_name();
    
        canm_param_name_f canm_param_name;
        LOAD_F_(canm_param_name);
        param_name_ = canm_param_name();
    
        LOAD_M_(canm_params);
        LOAD_M_(canm_ids);
        LOAD_M_(canm_make_iface);
        LOAD_M_(canm_err);
        LOAD_M_(can_del);
        LOAD_M_(can_recv);
        LOAD_M_(can_send);
        LOAD_M_(can_set_acc_filter);
        LOAD_M_(can_add_id_filter);
        LOAD_M_(can_rem_id_filter);
        LOAD_M_(can_err);
        LOG_DBG_("Lib '" << dll_name << "' successfully loaded");
        #undef LOAD_M_
        #undef LOAD_F_
    }

    ~CANExtModule() {
        UNLOAD_LIB_(lib_handle_);
        LOG_DBG_("Ext module " << name_ << " unloaded.");
    }
    
    virtual std::string name() const { return name_; }
    
    virtual std::string id_name() const { return id_name_; }
    
    virtual std::string param_name() const { return param_name_; }
    
    strlist_t CANExtModule::params() const 
    {
        char buf[1024];
        canm_params(buf, sizeof buf);
        return csv_to_strlist(buf);
    }

    strlist_t CANExtModule::ids() const 
    {
        char buf[1024];
        canm_ids(buf, sizeof buf);
        return csv_to_strlist(buf);
    }

    virtual can_iface_sp_t make_iface( const std::string& id,
                                       const std::string& param ) {
        uint err_code;
        can_iface_sp_t sp_iface;
        can_iface_t iface = canm_make_iface(id.c_str(), param.c_str(), &err_code);
        if(iface == 0){
            char buf[1024];
            canm_err(err_code, buf, sizeof buf);
            LOG_DBG_("make_iface error. " << buf);
            throw CANModuleException(std::string("Can't make iface. Error: ")+buf);
        }
        else
            sp_iface = can_iface_sp_t(new Iface(shared_from_this(), id, iface));
        return sp_iface;
    }
};

} // namespace details

CANIO_NS_END

#endif /* CAN_EXT_MODULE_HPP_ */
