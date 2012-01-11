import canio2
from canio2 import CANopenNode
import time 
import sys

#------------------------------------------------------------------------------
# consts
#------------------------------------------------------------------------------
NODE_ID = 1
ENCODER_LINES = 250
ENCODER_RESOLUTION = 4 * ENCODER_LINES
V = 1000 #rpm
REVOLUTIONS = 50
Imax = 4000  # ma

#------------------------------------------------------------------------------
#   Utilities
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

#------------------------------------------------------------------------------
# MAIN
#------------------------------------------------------------------------------
if __name__ == '__main__':
    iface = get_iface('ixxat', None, '1000K')
    io_service = canio2.IOService(iface)
    
    n = CANopenNode(NODE_ID, io_service) 
    waiter = canio2.TriggersWaiter(io_service)
    
    print('Resetting node...')
    if n.reset(6000) == False:
        print 'Boot Timeout'
        sys.exit(1)
    print('Node state after reset: ' + str(n.state))
    n.state = CANopenNode.STATES.OPERATIONAL
    time.sleep(0.1)
    if n.state == CANopenNode.STATES.OPERATIONAL:
        print('Node state : OPERATIONAL')
    else:
        raise Exception('Can\'t setup operational mode')
    try:
        # Initialisation
        n.wu32(0x3004,0)                       # disable power stage
        time.sleep(0.1)
        n.wu32(0x3900, 0x1) # motor type
        # controller feedback (encoder OR hall sensors)
        # Encoder feedback
        n.wu32(0x3910, 8)  # poles N
        n.wu32(0x3911, 9)                       
        n.wu32(0x3350, 0, 0x96A)                   # encoder feedback for the velocity controller
        n.wu32(0x3550, 0x9A6)                   # encoder feedback for the secondary velocity controller
        n.wu32(0x3962, ENCODER_RESOLUTION)       # encoder 
        #n.wu32(0x3963, ENCODER_LINES)       # encoder 
        # Controller parameters
        # Velocity controller
        n.wu32(0x3310, 100)                     # PID Vel Kp (proportional factor)
        n.wu32(0x3311, 0)                       # PID Vel Ki (integral factor)
        n.wu32(0x3312, 0)                       # PID Vel Kd (differential factor)
        n.wu32(0x3313, 0)                       # PID Vel KiLimit (integral limit)
        n.wu32(0x3314, 0)                    # PID Vel Kvff (velocity feed forward)
        
        # Secondary velocity controller
        n.wu32(0x3510, 100)                     # PI SVel Kp (proportional factor)
        n.wu32(0x3511, 100)                     # PI SVel Ki (integral factor)
        n.wu32(0x3517, 0)                       # SVel IxR factor
        
        # Current controller
        n.wu32(0x3210, 100)                     # PI Current controller Kp (proportional factor)
        n.wu32(0x3211, 100)                     # PI Current controller Ki (integral factor)
        
        # Current limits
        n.wu32(0x3221, Imax)                    # Current limit - max. positive in mA
        n.wu32(0x3223, Imax)                    # Current limit - max. negative in mA
        
        # Ramps
        # Acceleration = delta V / delta T
        n.wu32(0x3340, 3000)                    # VEL_Acc_dV (velocity in rpm)
        n.wu32(0x3341, 1000)                    # VEL_Acc_dT (time in ms)
                                                             # => acc = 3000 rpm / 1s
        # Deceleration = delta V / delta T
        n.wu32(0x3342, 3000)                    # Vel_Dec_dV (velocity in rpm)
        n.wu32(0x3343, 1000)                    # Vel_Dec_dt (time in ms)
                                                             # => dec = 3000 rpm / 1s
        # Position following error - window
        n.wu32(0x3732, 100000)                    # position following error - window
        
        # Device mode
        n.wu32(0x3003,7)                       # device mode "position mode"
        
        # TX PDO Setup
        # First TX-PDO (motor is sending)
        # Mapping table
        n.wu32(0x1A00, 0, 0)                       # delete mapping table first
        n.wu32(0x1A00, 1, 0x30020020)              # set first object (status register):
                                                                 # object 0x3002.0 (300200), length = 32bits (0x20h)
        n.wu32(0x1A00, 0, 1)                       # write numbers of objects into the table = 2
        # CobId
        n.wu32(0x1800, 1, 0x80000000)        # delete the old CobId first (Bit 31=1)
        n.wu32(0x1800, 1, 0x180 + NODE_ID)                 # set new CobId

        # Tansmissiontype
        n.wu32(0x1800, 2, 255)                     # asynchronous (see above)
        n.wu32(0x1800, 3, 200)                     # inhibit time in 100ys

        # Desired velocity
        n.ws32(0x3300,V)                    # desired velocity in rpm

        # Start of the test program
        n.wu32(0x3000,1)                       # reset error register
        n.wu32(0x3004,1)                       # enable power stage
        time.sleep(0.2)                         # 100ms delay for power stage to be enabled
        #n.wu32( 0x3150, 0, 1)                   # open brake (just in case)
        mc_trig = canio2.MsgDataTrigger(0x180+NODE_ID, 0x10, 0x10)
        dest = REVOLUTIONS*ENCODER_RESOLUTION
        while(1):
            dest = - dest
            print dest
            waiter.add(mc_trig)
            n.ws32(0x3791,dest)                 # relative positioning 4000 counts (hall or encoder)
            if waiter.wait(10000) == False:
                print('Motion Complete Timeout')
                n.wu32(0x3000,2)
                n.wu32(0x3004, 0)                       # disable power stage
            time.sleep(0.2)
            print 'Encoder position:', n.rs32(0x396A)
    except:
        print '\nError:', sys.exc_info()[0], sys.exc_info()[1]
    finally:
        print 'Stop drive.'
        n.wu32(0x3000,2)
        n.wu32(0x3004, 0)                       # disable power stage

