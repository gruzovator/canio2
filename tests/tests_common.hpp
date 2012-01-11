/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Common tests stuff
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/

#ifndef TESTS_COMMON_HPP_
#define TESTS_COMMON_HPP_
#include "canio2.hpp"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#define SLEEP(ms) boost::this_thread::sleep(boost::posix_time::millisec(ms))

using namespace std;
using namespace CANIO2;

#define PRINT(s)\
    {\
        ostringstream os__;\
        os__<<s;\
        fprintf( stdout, "%s\n", os__.str().c_str());\
    }

can_iface_sp_t init_can_iface( std::string& plg_name,
                                std::string& id,
                                std::string& param )
{
    // Module Loading
    can_module_sp_t m = MakeModule(plg_name);

    // Module Info
    cout << "\n*** Module " << plg_name << " Information ***" << endl;
    cout << "Module name: " << m->name() << endl;
    cout << "Module id_name: " << m->id_name() << endl;
    cout << "Module param_name: " << m->param_name() << endl;
    for_(const std::string& id, m->ids()){
        cout << "\tModule id: " << id << endl;
    }
    for_(const std::string& param, m->params()){
        cout << "\tModule param: " << param << endl;
    }
    cout << "*** End of Module Information ***" << endl;

    // CAN Iface making
    cout << "\nEnter can iface " << m->id_name() << ": ";
    if(!id.empty()) {
        cout << id << endl;
    }
    else
        cin >> id;
    cout << "Enter can iface " <<m->param_name() << ": ";
    if(!param.empty()){
        cout << param << endl;
    }
    else
        cin >> param;

    return m->make_iface(id, param);
}

#endif
