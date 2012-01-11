/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief
 ****************************************************************************
 @details
 ****************************************************************************
 @author    gruzovator 
 ****************************************************************************/
#ifndef PMD_NODE_HPP
#define PMD_NODE_HPP

#include "node.hpp"
#include "triggers.hpp"
#include <string.h>

CANIO_NS_START

//-----------------------------------------------------------------------------
struct PMDError : Error {
    enum {
        ERR_OK,
        ERR_RESET,
        ERR_INVALID_INSTRUCTION,
        ERR_INVALID_AXIS,
        ERR_INVALID_PARAM,
        ERR_TRACE_RUNNING
    };
    static std::string decode(int ev) {
        std::ostringstream os;
        os << "Unknown. Code: " << ev ;
        std::string res = os.str();
        switch(ev){
        case ERR_RESET:
            res = "Magellan reset.";
            break;
        case ERR_INVALID_INSTRUCTION:
            res = "Invalid instruction.";
            break;
        case ERR_INVALID_AXIS:
            res = "Invalid axis.";
            break;
        case ERR_INVALID_PARAM:
            res = "Invalid param.";
            break;
        case ERR_TRACE_RUNNING:
            res = "Trace running.";
            break;
        }
        return res;
    }

    PMDError(int ec):
        Error(std::string("PMD error: ")+PMDError::decode(ec)){}
};
typedef PMDError PMDException;

/** This node type is to work with PMD Magellan Motion Controller */
class PMD : public Node
{
public: // Constants, masks, type, etc
    
    enum UPDATE_MASK_BITS {
        UPD_TRAJ = 1<<0,
        UPD_POS_LOOP = 1<<1,
        UPD_ALL = (UPD_TRAJ | UPD_POS_LOOP )
    };

    typedef enum MOTOR_TYPES{
        MOTOR_TYPE_BLDC_3,
        MOTOR_TYPE_BLDC_2,
        MOTOR_TYPE_MICROSTEP_3,
        MOTOR_TYPE_MICROSTEP_2,
        MOTOR_TYPE_PULSE_DIR,
        MOTOR_TYPE_DC = 7
    } motor_type_t;

    typedef enum OUTPUT_MODES {
        OUT_MODE_PARAL_DAC_OFFSET_BIN,
        OUT_MODE_PWM_SIGN_MAGNITUDE,
        OUT_MODE_PWM_50_50_MAGNITUDE
    } out_mode_t;

    enum OPERATING_MODE_OPTIONS {
        OP_MODE_AXIS_ON = 1<<0,
        OP_MODE_MOTOR_OUT_ON = 1<<1,
        OP_MODE_POS_LOOP_ON = 1<<4,
        OP_MODE_TRAJ_ON = 1<<5,
        OP_MODE_ALL = (OP_MODE_AXIS_ON|OP_MODE_MOTOR_OUT_ON|
                OP_MODE_POS_LOOP_ON|OP_MODE_TRAJ_ON)
    };

    typedef enum COMMUTATION_MODES {
        COMMUTATION_SIN = 0,
        COMMUTATION_HALL = 1
    } commutation_mode_t;

    typedef enum SENS_INPUTS {
        SENS_ENC_A = 1<<0,
        SENS_ENC_B = 1<<1,
        SENS_ENC_I = 1<<2,
        SENS_CAPT = 1<<3,
        SENS_POSITIVE_LIMIT = 1<<4,
        SENS_NEGATIVE_LIMIT = 1<<5,
        SENS_AXIS_IN = 1<<6,
        SENS_HALL_A = 1<<7,
        SENS_HALL_B = 1<<8,
        SENS_HALL_C = 1<<9,
        SENS_AXIS_OUT = 1<<10,
        SENS_STEP_OUT = 1<<11,
        SENS_MOTOR_DIR = 1<<12
    } sens_t;

    typedef enum CAPTURE_SOURCES {
        CAPT_SRC_INDEX,
        CAPT_SRC_HOME,
        CAPT_SRC_HIGH_SPEED_CAPT
    } capt_src_t;

