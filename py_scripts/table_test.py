import time
import sys
from optparse import OptionParser
import canio2
from canio2 import PMD

#------------------------------------------------------------------------------
# COMMON CONSTS
#------------------------------------------------------------------------------
NODE_ID = 11
AXES_IDS = [2,3]
COUNTS_PER_REVOLUTION = 1024
COUNTS_PER_MM = 91
POS_ERR_LIMIT_MM = 3
MOTOR_CMD_LIMIT = 80
ACC = 2800
DEC = 2800
#------------------------------------------------------------------------------
# HOMING CONSTS
#------------------------------------------------------------------------------
HOMING_INIT_UP_MOVE_MM = 20
SAFE_SPEED_RPM = 500
#------------------------------------------------------------------------------
# TEST CONSTS
#------------------------------------------------------------------------------
TEST_V = 3200
#------------------------------------------------------------------------------
# Globals
#------------------------------------------------------------------------------
PMDCycleTime = 154 #us
LogFile = 'table_test_log.txt'

#------------------------------------------------------------------------------
# Conversions
#------------------------------------------------------------------------------
def r_to_c(pos):
    return pos * COUNTS_PER_REVOLUTION

def c_to_r(c):
    return float(c)/float(COUNTS_PER_REVOLUTION)

def mm_to_c(mm):
    return mm*COUNTS_PER_MM

def rpm_to_cc(v):
    print 'PMDCycleTime', PMDCycleTime
    return int(round(v * (float(COUNTS_PER_REVOLUTION)*PMDCycleTime/(60.0 * 10**6)) * 2**16))

def rpm_to_mms(v):
    return int(round(v*float(COUNTS_PER_REVOLUTION)/float(COUNTS_PER_MM*60)))

def cc_to_rpm(cc):
    return (cc * 60.0 * 10**6)/(COUNTS_PER_REVOLUTION * PMDCycleTime * 2**16)

def rpms_to_cc2(a):
    return int(round(a * (float(COUNTS_PER_REVOLUTION)*PMDCycleTime*PMDCycleTime/(60.0 * 10**12)) * 2**16))
    
