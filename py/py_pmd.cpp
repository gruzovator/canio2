/****************************************************************************
 *                                                                          *
 *                            CANIO2 Lib Project.                           *
 *                                                                          *
 ****************************************************************************/
/**
 @file
 ****************************************************************************
 @brief     Python Library (PMD part export)
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
// PMD Export Utils & Wrappers
//-----------------------------------------------------------------------------
// PMD Breakpoints
struct BreakpointSettings {
    BreakpointSettings(){}
    BreakpointSettings( int src, PMD::bp_trigger_t t, PMD::action_t a ):
        src_axis(src), trigger(t), action(a){}
    int src_axis;
    PMD::bp_trigger_t trigger;
    PMD::action_t action;
};

BreakpointSettings get_bp_settings( PMD::Axis& ax, PMD::bp_t bp ){
    BreakpointSettings s;
    ax.get_bp(bp,s.src_axis, s.trigger, s.action);
    return s;
}

void set_bp_settings( PMD::Axis& ax, PMD::bp_t bp, BreakpointSettings& s){
    ax.set_bp(bp, s.src_axis, s.trigger, s.action);
}

void pmd_ex_translator(const PMDException& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what().c_str());
}

//-----------------------------------------------------------------------------
// Exporting PMD lib part to python
//-----------------------------------------------------------------------------
void export_pmd()
{
    register_exception_translator<PMDException>(pmd_ex_translator);

    class_<PMD, boost::noncopyable> pmd("PMD",
                                        init<int, IOService&>()[with_custodian_and_ward<1,3>()]);
    scope PMD_SCOPE = pmd;
    class_<PMD::Axis, boost::noncopyable> axis("Axis", init<PMD&,int>());

    enum_<PMD::UPDATE_MASK_BITS>("UPDATE_MASK_BITS")
            .value("TRAJ", PMD::UPD_TRAJ)
            .value("POS_LOOP", PMD::UPD_POS_LOOP)
            .value("ALL", PMD::UPD_ALL)
            ;

    enum_<PMD::MOTOR_TYPES>("MOTOR_TYPES")
            .value("BLDC3", PMD::MOTOR_TYPE_BLDC_3)
            .value("DC", PMD::MOTOR_TYPE_DC)
            .export_values()
            ;
    enum_<PMD::OUTPUT_MODES>("OUTPUT_MODES")
            .value("PWM_50_50", PMD::OUT_MODE_PWM_50_50_MAGNITUDE)
            .value("SIGN_MAGNITUDE", PMD::OUT_MODE_PWM_SIGN_MAGNITUDE)
            .export_values()
            ;

    enum_<PMD::OPERATING_MODE_OPTIONS>("OPERATING_MODE_OPTIONS")
            .value("AXIS_ON", PMD::OP_MODE_AXIS_ON)
            .value("MOTOR_OUT_ON", PMD::OP_MODE_MOTOR_OUT_ON)
            .value("POS_LOOP_ON", PMD::OP_MODE_POS_LOOP_ON)
            .value("TRAJ_ON", PMD::OP_MODE_TRAJ_ON)
            .value("ALL", PMD::OP_MODE_ALL)
            .export_values()
            ;

    enum_<PMD::COMMUTATION_MODES>("COMMUTATION_MODES")
            .value("SIN", PMD::COMMUTATION_SIN)
            .value("HALL", PMD::COMMUTATION_HALL)
            .export_values()
            ;
    enum_<PMD::SENS_INPUTS>("SENS_INPUTS")
            .value("ENC_A", PMD::SENS_ENC_A)
            .value("ENC_B", PMD::SENS_ENC_B)
            .value("ENC_I", PMD::SENS_ENC_I)
            .value("CAPT", PMD::SENS_CAPT)
            .value("POSITIVE_LIMIT", PMD::SENS_POSITIVE_LIMIT)
            .value("NEGATIVE_LIMIT", PMD::SENS_NEGATIVE_LIMIT)
            .value("AXIS_IN", PMD::SENS_AXIS_IN)
            .value("HALL_A", PMD::SENS_HALL_A)
            .value("HALL_B", PMD::SENS_HALL_B)
            .value("HALL_C", PMD::SENS_HALL_C)
            .value("AXIS_OUT", PMD::SENS_AXIS_OUT)
            .value("STEP_OUT", PMD::SENS_STEP_OUT)
            .value("MOTOR_DIR", PMD::SENS_MOTOR_DIR)
            .export_values()
            ;
    enum_<PMD::CAPTURE_SOURCES>("CAPTURE_SOURCES")
            .value("INDEX", PMD::CAPT_SRC_INDEX)
            .value("HOME", PMD::CAPT_SRC_HOME)
            .value("HIGH_SPEED_CAPT", PMD::CAPT_SRC_HIGH_SPEED_CAPT)
            .export_values()
            ;
    enum_<PMD::ENCODER_SOURCES>("ENCODER_SOURCES")
            .value("INCR", PMD::ENC_SRC_INCREMENTAL)
            .value("PARALLEL", PMD::ENC_SRC_PARALLEL)
            .value("NONE", PMD::ENC_SRC_NONE)
            .value("LOOPBACK", PMD::ENC_SRC_LOOPBACK)
            .value("PULSE_DIR", PMD::ENC_SRC_PULSE_DIR)
            .export_values()
            ;
    enum_<PMD::PWM_FREQS>("PWM_FREQS")
            .value("_20KHZ", PMD::PWM_FREQ_20KHZ)
            .value("_80KHZ", PMD::PWM_FREQ_80KHZ)
            .export_values()
            ;
    enum_<PMD::EVENT_STATUS_BITS>("STATUS_BITS")
            .value("MOTION_COMPLETE", PMD::INT_MOTION_COMPLETE)
            .value("WRAP_ARROUND", PMD::INT_WRAP_ARROUND)
            .value("BP1", PMD::INT_BP1)
            .value("CAPTURE_RCV", PMD::INT_CAPTURE_RCV)
            .value("MOTION_ERR", PMD::INT_MOTION_ERR)
            .value("POSITIVE_LIMIT", PMD::INT_POSITIVE_LIMIT)
            .value("NEGATIVE_LIMIT", PMD::INT_NEGATIVE_LIMIT)
            .value("INSTRUCTION_ERR", PMD::INT_INSTRUCTION_ERR)
            .value("DISABLE", PMD::INT_DISABLE)
            .value("OVERTEMPERATURE", PMD::INT_OVERTEMPERATURE)
            .value("BUS_VOLTAGE", PMD::INT_BUS_VOLTAGE)
            .value("COMMUTATION_ERR", PMD::INT_COMMUTATION_ERR)
            .value("CURRENT_FOLDBACK", PMD::INT_CURRENT_FOLDBACK)
            .value("BP2", PMD::INT_BP2)
            .value("ALL", PMD::INT_ALL)
            .export_values()
            ;
    enum_<PMD::EVENTS>("EVENTS")
            .value("IMMEDIATE", PMD::EVENT_IMMEDIATE)
            .value("POSITIVE_LIMIT", PMD::EVENT_POSITIVE_LIMIT)
            .value("NEGATIVE_LIMIT", PMD::EVENT_NEGATIVE_LIMIT)
            .value("MOTION_ERR", PMD::EVENT_MOTION_ERR)
            .value("CURRENT_FOLDBACK", PMD::EVENT_CURRENT_FOLDBACK)
            .export_values()
            ;
    enum_<PMD::ACTIONS>("ACTIONS")
            .value("NONE", PMD::ACT_NONE)
            .value("UPDATE", PMD::ACT_UPDATE)
            .value("ABRUPT_STOP", PMD::ACT_ABRUPT_STOP)
            .value("SMOOTH_STOP", PMD::ACT_SMOOTH_STOP)
            .value("DISABLE_POS_LOOP", PMD::ACT_DISABLE_POS_LOOP)
            .value("DISABLE_CUR_LOOP", PMD::ACT_DISABLE_CUR_LOOP)
            .value("DISABLE_MOTOR_OUT", PMD::ACT_DISABLE_MOTOR_OUT)
            .value("ABRUPT_STOP_POS_ERR_CLR", PMD::ACT_ABRUPT_STOP_POS_ERR_CLR)
            .export_values()
            ;
    enum_<PMD::BREAKPOINTS>("BPS")
            .value("BP1", PMD::BP_1)
            .value("BP2", PMD::BP_2)
            .export_values()
            ;
    enum_<PMD::BREAKPOINT_TRIGGERS>("BP_TRIGGERS")
            .value("NONE", PMD::BP_TRIG_NONE)
            .value("GE_COMMAND_POS", PMD::BP_TRIG_GE_COMMAND_POS)
            .value("LE_COMMAND_POS", PMD::BP_TRIG_LE_COMMAND_POS)
            .value("GE_ACTUAL_POS", PMD::BP_TRIG_GE_ACTUAL_POS)
            .value("LE_ACTUAL_POS", PMD::BP_TRIG_LE_ACTUAL_POS)
            .value("COMMAND_POS_CROSSED", PMD::BP_TRIG_COMMAND_POS_CROSSED)
            .value("ACTUAL_POS_CROSSED", PMD::BP_TRIG_ACTUAL_POS_CROSSED)
            .value("TIME", PMD::BP_TRIG_TIME)
            .value("EVENT_STATUS", PMD::BP_TRIG_EVENT_STATUS)
            .value("ACTIVITY_STATUS", PMD::BP_TRIG_ACTIVITY_STATUS)
            .value("SIGNAL_STATUS", PMD::BP_TRIG_SIGNAL_STATUS)
            .value("DRIVE_STATUS", PMD::BP_TRIG_DRIVE_STATUS)
            .export_values()
            ;

    enum_<PMD::BP_UPDATE_MASK_BITS>("BP_UPDATE_MASK_BITS")
            .value("TRAJ", PMD::BP_UPD_TRAJ)
            .value("POS_LOOP", PMD::BP_UPD_POS_LOOP)
            .export_values()
            ;

    enum_<PMD::MOTION_COMPLETE_MODES>("MOTION_COMPLETE_MODES")
            .value("COMMANDED", PMD::MOTION_COMPL_COMMANDED)
            .value("ACTUAL", PMD::MOTION_COMPL_ACTUAL)
            .export_values()
            ;

    enum_<PMD::PROFILE_MODES>("PROFILE_MODES")
            .value("TRAPEZ", PMD::PROF_MODE_TRAPEZ)
            .value("VELOCITY", PMD::PROF_MODE_VELOCITY)
            .value("S_CURVE", PMD::PROF_MODE_S_CURVE)
            .value("E_GEAR", PMD::PROF_MODE_E_GEAR)
            .export_values()
            ;
    enum_<PMD::STOP_MODES>("STOP_MODES")
            .value("NO_STOP", PMD::STOP_MODE_NO_STOP)
            .value("ABRUPT", PMD::STOP_MODE_ABRUPT)
            .value("SMOOTH", PMD::STOP_MODE_SMOOTH)
            .export_values()
            ;

    class_<BreakpointSettings>("BPSettings", init<int, PMD::bp_trigger_t, PMD::action_t>())
            .def_readwrite("src_axis",&BreakpointSettings::src_axis)
            .def_readwrite("trigger",&BreakpointSettings::trigger)
            .def_readwrite("action",&BreakpointSettings::action)
            ;


#define PARAM_GETTER(param_value_type, name, param)\
    make_function( boost::bind(&PMD::Axis::name, _1, param),\
        default_call_policies(),boost::mpl::vector<param_value_type,PMD::Axis*>())

#define PARAM_SETTER(param_value_type, name, param)\
    make_function( boost::bind(&PMD::Axis::name, _1, param, _2),\
        default_call_policies(), boost::mpl::vector<void,PMD::Axis*,param_value_type>())
#define PARAM_GETSET(param_value_type, name, param)\
    PARAM_GETTER(param_value_type, get_##name, param),\
    PARAM_SETTER(param_value_type, set_##name, param)

    pmd.def("axis", &PMD::axis, return_internal_reference<1>())
            .add_property("id", &PMD::id)
            .add_property("operation_timeout", &PMD::get_operation_timeout, &PMD::set_operation_timeout)
            .def("disable_axes", &PMD::disable_axes)
            .def("reset", &PMD::reset)
            .add_property("time", &PMD::get_time)
            .add_property("sample_time", &PMD::get_sample_time, &PMD::set_sample_time)
            .add_property("interrupt_axes", &PMD::get_interrupt_axis )
            .def("multi_update", &PMD::multi_update)
            ;

    axis.add_property("number", &PMD::Axis::number)
            .add_property("number", &PMD::Axis::number)
            .def("enable", &PMD::Axis::enable)
            .def("disable", &PMD::Axis::disable)
            .def("update", &PMD::Axis::update)
            .add_property("update_mask", &PMD::Axis::get_update_mask, &PMD::Axis::set_update_mask)
            .add_property("motor_type", &PMD::Axis::get_motor_type, &PMD::Axis::set_motor_type)
            .add_property("output_mode", &PMD::Axis::get_output_mode, &PMD::Axis::set_output_mode)
            .add_property("operating_mode", &PMD::Axis::get_operating_mode, &PMD::Axis::set_operating_mode)
            .add_property("act_operating_mode", &PMD::Axis::get_active_operating_mode)
            .def("restore_operating_mode", &PMD::Axis::restore_operating_mode)
            .add_property("commutation_mode", &PMD::Axis::get_commutation_mode, &PMD::Axis::set_commutation_mode)
            .def("get_inversion", &PMD::Axis::get_inversion)
            .def("set_inversion", &PMD::Axis::set_inversion)
            .add_property("motor_cmd", &PMD::Axis::get_motor_command, &PMD::Axis::set_motor_command)
            .add_property("active_motor_cmd", &PMD::Axis::get_active_motor_command)
            .add_property("motor_limit", &PMD::Axis::get_motor_limit, &PMD::Axis::set_motor_limit)
            .add_property("capture_src", &PMD::Axis::get_capture_source, &PMD::Axis::set_capture_source)
            .add_property("enc_src", &PMD::Axis::get_encoder_source, &PMD::Axis::set_encoder_source)
            .add_property("pwm_freq", &PMD::Axis::get_pwm_freq, &PMD::Axis::set_pwm_freq)
            .add_property("interrupt_mask", &PMD::Axis::get_interrupt_mask, &PMD::Axis::set_interrupt_mask)
            .def("clear_interrupt",&PMD::Axis::clear_interrupt)
            .add_property("event_status", &PMD::Axis::get_event_status, &PMD::Axis::clear_event_status )
            .add_property("activity_status", &PMD::Axis::get_activity_status)
            .add_property("signal_status", &PMD::Axis::get_signal_status)
            .def("get_event_action", &PMD::Axis::get_event_action)
            .def("set_event_action", &PMD::Axis::set_event_action)
            .add_property("bp1_value", PARAM_GETSET(uint32_t, bp_value, PMD::BP_1))
            .add_property("bp2_value", PARAM_GETSET(uint32_t, bp_value, PMD::BP_2))
            .add_property("bp1_settings", make_function(
                     boost::bind(get_bp_settings, _1, PMD::BP_1),
                     default_call_policies(),
                     boost::mpl::vector<BreakpointSettings,PMD::Axis&>()),
                 make_function(boost::bind(set_bp_settings, _1, PMD::BP_1, _2),
                               default_call_policies(),
                               boost::mpl::vector<void,PMD::Axis&,BreakpointSettings>()))
            .add_property("bp2_settings",
                          make_function(
                              boost::bind(get_bp_settings, _1, PMD::BP_2),
                              default_call_policies(),
                              boost::mpl::vector<BreakpointSettings,PMD::Axis&>()),
                          make_function(
                              boost::bind(set_bp_settings, _1, PMD::BP_2, _2),
                              default_call_policies(),
                              boost::mpl::vector<void,PMD::Axis&,BreakpointSettings>()))
            .add_property("bp1_update_mask", PARAM_GETSET(uint32_t, bp_update_mask, PMD::BP_1))
            .add_property("bp2_update_mask", PARAM_GETSET(uint32_t, bp_update_mask, PMD::BP_2))
            .add_property("pl_prpotional_gain", PARAM_GETSET(uint32_t, position_loop, PMD::PL_PROPORTIONAL_GAIN))
            .add_property("pl_integrator_gain", PARAM_GETSET(uint32_t, position_loop, PMD::PL_INTEGRATOR_GAIN))
            .add_property("pl_integrator_limit", PARAM_GETSET(uint32_t, position_loop, PMD::PL_INTEGRATOR_LIMIT))
            .add_property("pl_derivative_gain", PARAM_GETSET(uint32_t, position_loop, PMD::PL_DERIVATIVE_GAIN))
            .add_property("pl_derivative_time", PARAM_GETSET(uint32_t, position_loop, PMD::PL_DERIVATIVE_TIME))
            .add_property("pl_output_gain", PARAM_GETSET(uint32_t, position_loop, PMD::PL_OUTPUT_GAIN))
            .add_property("pl_v_ff_gain", PARAM_GETSET(uint32_t, position_loop, PMD::PL_VELOCITY_FF_GAIN))
            .add_property("pl_a_ff_gain", PARAM_GETSET(uint32_t, position_loop, PMD::PL_ACCELERATION_FF_GAIN))
            .add_property("pl_bqf1_enable", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_ENABLE))
            .add_property("pl_bqf1_b0", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_B0))
            .add_property("pl_bqf1_b1", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_B1))
            .add_property("pl_bqf1_b2", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_B2))
            .add_property("pl_bqf1_a1", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_A1))
            .add_property("pl_bqf1_a2", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_A2))
            .add_property("pl_bqf1_k", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF1_K))
            .add_property("pl_bqf2_enable", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_ENABLE))
            .add_property("pl_bqf2_b0", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_B0))
            .add_property("pl_bqf2_b1", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_B1))
            .add_property("pl_bqf2_b2", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_B2))
            .add_property("pl_bqf2_a1", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_A1))
            .add_property("pl_bqf2_a2", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_A2))
            .add_property("pl_bqf2_k", PARAM_GETSET(uint32_t, position_loop, PMD::PL_BQF2_K))
            .add_property("motion_complete_mode",
                          &PMD::Axis::get_motion_compl_mode,
                          &PMD::Axis::set_motion_compl_mode)
            .add_property("profile_mode",
                          &PMD::Axis::get_profile_mode,
                          &PMD::Axis::set_profile_mode)
            .add_property("pos_err_limit",
                          &PMD::Axis::get_pos_error_limit,
                          &PMD::Axis::set_pos_error_limit)
            .add_property("position",
                          &PMD::Axis::get_position,
                          &PMD::Axis::set_position)
            .add_property("velocity",
                          &PMD::Axis::get_velocity,
                          &PMD::Axis::set_velocity)
            .add_property("actual_velocity",
                          &PMD::Axis::get_actual_velocity)
            .add_property("commanded_velocity",
                          &PMD::Axis::get_commanded_velocity)
            .add_property("acceleration",
                          &PMD::Axis::get_acceleration,
                          &PMD::Axis::set_acceleration)
            .add_property("deceleration",
                          &PMD::Axis::get_deceleration,
                          &PMD::Axis::set_deceleration)
            .add_property("actual_position",
                          &PMD::Axis::get_actual_position,
                          &PMD::Axis::set_actual_position)
            .add_property("commanded_position",
                          &PMD::Axis::get_commanded_position)
            .add_property("position_err",
                          &PMD::Axis::get_position_error)
            .def("clear_pos_error", &PMD::Axis::clear_position_error)
            .add_property("stop_mode", &PMD::Axis::get_stop_mode, &PMD::Axis::set_stop_mode)
            .add_property("event_trigger",
                          make_function(&PMD::Axis::event_trigger, return_internal_reference<1>()))
            ;
}

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
