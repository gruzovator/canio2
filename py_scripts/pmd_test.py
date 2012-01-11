import canio2
from canio2 import PMD
import time
import sys

#------------------------------------------------------------------------------
# CONSTS
#------------------------------------------------------------------------------
COUNTS_PER_REVOLUTION = 1024
MOTOR_CMD_LIMIT = 80
TEST_SPEED_RPM = 1500
TEST_ACC = 15000
TEST_DEC = 15000

#------------------------------------------------------------------------------
# Globals
#------------------------------------------------------------------------------
PMDCycleTime = 154 #us

#------------------------------------------------------------------------------
# Conversions
#------------------------------------------------------------------------------
def r_to_c(pos):
    return pos * COUNTS_PER_REVOLUTION

def c_to_r(c):
    return float(c)/float(COUNTS_PER_REVOLUTION)

def rpm_to_cc(v):
    return int(round(v * (float(COUNTS_PER_REVOLUTION)*PMDCycleTime/(60.0 * 10**6)) * 2**16))

def cc_to_rpm(cc):
    return (cc * 60.0 * 10**6)/(COUNTS_PER_REVOLUTION * PMDCycleTime * 2**16)

def rpms_to_cc2(a):
    return int(round(a * (float(COUNTS_PER_REVOLUTION)*PMDCycleTime*PMDCycleTime/(60.0 * 10**12)) * 2**16))

#------------------------------------------------------------------------------
# DC control methods
#------------------------------------------------------------------------------
def init_axis(ampl_type, ax):
    ax.disable()
    ax.enable()
    # Motor type
    ax.motor_type = PMD.MOTOR_TYPES.DC
    ax.commutation_mode = PMD.COMMUTATION_MODES.HALL
    if ampl_type == 'nic':
        ax.set_inversion(PMD.SENS_INPUTS.HALL_A, True)
        ax.set_inversion(PMD.SENS_INPUTS.HALL_B, True)
        ax.set_inversion(PMD.SENS_INPUTS.HALL_C, True)
    else:
        ax.set_inversion(PMD.SENS_INPUTS.HALL_A, False)
        ax.set_inversion(PMD.SENS_INPUTS.HALL_B, False)
        ax.set_inversion(PMD.SENS_INPUTS.HALL_C, False)
    # Motor Output
    ax.output_mode = PMD.OUTPUT_MODES.SIGN_MAGNITUDE
    ax.pmw_freq = PMD.PWM_FREQS._20KHZ
    if ampl_type == 'nic':
        ax.set_inversion(PMD.SENS_INPUTS.MOTOR_DIR, True)
    else:
        ax.set_inversion(PMD.SENS_INPUTS.MOTOR_DIR, False)
    # Feedback
    ax.enc_src = PMD.ENCODER_SOURCES.INCR
    if ampl_type == 'nic':
        ax.set_inversion(PMD.SENS_INPUTS.ENC_A, True)
        ax.set_inversion(PMD.SENS_INPUTS.ENC_B, True)
        ax.set_inversion(PMD.SENS_INPUTS.ENC_I, True)
    else:
        ax.set_inversion(PMD.SENS_INPUTS.ENC_A, False)
        ax.set_inversion(PMD.SENS_INPUTS.ENC_B, False)
        ax.set_inversion(PMD.SENS_INPUTS.ENC_I, False)
    # Motor Control
    ax.motor_limit = MOTOR_CMD_LIMIT
    ax.motor_cmd = 0
    ax.update()
    ax.enable()
    ax.operating_mode = PMD.OPERATING_MODE_OPTIONS.AXIS_ON |PMD.OPERATING_MODE_OPTIONS.MOTOR_OUT_ON
    ax.interrupt_mask = 0
    # Position Loop
    if ampl_type == 'nic':
        ax.pl_prpotional_gain = 100
        ax.pl_integrator_gain = 15 #15
        ax.pl_integrator_limit = 10 #50
        ax.pl_derivative_gain = 0 #70
        ax.pl_derivative_time = 1 #40
        ax.pl_output_gain = 23248#22248
        ax.pl_v_ff_gain = 0
        ax.pl_a_ff_gain = 0
        ax.pl_bqf1_enable = 0
        ax.pl_bqf2_enable = 0
    else:
        ax.pl_prpotional_gain = 100
        ax.pl_integrator_gain = 10
        ax.pl_integrator_limit = 50
        ax.pl_derivative_gain = 0
        ax.pl_derivative_time = 1
        ax.pl_output_gain = 12248
        ax.pl_v_ff_gain = 0
        ax.pl_a_ff_gain = 0
        ax.pl_bqf1_enable = 0
        ax.pl_bqf2_enable = 0
    ax.acceleration = rpms_to_cc2(TEST_ACC)
    ax.deceleration = rpms_to_cc2(TEST_DEC)
    ax.pos_err_limit = 0xFFFF
    ax.profile_mode = PMD.PROFILE_MODES.TRAPEZ
    ax.motion_complete_mode = PMD.MOTION_COMPLETE_MODES.COMMANDED
    ax.operating_mode = PMD.OPERATING_MODE_OPTIONS.ALL
    ax.update()
    # Interrupts and common
    ax.interrupt_mask = PMD.STATUS_BITS.MOTION_COMPLETE | PMD.STATUS_BITS.MOTION_ERR
    ax.event_status = PMD.STATUS_BITS.ALL
    ax.clear_interrupt()

