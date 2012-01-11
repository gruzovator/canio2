/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     CAN plugins test
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator
 ****************************************************************************/
#include "tests_common.hpp"
#include <set>
#include "pmd_node.hpp"

//-----------------------------------------------------------------------------
// Test
//-----------------------------------------------------------------------------
void ABOUT()
{
    cout << " Usage: " << PROG_NAME << " lib_name" << endl;
}

//-----------------------------------------------------------------------------
// EventsMonitor to print interrupts
//-----------------------------------------------------------------------------
struct EventsMonitor : IOClient {
    uint32_t event_id;
    EventsMonitor( int node, IOService& io ): IOClient(io),event_id(0x180+node){}

    void on_msg(const canmsg_t& in){
        if( in.id == event_id ) {
            ostringstream os;
            os << "ID:" << hex << showbase << in.id
               << dec << noshowbase << " Len:" << (int)in.len;
            if(in.rtr)
               os << " RTR:" << (int)in.rtr;
            for(int i=0; i<in.len; ++i){
                os << ' ' << hex << setfill('0') << setw(2) << (int)in.data[i];
            }
            fprintf(stdout, "%s\n", os.str().c_str());
        }
    }
};

//-----------------------------------------------------------------------------
// High level functions
//-----------------------------------------------------------------------------
void pmd_init( PMD& node )
{
//    node.reset(); <--- don't use it, cause it breaks CAN
//    SLEEP(1000);
    node.disable_axes();
}