    typedef enum ENCODER_SOURCES{
        ENC_SRC_INCREMENTAL,
        ENC_SRC_PARALLEL,
        ENC_SRC_NONE,
        ENC_SRC_LOOPBACK,
        ENC_SRC_PULSE_DIR
    } enc_src_t;

    typedef enum PWM_FREQS {
        PWM_FREQ_UNKNOWN = 0,
        PWM_FREQ_20KHZ = 5000,
        PWM_FREQ_80KHZ = 20000
    } pwm_freq_t;

    enum EVENT_STATUS_BITS{ // event status register bits
        INT_MOTION_COMPLETE = 0x1,
        INT_WRAP_ARROUND = 0x2,
        INT_BP1 = 0x4,
        INT_CAPTURE_RCV = 0x8,
        INT_MOTION_ERR = 0x10,
        INT_POSITIVE_LIMIT = 0x20,
        INT_NEGATIVE_LIMIT = 0x40,
        INT_INSTRUCTION_ERR = 0x80,
        INT_DISABLE = 0x100,
        INT_OVERTEMPERATURE = 0x200,
        INT_BUS_VOLTAGE = 0x400,
        INT_COMMUTATION_ERR = 0x800,
        INT_CURRENT_FOLDBACK = 0x1000,
        INT_BP2 = 0x4000,
        INT_ALL = 0xFFFF
    };

    enum ACTIVITY_STATUS_BITS {
        ACTIVITY_POSITIVE_LIMIT = 1<<11,
        ACTIVITY_NEGATIVE_LIMIT = 1<<12
    };

    enum SIGNAL_STATUS_BITS {
        SIG_ST_BIT_ENC_A = 1<<0,
        SIG_ST_BIT_ENC_B = 1<<1,
        SIG_ST_BIT_ENC_INDX = 1<<2,
        SIG_ST_BIT_CAPT_IN = 1<<3,
        SIG_ST_BIT_POS_LIMIT = 1<<4,
        SIG_ST_BIT_NEG_LIMIT = 1<<5,
        SIG_ST_BIT_AXIS_IN = 1<<6,
        SIG_ST_BIT_AXIS_OUT = 1<<10
        // not everything
    };

    typedef enum EVENTS {
        EVENT_IMMEDIATE,
        EVENT_POSITIVE_LIMIT,
        EVENT_NEGATIVE_LIMIT,
        EVENT_MOTION_ERR,
        EVENT_CURRENT_FOLDBACK
    } event_t;

    typedef enum ACTIONS {
        ACT_NONE =0,
        ACT_UPDATE =1,
        ACT_ABRUPT_STOP = 2,
        ACT_SMOOTH_STOP = 3,
        ACT_DISABLE_POS_LOOP = 5,
        ACT_DISABLE_CUR_LOOP = 6,
        ACT_DISABLE_MOTOR_OUT = 7,
        ACT_ABRUPT_STOP_POS_ERR_CLR = 8
    } action_t;

    typedef enum BREAKPOINTS {
        BP_1 = 0,
        BP_2 = 1
    } bp_t;

    typedef enum BREAKPOINT_TRIGGERS{
        BP_TRIG_NONE,
        BP_TRIG_GE_COMMAND_POS,
        BP_TRIG_LE_COMMAND_POS,
        BP_TRIG_GE_ACTUAL_POS,
        BP_TRIG_LE_ACTUAL_POS,
        BP_TRIG_COMMAND_POS_CROSSED,
        BP_TRIG_ACTUAL_POS_CROSSED,
        BP_TRIG_TIME,
        BP_TRIG_EVENT_STATUS,
        BP_TRIG_ACTIVITY_STATUS,
        BP_TRIG_SIGNAL_STATUS,
        BP_TRIG_DRIVE_STATUS
    } bp_trigger_t;

    enum BP_UPDATE_MASK_BITS {
        BP_UPD_TRAJ = 1<<0,
        BP_UPD_POS_LOOP = 1<<1
    };

