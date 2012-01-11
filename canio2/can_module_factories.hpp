/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file 
 ****************************************************************************
 @brief     CANModule factories
 ****************************************************************************
 @details   Factory functions to make CAN modules
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#ifndef CAN_MODULE_FACTORIES_HPP_
#define CAN_MODULE_FACTORIES_HPP_

#include "detail/can_ext_module.hpp"

CANIO_NS_START

/// Makes list of all available modules
inline std::vector<can_module_sp_t> GetModules()
{
    namespace fs = boost::filesystem;
    fs::path dir(".");
    fs::directory_iterator end_it;
    std::vector<can_module_sp_t> res;
    for( fs::directory_iterator it(dir); it != end_it; ++it ){
        if( fs::extension(*it) != CAN_PLUGIN_FILE_EXT_ )
            continue;
        try {
            res.push_back(
                can_module_sp_t(new details::CANExtModule(it->path().string()))
            );
        }
        catch( Error& ){
            continue;
        }
    }
    return res;
}

/** Modules Factory
    @param name of module, lib name (without extension) in case of external module
    @throw CANModuleException */
inline can_module_sp_t MakeModule(const std::string& name ) {
    std::string lib_name = name;
    // add extension if there is no one
    if( lib_name.find(".") == std::string::npos ){
        lib_name += CAN_PLUGIN_FILE_EXT_;
    }
    // add plg_ prefix
    if( lib_name.substr(0,4)!= "plg_"){
        lib_name = std::string("plg_") + lib_name;
    }
    return can_module_sp_t(new details::CANExtModule(lib_name));
}

CANIO_NS_END

#endif /* CAN_MODULE_HPP_ */