void pmd_axis_init( PMD::Axis ax, PMD::motor_type_t mt )
{
    assert( mt == PMD::MOTOR_TYPE_DC || mt == PMD::MOTOR_TYPE_BLDC_3 );

    ax.enable();
    ax.set_interrupt_mask(0);
    ax.clear_event_status(PMD::INT_ALL);
    ax.clear_interrupt();

    ax.set_motor_type(mt);
    assert( mt == ax.get_motor_type() );

    PMD::out_mode_t om;
    if(mt == PMD::MOTOR_TYPE_DC )
        om = PMD::OUT_MODE_PWM_SIGN_MAGNITUDE;
    else
        om = PMD::OUT_MODE_PWM_50_50_MAGNITUDE;
    ax.set_output_mode(om);
    assert( om == ax.get_output_mode() );

    PMD::pwm_freq_t pwm_f = PMD::PWM_FREQ_20KHZ;
    ax.set_pwm_freq(pwm_f);
    assert( pwm_f == ax.get_pwm_freq());

    PMD::enc_src_t es = PMD::ENC_SRC_INCREMENTAL;
    ax.set_encoder_source(es);
    assert( es == ax.get_encoder_source());

    PMD::commutation_mode_t c = PMD::COMMUTATION_HALL;
    ax.set_commutation_mode(c);
    assert(c == ax.get_commutation_mode());

    uint16_t op_mode = PMD::OP_MODE_AXIS_ON | PMD::OP_MODE_MOTOR_OUT_ON;
    ax.set_operating_mode(op_mode);
    assert(op_mode==ax.get_operating_mode());

    PMD::capt_src_t cs = PMD::CAPT_SRC_INDEX;
    ax.set_capture_source(cs);
    assert(cs == ax.get_capture_source());

    ax.set_inversion(PMD::SENS_HALL_A, true);
    assert(ax.get_inversion(PMD::SENS_HALL_A));
    ax.set_inversion(PMD::SENS_HALL_B, true);
    assert(ax.get_inversion(PMD::SENS_HALL_B));
    ax.set_inversion(PMD::SENS_HALL_C, true);
    assert(ax.get_inversion(PMD::SENS_HALL_C));

    ax.set_inversion(PMD::SENS_ENC_A, true);
    assert(ax.get_inversion(PMD::SENS_ENC_A));
    ax.set_inversion(PMD::SENS_ENC_B, true);
    assert(ax.get_inversion(PMD::SENS_ENC_B));
    ax.set_inversion(PMD::SENS_ENC_I, true);
    assert(ax.get_inversion(PMD::SENS_ENC_I));

    // Motor limit
    float limit = 40.0;// %
    ax.set_motor_limit(limit);
    assert( abs(limit-ax.get_motor_limit())<=1 );

    // Position loop
    int32_t Kp = 100;
    int32_t Ki = 15;
    int32_t integr_limit = 50;
    int32_t Kd = 70;
    int32_t deriv_time = 40;
    int32_t Kout =  22248;
    int32_t v_ff = 0;
    int32_t a_ff = 0;

    ax.set_position_loop(PMD::PL_PROPORTIONAL_GAIN, Kp);
    assert( Kp == ax.get_position_loop(PMD::PL_PROPORTIONAL_GAIN) );
    ax.set_position_loop(PMD::PL_INTEGRATOR_GAIN, Ki);
    assert( Ki == ax.get_position_loop(PMD::PL_INTEGRATOR_GAIN) );
    ax.set_position_loop(PMD::PL_INTEGRATOR_LIMIT, integr_limit);
    assert( integr_limit == ax.get_position_loop(PMD::PL_INTEGRATOR_LIMIT) );
    ax.set_position_loop(PMD::PL_DERIVATIVE_GAIN, Kd);
    assert( Kd == ax.get_position_loop(PMD::PL_DERIVATIVE_GAIN) );
    ax.set_position_loop(PMD::PL_DERIVATIVE_TIME, deriv_time);
    assert( deriv_time == ax.get_position_loop(PMD::PL_DERIVATIVE_TIME) );
    ax.set_position_loop(PMD::PL_OUTPUT_GAIN, Kout);
    assert( Kout == ax.get_position_loop(PMD::PL_OUTPUT_GAIN) );
    ax.set_position_loop(PMD::PL_VELOCITY_FF_GAIN, v_ff);
    assert( v_ff == ax.get_position_loop(PMD::PL_VELOCITY_FF_GAIN) );
    ax.set_position_loop(PMD::PL_ACCELERATION_FF_GAIN, a_ff);
    assert( a_ff == ax.get_position_loop(PMD::PL_ACCELERATION_FF_GAIN) );
    //Biquad filters disabled
    ax.set_position_loop(PMD::PL_BQF1_ENABLE, 0);
    assert(!ax.get_position_loop(PMD::PL_BQF1_ENABLE));
    ax.set_position_loop(PMD::PL_BQF2_ENABLE, 0);
    assert(!ax.get_position_loop(PMD::PL_BQF2_ENABLE));

    ax.update();


    //-----------------------------
    // Setup Breakpoints and Events
    //-----------------------------

    // HW Error Notification setup

    // Invert AxisIn (error - 1, no error - 0)
    ax.set_inversion(PMD::SENS_AXIS_IN, false);
    assert( !ax.get_inversion(PMD::SENS_AXIS_IN) );
    // Setup BP1 to handle hw error
    uint32_t bp_v = /*what << 16 | which_level */
            PMD::SIG_ST_BIT_AXIS_IN << 16 | PMD::SIG_ST_BIT_AXIS_IN;
    ax.set_bp_value( PMD::BP_1,bp_v);
    assert( bp_v == ax.get_bp_value( PMD::BP_1));
    PMD::bp_trigger_t trig = PMD::BP_TRIG_SIGNAL_STATUS;
    PMD::action_t act = PMD::ACT_NONE;
    int real_src_ax;
    PMD::bp_trigger_t real_trig;
    PMD::action_t real_act;
    ax.set_bp( PMD::BP_1, ax.number(), trig, act);
    ax.get_bp( PMD::BP_1, real_src_ax, real_trig, real_act);
    assert( ax.number() == real_src_ax && trig == real_trig && act == real_act );

    // Motion Complete & Motion Error Notification setup

    // motion complete mode
    ax.set_motion_compl_mode( PMD::MOTION_COMPL_COMMANDED);
    assert( PMD::MOTION_COMPL_COMMANDED == ax.get_motion_compl_mode() );
    // Motion error action  setup
    uint32_t max_pos_err = 2000;
    ax.set_pos_error_limit( max_pos_err);
    assert( max_pos_err == ax.get_pos_error_limit() );
    ax.set_event_action( PMD::EVENT_MOTION_ERR, PMD::ACT_NONE);

    // Setup Interrupts
    uint16_t int_mask = ax.get_interrupt_mask();
    int_mask |= PMD::INT_BP1;
    int_mask |= PMD::INT_MOTION_COMPLETE;
    int_mask |= PMD::INT_MOTION_ERR;
    ax.set_interrupt_mask( int_mask);
    assert( int_mask == ax.get_interrupt_mask() );
    ax.clear_event_status(int_mask);
    ax.clear_interrupt();
}