    typedef enum POS_LOOP_PARAMS {
        PL_PROPORTIONAL_GAIN,
        PL_INTEGRATOR_GAIN,
        PL_INTEGRATOR_LIMIT,
        PL_DERIVATIVE_GAIN,
        PL_DERIVATIVE_TIME,
        PL_OUTPUT_GAIN,
        PL_VELOCITY_FF_GAIN,
        PL_ACCELERATION_FF_GAIN,
        PL_BQF1_ENABLE,
        PL_BQF1_B0,
        PL_BQF1_B1,
        PL_BQF1_B2,
        PL_BQF1_A1,
        PL_BQF1_A2,
        PL_BQF1_K,
        PL_BQF2_ENABLE,
        PL_BQF2_B0,
        PL_BQF2_B1,
        PL_BQF2_B2,
        PL_BQF2_A1,
        PL_BQF2_A2,
        PL_BQF2_K
    } pl_param_t;

    typedef enum MOTION_COMPLETE_MODES {
        MOTION_COMPL_COMMANDED,
        MOTION_COMPL_ACTUAL
    } motion_compl_mode_t;

    typedef enum PROFILE_MODES {
        PROF_MODE_TRAPEZ,
        PROF_MODE_VELOCITY,
        PROF_MODE_S_CURVE,
        PROF_MODE_E_GEAR
    } profile_mode_t;

    typedef enum STOP_MODES {
        STOP_MODE_NO_STOP = 0,
        STOP_MODE_ABRUPT = 1,
        STOP_MODE_SMOOTH = 2
    } stop_mode_t;

public: 
    /** PMD Motion Controller Axis abstraction.
        See Magellan Motion Processor Programmers's Command Reference for
        constants and methods description.
        All constants and methods of that class are look (and behave) like origignal
        PMD APIs constants and functions.*/
    class Axis {
    private:
        PMD& pmd_;
        int ax_;

        class EventTrg : public Trigger
        {
        public:
            EventTrg( int node_id, int ax ):
                event_id_(0x180+node_id), axis_(ax){}
        private:
            uint32_t event_id_;
            int axis_;
            virtual bool activated_(const canmsg_t& in){
                return ((in.id == event_id_) && (in.data[1] == axis_) && (in.len == 4) );
            }
        };
        EventTrg event_trg_;

    public:
        Axis( PMD& pmd, int n ):pmd_(pmd), ax_(n), event_trg_(pmd.id(), n){
            if(ax_<0||ax_>=AXES_N) throw PMDException(PMDError::ERR_INVALID_AXIS);
        }

        int number() const { return ax_; }

        /** This trigger is used to wait PMD Events.
            PMD Events are CAN messages, that is sent by PMD when interupt occurs.
            See part 8.10 'Host Interrupts' of Magellan Motion Processor User's Guide.*/
        Trigger& event_trigger(){ return event_trg_; }

        void enable() { pmd_.enable_axis(ax_);}

        void disable() { pmd_.disable_axis(ax_);}

        void update() { pmd_.pmd_op(0x1A, ax_); }
        
        uint16_t get_update_mask(){
            uint16_t res;
            pmd_.pmd_op(0xFA, ax_, &res);
            return res;
        }

        void set_update_mask(uint16_t mask) {
            pmd_.pmd_op(0xF9, ax_, mask);
        }

        motor_type_t get_motor_type() {
            uint16_t t = 0;
            pmd_.pmd_op(0x03, ax_, &t);
            return (motor_type_t)t;
        }

        void set_motor_type(motor_type_t t){ pmd_.pmd_op(0x02, ax_,(uint16_t)t); }

        out_mode_t get_output_mode(){
            uint16_t m = 0;
            pmd_.pmd_op(0x6E, ax_, &m);
            return (out_mode_t)m;
        }

        void set_output_mode( out_mode_t mode ){ pmd_.pmd_op(0xE0, ax_,(uint16_t)mode); }

        uint16_t get_operating_mode(){
            uint16_t res;
            pmd_.pmd_op(0x66, ax_, &res);
            return res;
        }

        void set_operating_mode( uint16_t mode ){ pmd_.pmd_op(0x65, ax_, mode); }