#------------------------------------------------------------------------------
#   PMD Utilities
#------------------------------------------------------------------------------
def init_axes(ampl_type, axes):
    for ax in axes:
        ax.disable()
        ax.enable()
        # Motor type
        ax.motor_type = PMD.MOTOR_TYPES.DC
        ax.commutation_mode = PMD.COMMUTATION_MODES.HALL
        if ampl_type == 'nic':
            ax.set_inversion(PMD.SENS_INPUTS.HALL_A, True)
            ax.set_inversion(PMD.SENS_INPUTS.HALL_B, True)
            ax.set_inversion(PMD.SENS_INPUTS.HALL_C, True)
            ax.set_inversion(PMD.SENS_INPUTS.NEGATIVE_LIMIT, True)
            ax.set_inversion(PMD.SENS_INPUTS.POSITIVE_LIMIT, True)
        else:
            ax.set_inversion(PMD.SENS_INPUTS.HALL_A, False)
            ax.set_inversion(PMD.SENS_INPUTS.HALL_B, False)
            ax.set_inversion(PMD.SENS_INPUTS.HALL_C, False)
        # Motor Output
        ax.output_mode = PMD.OUTPUT_MODES.SIGN_MAGNITUDE
        ax.pmw_freq = PMD.PWM_FREQS._20KHZ
        if ampl_type == 'nic':
            ax.set_inversion(PMD.SENS_INPUTS.MOTOR_DIR, False)
        else:
            ax.set_inversion(PMD.SENS_INPUTS.MOTOR_DIR, False)
        # Feedback
        ax.enc_src = PMD.ENCODER_SOURCES.INCR
        if ampl_type == 'nic':
            ax.set_inversion(PMD.SENS_INPUTS.ENC_A, False)
            ax.set_inversion(PMD.SENS_INPUTS.ENC_B, False)
            ax.set_inversion(PMD.SENS_INPUTS.ENC_I, False)
        else:
            ax.set_inversion(PMD.SENS_INPUTS.ENC_A, False)
            ax.set_inversion(PMD.SENS_INPUTS.ENC_B, False)
            ax.set_inversion(PMD.SENS_INPUTS.ENC_I, False)
        # Motor Control
        ax.motor_limit = MOTOR_CMD_LIMIT
        ax.motor_cmd = 0
        ax.update_mask = PMD.UPDATE_MASK_BITS.ALL
        ax.update()
        ax.enable()
        ax.operating_mode = PMD.OPERATING_MODE_OPTIONS.AXIS_ON | PMD.OPERATING_MODE_OPTIONS.MOTOR_OUT_ON
        ax.interrupt_mask = 0
        # Position Loop
        if ampl_type == 'nic':
            ax.pl_prpotional_gain = 400
            ax.pl_integrator_gain = 1
            ax.pl_integrator_limit = 1
            ax.pl_derivative_gain = 130
            ax.pl_derivative_time = 1
            ax.pl_output_gain = 23000
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
        
        ax.set_event_action(PMD.EVENTS.NEGATIVE_LIMIT, PMD.ACTIONS.ABRUPT_STOP_POS_ERR_CLR)
        ax.set_event_action(PMD.EVENTS.POSITIVE_LIMIT, PMD.ACTIONS.SMOOTH_STOP)
        ax.acceleration = rpms_to_cc2(ACC)
        ax.deceleration = rpms_to_cc2(DEC)
        ax.pos_err_limit = mm_to_c(POS_ERR_LIMIT_MM)
        ax.profile_mode = PMD.PROFILE_MODES.TRAPEZ
        ax.motion_complete_mode = PMD.MOTION_COMPLETE_MODES.COMMANDED
        ax.operating_mode = PMD.OPERATING_MODE_OPTIONS.ALL
        ax.update()
        # Interrupts and common
        ax.interrupt_mask = PMD.STATUS_BITS.MOTION_COMPLETE | PMD.STATUS_BITS.MOTION_ERR
        ax.event_status = PMD.STATUS_BITS.ALL
        ax.clear_interrupt()

def stop_axes(pmd, axes):
    axes_mask = 0
    for ax in axes:
        ax.stop_mode = PMD.STOP_MODES.SMOOTH
        ax.update_mask = PMD.UPDATE_MASK_BITS.TRAJ
        axes_mask += (1<< ax.number)
    pmd.multi_update(axes_mask)    

def disable_axes(pmd, axes):
    stop_axes(pmd, axes) 
    for ax in axes:
        ax.set_event_action(PMD.EVENTS.IMMEDIATE, PMD.ACTIONS.DISABLE_MOTOR_OUT)
       
def move_abs(pmd, axes, v_rpm, pos_mm):
    axes_mask = 0
    for ax in axes:
        ax.event_status = PMD.STATUS_BITS.ALL
        ax.restore_operating_mode()
        ax.clear_pos_error();
        ax.velocity = rpm_to_cc(v_rpm)
        ax.position = mm_to_c(pos_mm)
        ax.event_status = PMD.STATUS_BITS.MOTION_COMPLETE | PMD.STATUS_BITS.MOTION_ERR;
        ax.clear_interrupt()
        ax.update_mask = PMD.UPDATE_MASK_BITS.ALL
        axes_mask += (1<< ax.number)
    pmd.multi_update(axes_mask)    

def move_rel(pmd, axes, v_rpm, pos_mm):
    axes_mask = 0
    for ax in axes:
        ax.event_status = PMD.STATUS_BITS.ALL
        ax.restore_operating_mode()
        ax.clear_pos_error();
        ax.velocity = rpm_to_cc(v_rpm)
        ax.position = ax.actual_position + mm_to_c(pos_mm)
        ax.event_status = PMD.STATUS_BITS.MOTION_COMPLETE | PMD.STATUS_BITS.MOTION_ERR;
        ax.clear_interrupt()
        ax.update_mask = PMD.UPDATE_MASK_BITS.ALL
        axes_mask += (1<< ax.number)
    pmd.multi_update(axes_mask)    