void move( PMD::Axis& ax, int target_pos )
{
    ax.clear_event_status(PMD::INT_MOTION_COMPLETE | PMD::INT_MOTION_ERR);
    ax.set_operating_mode(PMD::OP_MODE_ALL);
    assert( PMD::OP_MODE_ALL == ax.get_operating_mode());
    ax.set_profile_mode(PMD::PROF_MODE_TRAPEZ);
    assert( PMD::PROF_MODE_TRAPEZ == ax.get_profile_mode());
    ax.set_position( target_pos);
    ax.set_velocity(200000);
    ax.set_acceleration(100);
    ax.set_deceleration(100);
    ax.update();
}

//void axis_init( PMD& node, int ax )
//{
//    //
//    node.write_io(0,0);
//    node.write_io(0,15);
//    //
//    PMD::commutation_mode_t c;
//    node.set_commutation_mode(ax,PMD::COMMUTATION_HALL);
//    c = node.get_commutation_mode(ax);
//    assert(c==PMD::COMMUTATION_HALL);
//    //
//    bool invert;
//    node.set_sens(ax, PMD::SENS_HALL_A, true);
//    invert = node.get_sens(ax, PMD::SENS_HALL_A);
//    assert(invert==true);
//    node.set_sens(ax, PMD::SENS_HALL_B, true);
//    invert = node.get_sens(ax, PMD::SENS_HALL_B);
//    assert(invert==true);
//    node.set_sens(ax, PMD::SENS_HALL_C, true);
//    invert = node.get_sens(ax, PMD::SENS_HALL_C);
//    assert(invert==true);
//    //
//    PMD::enc_source_t enc_src;
//    node.set_encoder_source(ax,PMD::ENC_SRC_NONE);
//    enc_src = node.get_encoder_source(ax);
//    assert(enc_src==PMD::ENC_SRC_NONE);
//    //
//    uint16_t int_mask;
//    node.set_interrupt_mask(ax,0);
//    int_mask = node.get_interrupt_mask(ax);
//    assert( int_mask == 0 );
//    //
//    node.reset_event_status(ax,PMD::EV_STATUS_EVERYTHING);
//    node.clear_interrupt(ax);
//    uint16_t op_mode;
//    node.set_operating_mode(ax,PMD::OP_MODE_AXIS_ON|PMD::OP_MODE_MOTOR_OUTPUT_ON);
//    op_mode = node.get_operating_mode(ax);
//    assert(op_mode == (PMD::OP_MODE_AXIS_ON|PMD::OP_MODE_MOTOR_OUTPUT_ON));
//}

//void go( PMD& node, int ax, float speed )
//{
//    node.set_motor_command(ax,speed);
//    node.update(ax);
//}