        uint16_t get_active_operating_mode() {
            uint16_t res;
            pmd_.pmd_op(0x57, ax_, &res);
            return res;
        }
        void restore_operating_mode() { pmd_.pmd_op(0x2E, ax_); }

        commutation_mode_t get_commutation_mode() {
            uint16_t c;
            pmd_.pmd_op(0xE3, ax_, &c);
            return (commutation_mode_t)c;
        }

        void set_commutation_mode( commutation_mode_t c ) { pmd_.pmd_op(0xE2,ax_,(uint16_t)c); }

        bool get_inversion( sens_t type ){
            uint16_t res;
            pmd_.pmd_op(0xA3,ax_,&res);
            return (res & type) > 0;
        }

        void set_inversion( sens_t type, bool inv) {
            uint16_t all;
            pmd_.pmd_op(0xA3,ax_,&all);
            if(inv)
                all |= (uint16_t)type;
            else
                all &= ~((uint16_t)type);
            pmd_.pmd_op(0xA2,ax_,all);
        }

        float get_motor_command(){
            int16_t mc;
            pmd_.pmd_op(0x69, ax_, &mc);
            return mc*float(100.0)/float(32768.0);
        }

        float get_active_motor_command(){
            int16_t mc;
            pmd_.pmd_op(0x3A, ax_, &mc);
            return mc*float(100.0)/float(32768.0);
        }

        void set_motor_command( float v ){
            int16_t mc = (int16_t)floor(v*32768.0/100.0 + 0.5);
            pmd_.pmd_op(0x77, ax_, mc);
        }

        float get_motor_limit(){
            uint16_t mc;
            pmd_.pmd_op(0x07, ax_, &mc);
            return mc*float(100.0)/float(32768.0);
        }

        void set_motor_limit( float v ){
            uint16_t mc = (uint16_t)abs(floor(v*32768.0/100.0 + 0.5));
            pmd_.pmd_op(0x06, ax_, mc);
        }

        capt_src_t get_capture_source(){
            uint16_t s;
            pmd_.pmd_op(0xD9, ax_, &s);
            return (capt_src_t)s;
        }

        void set_capture_source( capt_src_t s ) { pmd_.pmd_op(0xD8, ax_,(uint16_t)s); }

        enc_src_t get_encoder_source() {
            uint16_t s;
            pmd_.pmd_op(0xDB, ax_, &s);
            return (enc_src_t)s;
        }

        void set_encoder_source( enc_src_t s ){ pmd_.pmd_op(0xDA, ax_,(uint16_t)s); }

        pwm_freq_t get_pwm_freq() {
            uint16_t v;
            pwm_freq_t f = PWM_FREQ_UNKNOWN;
            pmd_.pmd_op(0x0D, ax_, &v);
            if(v==5000) f = PWM_FREQ_20KHZ;
            else
            if(v==20000) f = PWM_FREQ_80KHZ;
            return f;
        }

        void set_pwm_freq( pwm_freq_t f ){
            uint16_t v=0;
            if(f==PWM_FREQ_20KHZ) v = 5000;
            else
            if(f==PWM_FREQ_80KHZ) v = 20000;
            pmd_.pmd_op(0x0C, ax_, v);
        }

        uint16_t get_interrupt_mask() {
            uint16_t mask;
            pmd_.pmd_op(0x56,ax_,&mask);
            return mask;
        }

        void set_interrupt_mask( uint16_t mask ){ pmd_.pmd_op(0x2F,ax_,mask); }

        void clear_interrupt(){ pmd_.pmd_op(0xAC,ax_); }

        uint16_t get_event_status() {
            uint16_t st;
            pmd_.pmd_op(0x31,ax_,&st);
            return st;
        }

        /// This function is more suitable replacement for original ResetEventStatus

        void clear_event_status( uint16_t bits_to_clear ){ //this is like reset_event_status
            pmd_.pmd_op(0x34, ax_, (uint16_t)(~bits_to_clear));
        }