def unlock_axes(axes):
    for ax in axes:
        ax.set_inversion(PMD.SENS_INPUTS.AXIS_OUT, True)
    print '\tUnlocking Axes'
    time.sleep(1)

def lock_axes(axes):            
    for ax in axes:
        ax.set_inversion(PMD.SENS_INPUTS.AXIS_OUT, False)
    print '\tLocking Axes'
    time.sleep(0.5)

#------------------------------------------------------------------------------
# Exceptions
#------------------------------------------------------------------------------
class TimeoutEx(Exception):
    pass
class MotionErr(Exception):
    pass
class NoLimitErr(Exception):
    pass

#------------------------------------------------------------------------------
# Utils
#------------------------------------------------------------------------------
def get_iface(module_name, instance_name = None, param = None):
    module = canio2.make_module(module_name)
    ids = module.ids()
    if param ==None : param = '1000K'
    if instance_name: 
        return module.make_iface(instance_name, param)
    if len(ids) == 0: # no ifaces
        return None
    if len(ids) == 1: # there is only one iface
        return module.make_iface(ids[0],param)
    # otherwise
    print('Available IDs:')
    n = 0
    for i in ids: 
        n+=1
        print('[{0}] {1}'.format(n, i))
    n = input('Select iface (1-{0}):'.format(len(ids)))
    return module.make_iface(ids[n-1],param)

def wait(waiter, delay_seconds):
    i = delay_seconds*10
    while i>0:
        if waiter.wait(100) == True:
            break
        i-=1
    if i==0:
        raise TimeoutEx()

def wait_ex(waiter, delay_seconds, axes):
    i = delay_seconds*10
    f = open(LogFile, 'a')
    while i>0:
        for ax in axes:
            print >> f, ax.position_err,
        print >> f
        if waiter.wait(100) == True:
            break
        i-=1
    if i==0:
        raise TimeoutEx()