def move_to(ax, v, pos): # v - rpm, pos - revolutions
    ax.clear_pos_error();
    ax.actual_position = 0
    ax.velocity = rpm_to_cc(v)
    ax.position = r_to_c(pos)
    #print('MOVE TO: {0} (v: {1}, a: {2}, d: {3})'.
    #        format(ax.actual_position, ax.position, ax.velocity, ax.acceleration, ax.deceleration))
    ax.event_status = PMD.STATUS_BITS.ALL
    ax.restore_operating_mode()
    ax.update();
    ax.event_status = PMD.STATUS_BITS.MOTION_COMPLETE | PMD.STATUS_BITS.MOTION_ERR;
    ax.clear_interrupt()

def stop(ax):
    ax.set_event_action( PMD.EVENTS.IMMEDIATE, PMD.ACTIONS.ABRUPT_STOP )
    ax.operating_mode = PMD.OPERATING_MODE_OPTIONS.AXIS_ON |PMD.OPERATING_MODE_OPTIONS.MOTOR_OUT_ON

#------------------------------------------------------------------------------
# MAIN
#------------------------------------------------------------------------------
if __name__ == '__main__':
    ixxat = canio2.make_module('ixxat')
    iface = ixxat.make_iface('*', '1000K')
    io_service = canio2.IOService(iface)

    NODE_ID = 0
    AXIS_N = 0
    pmd = canio2.PMD(NODE_ID, io_service) 
    axis = pmd.axis(AXIS_N)
    waiter = canio2.TriggersWaiter(io_service)

    if len(sys.argv) == 1:
        print('Specify power stage type: nic or amc')
        sys.exit(2)
            
    init_axis(sys.argv[1], axis)
    destination = 140 # revolutions
    v = TEST_SPEED_RPM #rpm
    cycles = 100
    pos_errors = [0]
    try:
        for i in range(2*cycles):
            #print i
            destination = -destination
            waiter.reset()
            waiter.add(axis.event_trigger)
            move_to(axis, v, destination)
            for j in range(100):
                #print('a: {0:.2f}. c: {1:.2f}'.format(axis.actual_position, axis.commanded_position))
                #pos_errors.append(axis.position_err)
                print('{0}'.format(axis.position_err))
                #process_pos_errors(pos_errors)
                if axis.event_trigger.activated:
                    break
                time.sleep(0.1)
            if axis.event_trigger.activated == False:
                print axis.event_status
                print axis.actual_position
                print('Motion Timeout')
                stop(axis)
                exit(0)
        stop(axis)
    except KeyboardInterrupt:
        stop(axis)
    

    '''waiter.add(axis.event_trigger)
    move_to(axis,2200, 40)
    if waiter.wait(5000) == False:
        print 'Timeout'
    else:
        print 'OK'
    waiter.add(axis.event_trigger)
    move_to(axis,2200, -40)
    if waiter.wait(5000) == False:
        print 'Timeout'
    else:
        print 'OK'''

#------------------------------------------------------------------------------