        uint16_t get_activity_status(){
            uint16_t act;
            pmd_.pmd_op(0xA6, ax_, &act);
            return act;
        }

        uint16_t get_signal_status(){
            uint16_t s;
            pmd_.pmd_op(0xA4, ax_, &s);
            return s;
        }

        action_t get_event_action( event_t event ){
            uint16_t act;
            pmd_.pmd_op(0x49, ax_, (uint16_t)event, &act);
            return (action_t)act;
        }

        void set_event_action( event_t event, action_t action ){
            pmd_.pmd_op(0x48, ax_, (uint16_t)event, (uint16_t)action);
        }

        uint32_t get_bp_value( bp_t bp ){
            uint32_t v;
            pmd_.pmd_op(0xD7, ax_, (uint16_t)bp, &v);
            return v;
        }

        void set_bp_value( bp_t bp, uint32_t v ) { pmd_.pmd_op(0xD6, ax_, (uint16_t)bp, v); }

        void get_bp( bp_t bp, int& src_axis, bp_trigger_t& trig, action_t& a ) {
            uint16_t res;
            pmd_.pmd_op(0xD5, ax_, (uint16_t)bp, &res);
            src_axis = (res&0xF);
            a = (action_t)((res>>4)&0xF);
            trig = (bp_trigger_t)((res>>8)&0xFF);
        }

        void set_bp( bp_t bp, int src_axis,
                     bp_trigger_t trig, action_t a )
        {
            uint16_t v = (trig<<8) + (a<<4) + src_axis;
            pmd_.pmd_op(0xD4,ax_, (uint16_t)bp, v);
        }

        uint16_t get_bp_update_mask( bp_t bp ){
            uint16_t mask;
            pmd_.pmd_op(0x33, ax_, (uint16_t)bp, &mask);
            return mask;
        }

        void set_bp_update_mask( bp_t bp, uint16_t mask ){
            pmd_.pmd_op(0x32, ax_, (uint16_t)bp, mask);
        }

        int32_t get_position_loop( pl_param_t param ){
            int32_t v = 0;
            pmd_.pmd_op(0x68,ax_,(uint16_t)param, &v);
            return v;
        }

        void set_position_loop( pl_param_t param, int32_t v ){
            pmd_.pmd_op(0x67,ax_,(uint16_t)param, v);
        }

        uint32_t get_pos_error_limit(){
            uint32_t v = 0;
            pmd_.pmd_op(0x98,ax_,&v);
            return v;
        }

        void set_pos_error_limit( uint32_t limit ){ pmd_.pmd_op(0x97,ax_,limit); }

        motion_compl_mode_t get_motion_compl_mode(){
            uint16_t v = 0;
            pmd_.pmd_op(0xEC,ax_,&v);
            return (motion_compl_mode_t)v;
        }

        void set_motion_compl_mode( motion_compl_mode_t mode ){
            pmd_.pmd_op(0xEB,ax_,(uint16_t)mode);
        }

        profile_mode_t get_profile_mode(){
            uint16_t v = 0;
            pmd_.pmd_op(0xA1,ax_,&v);
            return (profile_mode_t)v;
        }

        void set_profile_mode( profile_mode_t pm ){
            pmd_.pmd_op(0xA0,ax_,(uint16_t)pm);
        }

        int32_t get_position(){
            int32_t p = 0;
            pmd_.pmd_op(0x4A,ax_,&p);
            return p;
        }

        void set_position( int32_t p ){
            pmd_.pmd_op(0x10,ax_,p);
        }

        int32_t get_commanded_position(){
            int32_t p = 0;
            pmd_.pmd_op(0x1D,ax_,&p);
            return p;
        }

        int32_t get_actual_position(){
            int32_t p = 0;
            pmd_.pmd_op(0x37,ax_,&p);
            return p;
        }

        void set_actual_position( int32_t p ){
            pmd_.pmd_op(0x4D,ax_,p);
        }

        int32_t get_velocity(){
            int32_t v = 0;
            pmd_.pmd_op(0x4B,ax_,&v);
            return v;
        }