#------------------------------------------------------------------------------
# Main
#------------------------------------------------------------------------------
def main():
    parser = OptionParser()
    parser.add_option('-m', '--mode', action='store', dest='mode', type='choice', 
            choices=['homing', 'test', 'srv'], default='homing',
            help='Modes: homing = do homing, test = do test; service = move one axis' )
    parser.add_option('-d', action='store', type='int', dest='distance', default = 0,
            help='This is a relative distance in mm for service mode and amplitude for test mode.' )
    parser.add_option('-a', action='store', type='int', dest='service_axis', default=-1,
            help='This is a target axis [0-3] for service mode. Both table axes will be used if this option is not supplied.' )
    parser.add_option('-n', action='store', type='int', dest='test_cycles', default=1,
            help='This is a number of test iterations for test mode.' )
    #parser.add_option('-l', action='store', type='string', dest='log', default=LogFile,
    #        help='Log file.' )
    (options, args) = parser.parse_args()
    #LogFile = options.log

    iface = get_iface('ixxat')
    io_service = canio2.IOService(iface)
    pmd = canio2.PMD(NODE_ID, io_service) 
    waiter = canio2.TriggersWaiter(io_service)
    axes = []
    print '*** Axes Initialization ***'
    if options.mode == 'srv' and options.service_axis >=0 and options.service_axis<=3:
        axes = [pmd.axis(options.service_axis),]
    else:
        for n in AXES_IDS:
            ax = pmd.axis(n)
            axes.append(ax)
    init_axes('nic',axes)
    global PMDCycleTime
    PMDCycleTime = pmd.sample_time
    print 'PMDCycleTime', PMDCycleTime
    print '*** Axes Initialization Done ***'

    try:
        if options.mode == 'homing' or options.mode == 'test':
            print '*** HOMING ***'
            stop_axes(pmd,axes)
            unlock_axes(axes)
            print '\tHoming. Step1. Moving Up...'
            for ax in axes:
                waiter.add(ax.event_trigger)
            move_rel(pmd, axes, SAFE_SPEED_RPM, HOMING_INIT_UP_MOVE_MM)
            wait(waiter, 4) #TODO wait time calc
            for ax in axes:
                st = ax.event_status
                print '\tAxis', ax.number, 'status reg:', bin(st)
                if (st & PMD.STATUS_BITS.MOTION_ERR) > 0:
                    raise MotionErr()
            print '\tHoming. Step1. Completed.'
            time.sleep(1)
            print '\tHoming. Step2. Moving Down to Negative Limit...'
            for ax in axes:
                waiter.add(ax.event_trigger)
            move_rel(pmd, axes, SAFE_SPEED_RPM, -1000) # -1000 mm
            wait(waiter, 2)
            for ax in axes:
                st = ax.event_status
                print '\tAxis', ax.number, 'status reg:', bin(st)
                if (st & PMD.STATUS_BITS.MOTION_ERR) > 0:
                    raise MotionErr()
                if (st & PMD.STATUS_BITS.NEGATIVE_LIMIT) == 0:
                    raise NoLimitErr()
                ax.actual_position = 0
            print '\tHoming. Step2. Completed.'

        elif options.mode == 'srv':
            print '*** SERVICE ***'
            print '\tMoving', options.distance, 'mm from current position'
            stop_axes(pmd,axes)
            unlock_axes(axes)
            for ax in axes:
                waiter.add(ax.event_trigger)
            v = SAFE_SPEED_RPM 
            move_rel(pmd, axes, v, options.distance)
            wait(waiter, 1+abs(options.distance)/rpm_to_mms(v))
            # show status reg at the end
            for ax in axes:
                st = ax.event_status
                print '\tAxis', ax.number, 'status reg:', bin(st)
                if (st & PMD.STATUS_BITS.MOTION_ERR) > 0:
                    raise MotionErr()
            print '\tMoving done.'

        if options.mode =='test':
            time.sleep(1) # do delay after homing
            print '*** TABLE TEST (', options.test_cycles, 'times ) ***'
            v = TEST_V
            down = 20
            up = options.distance + 20
            waiter.reset();
            t0 = time.clock()
            for i in range(options.test_cycles):
                print '\tITERATION', i+1
                print '\tMoving UP...'
                for ax in axes:
                    waiter.add(ax.event_trigger)
                move_abs(pmd, axes, v, up)
                wait_ex(waiter, 10+abs(options.distance)/rpm_to_mms(v),axes) #TODO wait time
                # show status reg at the end
                for ax in axes:
                    st = ax.event_status
                    print '\tAxis', ax.number, 'status reg:', bin(st)
                    if (st & PMD.STATUS_BITS.MOTION_ERR) > 0:
                        raise MotionErr()
                print '\tMoving UP Done.'                
                print '\tMoving DOWN...'
                for ax in axes:
                    waiter.add(ax.event_trigger)
                move_abs(pmd, axes, v, down)
                wait_ex(waiter, 10+abs(options.distance)/rpm_to_mms(v), axes) #TODO wait time
                # show status reg at the end
                for ax in axes:
                    st = ax.event_status
                    print '\tAxis', ax.number, 'status reg:', bin(st)
                    if (st & PMD.STATUS_BITS.MOTION_ERR) > 0:
                        raise MotionErr()
                print '\tMoving DOWN Done.'                
            t1 = time.clock()
            print 'Test duration', t1-t0

    except TimeoutEx:
        print 'Error: Timeout'
    except MotionErr:
        print 'Error: Motion Error'
    except NoLimitErr:
        print 'Error: Can\'t find limit'
    except KeyboardInterrupt:
        print 'Keyboard Interrupt'
    finally:
        print '\tFinally...'
        stop_axes(pmd, axes)
        time.sleep(1)
        lock_axes(axes)
        disable_axes(pmd, axes)
        time.sleep(0.1) #to handle unreceived CAN messages, cause there is a problem...
    
if __name__ == '__main__':
    main()
    
