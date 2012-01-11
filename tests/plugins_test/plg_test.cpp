/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief		CAN plugins test
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#include "tests_common.hpp"
#include <stdexcept>

using namespace std;

void ABOUT()
{
    cout << "CAN external module 'ping-pong' test." << endl;
    cout << " Usage: " << PROG_NAME << " lib_name [id param cycles_n node_id]" << endl;
}

int main(int argc, char *argv[]) {
    if(argc < 2 ){
		ABOUT();
        return EXIT_FAILURE;
    }

    std::string plg_name(argv[1]);
    std::string id;
    std::string param;
    if(argc>2) id = argv[2];
    if(argc>3) param = argv[3];
    try {
        can_iface_sp_t iface = init_can_iface(plg_name, id, param);
        //-----------------------
        // Do ping/pong test
        //-----------------------

        int cycles_n = 10000;
        if(argc>4){
            cycles_n  = atoi(argv[4]);
        }
        int node_id = 4;
        if(argc>5){
            node_id = atoi(argv[5]);
        }
        cout << "\n*** TX/RX test. Do " << cycles_n <<
                " cycles. Node id: " << node_id << " ***" << endl;

        canmsg_t out, in;
        out.id = 0x600+node_id;
        out.rtr = 0;
        out.len = 8;
        fill_n(out.data, 8, 0);
        out.data[0] = 0x40;
        out.data[1] = 0x00;
        out.data[2] = 0x10;

        using namespace boost::posix_time;
        ptime start = microsec_clock::local_time();
        while(cycles_n-->0){
            if(iface->send(out, 100) == false){
                cout << "TX timeout" << endl;
                break;
            }
            if(iface->recv(in,100) == false){
                cout << "RX timeout" << endl;
                break;
            }
        }
        ptime stop = microsec_clock::local_time();
        cout << "  Duration: " << stop - start << endl;
        cout << "*** End of High load test ***" << endl;
    }
    catch( const Exception& e ){
        cout << "CANIO2 Exception: " << e.what() << endl;
    }

//    int code, mask;
//    cout << "Enter acceptance filter code: ";
//    cin >> code;
//    cout << "Enter acceptance filter mask: ";
//    cin >> mask;
//    iface->set_acc_filter(code,mask);
//    cout << "\n*** Waiting for incoming packets ***" << endl;
//    while(1){
//        if(iface->recv(&in,300)){
//            printf("ID:0x%X Len:%d RTR:%d Data:%x %x %x %x %x %x %x %x\n",
//                   in.id, in.len, in.rtr, in.data[0],
//                   in.data[1],in.data[2],in.data[3],in.data[4],
//                   in.data[5],in.data[6],in.data[7]);
//        }
//    }
    return EXIT_SUCCESS;
}