        void set_velocity( int32_t v ){
            pmd_.pmd_op(0x11,ax_,v);
        }

        int32_t get_actual_velocity(){
            int32_t v = 0;
            pmd_.pmd_op(0xAD,ax_,&v);
            return v;
        }

        int32_t get_commanded_velocity(){
            int32_t v = 0;
            pmd_.pmd_op(0x1E,ax_,&v);
            return v;
        }

        uint32_t get_acceleration(){
            uint32_t a = 0;
            pmd_.pmd_op(0x4C,ax_,&a);
            return a;
        }

        void set_acceleration( uint32_t a ){
            pmd_.pmd_op(0x90,ax_,a);
        }

        uint32_t get_deceleration( ){
            uint32_t d = 0;
            pmd_.pmd_op(0x92,ax_,&d);
            return d;
        }

        void set_deceleration( uint32_t d ){
            pmd_.pmd_op(0x91,ax_,d);
        }

        void clear_position_error() {
            pmd_.pmd_op(0x47,ax_);
        }

        int32_t get_position_error(){
            int32_t p = 0;
            pmd_.pmd_op(0x99,ax_,&p);
            return p;
        }

        stop_mode_t get_stop_mode() {
            uint16_t sm;
            pmd_.pmd_op(0xD1, ax_, &sm);
            return (stop_mode_t)sm;
        }

        void set_stop_mode( stop_mode_t sm ){
            pmd_.pmd_op(0xD0,ax_,(uint16_t)sm);
        }
    };

public: //PMD iface

    Axis& axis(int ax){
        if(ax<0 || ax>=AXES_N) throw PMDException(PMDError::ERR_INVALID_AXIS);
        return *(axes_[ax]);
    }

    void write_io( uint16_t address, uint16_t val ){
        pmd_op(0x82,0,address,val);
    }
    uint16_t read_io( uint16_t address ){
        uint16_t v;
        pmd_op(0x83,0,address,&v);
        return v;
    }

    void disable_axis(int ax){
        axes_enable_mask_ &= ~(1<<ax);
        write_io(0,axes_enable_mask_);
    }

    void disable_axes(){
        axes_enable_mask_ = 0;
        write_io(0,0);
    }

    void enable_axis(int ax){
        axes_enable_mask_ |= (1<<ax);
        write_io(0,axes_enable_mask_);
    }

    void multi_update( uint16_t axes_mask ) {
        pmd_op(0x5B, 0, axes_mask);
    }

    void reset() {
        pmd_op(0x39, 0);
    }

    PMDError get_instruction_error() {
        uint16_t err;
        pmd_op(0xA5, 0, &err);
        return PMDError(err);
    }

    uint32_t get_sample_time() {
        uint32_t res;
        pmd_op(0x3C, 0, &res);
        return res;
    }

    void set_sample_time( uint32_t t ){
        pmd_op(0x3B, 0, t);
    }

    uint32_t get_time(){
        uint32_t res;
        pmd_op(0x3E,0,&res);
        return res;
    }

    int get_interrupt_axis(){ // returns bit field
        uint16_t ax;
        pmd_op(0xE1,0,&ax);
        return ax;
    }

    PMD(int id, IOService& io) : Node(id,io),
        axes_enable_mask_(0),
        reply_id_(0x580+id),
        is_wait_reply_(false)
    {
        axes_.push_back(boost::shared_ptr<Axis>(new Axis(*this, 0)));
        axes_.push_back(boost::shared_ptr<Axis>(new Axis(*this, 1)));
        axes_.push_back(boost::shared_ptr<Axis>(new Axis(*this, 2)));
        axes_.push_back(boost::shared_ptr<Axis>(new Axis(*this, 3)));
    }
private:
    static const int AXES_N = 4;
    int axes_enable_mask_;
    const uint32_t reply_id_;
    boost::mutex mtx_;
    boost::condition_variable rx_cond_var_;
    bool is_wait_reply_;
    canmsg_t tx_;
    canmsg_t rx_;
    std::vector< boost::shared_ptr<Axis> > axes_;

private:
    virtual void on_msg_( const canmsg_t & in ){
        lock_t lock(mtx_);
        if(is_wait_reply_ && in.id == reply_id_ ){
            rx_ = in;
            is_wait_reply_ = false;
            rx_cond_var_.notify_one();
        }
    }