//void set_lim_bp( PMD& node, int ax, bool pos )
//{
//    PRINT("Event status: " << node.get_event_status(ax) );
//    node.reset_event_status(ax,PMD::EV_STATUS_EVERYTHING);
//    if(pos)
//        node.set_bp_value(ax, 0, (1<<4)<<16 );
//    else
//        node.set_bp_value(ax, 0, (1<<5)<<16 );
//    node.set_bp(ax, 0, ax, PMD::TRIG_SIGNAL_STATUS, PMD::ACT_NONE);
//    node.clear_interrupt(ax);
//    PRINT("Event status: " << node.get_event_status(ax) );
//}

//void set_time_bp( PMD& node, int ax )
//{
//    node.set_interrupt_mask(ax,PMD::INT_BP1);
//    node.set_bp_value(ax,0,node.get_time()+3000);
//    node.set_bp(ax, 0, ax, PMD::TRIG_TIME, PMD::ACT_NONE);
//    uint16_t reg = node.get_event_status(ax);
//    node.reset_event_status(ax,~reg);
//    node.clear_interrupt(ax);
//}
struct PMDPositionMonitor : IOClient
{
    PMDPositionMonitor( IOService& io ): IOClient(io){}
    void on_msg_(const canmsg_t &m){
        if( m.data[0] == 0x99 ){
            std::cout << std::hex << std::showbase << (int)m.data[0]
                      << ' ' << (int)m.data[1]
                      << ' ' << (int)m.data[2]
                      << ' ' << (int)m.data[3]
                      << ' ' << (int)m.data[4] << std::endl;
        }
    }
};


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
    can_iface_sp_t iface = init_can_iface(plg_name, id, param);
    if(!iface){
        return EXIT_FAILURE;
    }
    //-----------------------
    // test
    //-----------------------
    try{
        IOService io(iface);
        PMDPositionMonitor cl(io);
        while(1){ SLEEP(10000); }

        /*int cycles_n = 1; //0000;
        if(argc>4){
            cycles_n  = atoi(argv[4]);
        }
        int node_id = 4;
        if(argc>5){
            node_id = atoi(argv[5]);
        }
        PMD node(node_id,io);
        EventsMonitor monitor(node_id,io);

        using namespace boost::posix_time;
        ptime start = microsec_clock::local_time();
        PRINT("*** TEST START ***");
        pmd_init(node);
        for(int i=0;i<4;++i){
            PRINT("init axis " << i);
            pmd_axis_init(node.axis(i), PMD::MOTOR_TYPE_DC );
        }
        TriggersWaiter waiter(io);
        PMDEventStatusTrigger trig0(node.id(),0);
        PMDEventStatusTrigger trig1(node.id(),1);
        PMDEventStatusTrigger trig2(node.id(),2);
        PMDEventStatusTrigger trig3(node.id(),3);
        waiter.add(trig0);
        waiter.add(trig1);
        waiter.add(trig2);
        waiter.add(trig3);
        for(size_t i=0;i<4;++i){
            SLEEP(2000);
            move(node.axis(i), node.axis(i).get_position()+100000);
        }
        waiter.wait_any(60000);
//        chip_init(node);
//        axis_init(node, ax);

//        TriggersWaiter waiter(io);
//        PMDEventStatusTrigger trig(node_id, ax);
//        while(cycles_n-->0){
//            waiter.add_trigger(&trig);
//            set_time_bp(node,ax);
//            if(!waiter.wait(10000)){
//                PRINT("Timeout");
//                break;
//            }
////            if(!node.wait_event(ax, &status, 10000)){
////                PRINT("Timeout");
////                break;
////            }
//            PRINT(hex<<showbase<<node.get_event_status(ax));
//        }
        PRINT("*** TEST STOP ***");
        ptime stop = microsec_clock::local_time();
        PRINT("Duration: " << stop - start);*/
    }
    catch(const PMDException& e){
        cout << "PMDException: " << e.what() << endl;
    }
    catch(const OperationTimeoutExcept& e ){
        cout << "NodeTimeoutException: " << e.what() << endl;
    }
    catch(const Exception& e){
        cout << "CANIO2 Exception: " << e.what() << endl;
    }

    return EXIT_SUCCESS;
}