    void do_transaction(){
        lock_t lock(mtx_);
        is_wait_reply_ = true;
        lock.unlock();
        my_io_service_.send(tx_);
        lock.lock();
        while(is_wait_reply_){
            if(!rx_cond_var_.timed_wait(lock, boost::posix_time::milliseconds(op_timeout_ms_))){
                throw OperationTimeoutExcept();
            }
        }
        if(rx_.data[1]){
            throw PMDException(rx_.data[1]);
        }
    }

    void build_tx_msg(int axis, int op_code, size_t data_sz){
        tx_.id = 0x600 + my_id_;
        tx_.rtr = 0;
        tx_.len = data_sz;
        tx_.data[0] = (uint8_t)axis;
        tx_.data[1] = (uint8_t)op_code;
    }

    template< typename T >
    T* change_order(T* v){
        if(sizeof(T) == 2 ){
            uint8_t* data = reinterpret_cast<uint8_t*>(v);
            uint8_t tmp = data[0];
            data[0] = data[1];
            data[1] = tmp;
        }
        else
        if( sizeof(T) == 4 ){
            uint8_t* data = reinterpret_cast<uint8_t*>(v);
            uint8_t tmp = data[0];
            data[0] = data[3];
            data[3] = tmp;
            tmp = data[1];
            data[1] = data[2];
            data[2] = tmp;
        }
        return v;
    }

    void pmd_op( int op_code, int axis ){
        build_tx_msg(axis, op_code, 2);
        do_transaction();
    }

    template<typename ArgT >
    void pmd_op( int op_code, int axis, ArgT a ){
        build_tx_msg(axis, op_code, 2+sizeof(ArgT));
        memcpy(tx_.data+2,change_order(&a), sizeof(ArgT));
        do_transaction();
    }

    template<typename ArgT >
    void pmd_op( int op_code, int axis, ArgT* a ){
        build_tx_msg(axis, op_code, 2);
        do_transaction();
        memcpy(reinterpret_cast<uint8_t*>(a), rx_.data+2, sizeof(ArgT));
        change_order(a);
    }

    template< typename Arg1T, typename Arg2T >
    void pmd_op( int op_code, int axis, Arg1T a1, Arg2T a2 ){
        build_tx_msg(axis, op_code, 2+sizeof(Arg1T) + sizeof(Arg2T));
        memcpy(tx_.data+2,change_order(&a1), sizeof(Arg1T));
        memcpy(tx_.data+2+sizeof(Arg1T),change_order(&a2), sizeof(Arg2T));
        do_transaction();
    }

    template<typename Arg1T, typename Arg2T >
    void pmd_op( int op_code, int axis, Arg1T a1, Arg2T* a2 ){
        build_tx_msg(axis, op_code, 2+sizeof(Arg1T));
        memcpy(tx_.data+2,change_order(&a1), sizeof(Arg1T));
        do_transaction();
        memcpy(reinterpret_cast<uint8_t*>(a2), rx_.data+2, sizeof(Arg2T));
        change_order(a2);
    }

    template<typename Arg1T, typename Arg2T,typename Arg3T>
    void pmd_op( int op_code, int axis, Arg1T a1, Arg2T a2, Arg3T a3 ){
        build_tx_msg(axis, op_code, 2+sizeof(Arg1T)+sizeof(Arg2T)+sizeof(Arg3T));
        memcpy(tx_.data+2,change_order(&a1), sizeof(Arg1T));
        memcpy(tx_.data+2+sizeof(Arg1T),change_order(&a2), sizeof(Arg2T));
        memcpy(tx_.data+2+sizeof(Arg1T) + sizeof(Arg2T),change_order(&a3), sizeof(Arg3T));
        do_transaction();
    }
};

CANIO_NS_END

#endif // PMD_NODE_HPP
