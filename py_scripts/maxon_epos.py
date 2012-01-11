import canio2
from canio2 import CANopenNode
import time 
import sys

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
    iface = get_iface('ixxat')
    io_service = canio2.IOService(iface)
    
    NODE_ID = 1
    node = CANopenNode(NODE_ID, io_service) 
    waiter = canio2.TriggersWaiter(io_service)
    
    print 'Resetting node...' 
    if node.reset(5000) == False:
        print 'Boot Timeout'
        sys.exit(1)
    print 'Node state after reset: ' + str(node.state)
    node.state = CANopenNode.STATES.OPERATIONAL
    print 'Node state : ' + str(node.state)

    print('* Drive Initialization.')
    print('\tClear Faults.')
    node.wu16(0x6040,0x80) 
    print('\tMotor parameters Setup.')
    node.wu16(0x6402,1) # motor type
    node.wu16(0x6410,1,7000) # cont. current limit, ma
    node.wu16(0x6410,2,8000) # out current limit, ma
    node.wu8(0x6410,3,1) # pole paires
    node.wu16(0x6410,4,3000) # max speed in current mode
    node.wu16(0x6410,5,400) # Thermal constant
    print('\tPosition Sensor Setup.')
    node.wu16(0x2210, 1, 500) # enc. pulse n
    node.wu16(0x2210, 2, 1) # pos. sensor type
    print('\tCurrent Regulator Gains Setup.')
    node.ws16(0x60F6, 1, 1250) # P
    node.ws16(0x60F6, 2, 300) # I
    print('\tSpeed Regulator Gains Setup.')
    node.ws16(0x60F9, 1, 1700) # P
    node.ws16(0x60F9, 2, 200) # I
    print('\tPosition Regulator Gains Setup.')
    node.ws16(0x60FB, 1, 350) # P
    node.ws16(0x60FB, 2, 10) # I
    node.ws16(0x60FB, 3, 400) # D
    print('\tClear Faults.')
    node.wu16(0x6040,0x80) 
    #---------------------------
    # TESTS
    #---------------------------
    if len(sys.argv) > 1 and sys.argv[1] == 'c': 
        print('* Current Mode Test.')
        try:
            for i in range(10):
                print('Start in {0} seconds'.format(10-i))
                time.sleep(1)
            I = 2000
            node.ws8(0x6060,-3) # mode of operation
            node.wu16(0x6040,0x6) # controlword (shutdown)
            node.wu16(0x6040,0xF) # controlword (switch on)
            node.ws16(0x2030,I)
            for i in range(500):
                time.sleep(0.2)
                print('Actual Current: {0} mA'.format(node.rs16(0x6078)))
        except KeyboardInterrupt:
            node.ws16(0x2030,0)
            
    else:
        print('* Velocity Mode Test.')
        V = 1000
        A = 500
        D = 500
        node.ws8(0x6060,3) # mode of operation
        node.wu32(0x607F,4000) # max profile v rpm
        node.wu32(0x6083,A) # profile acceleration
        node.wu32(0x6084,D) # profile deceleration
        node.wu32(0x6085,10000) # quick stop deceleration
        node.ws16(0x6086,0) # motion profile type, 0 - trapez.
        node.wu16(0x6040,0x6) # controlword (shutdown)
        node.wu16(0x6040,0xF) # controlword (switch on)
        for i in range(4):
            if i%2 == 0:
                node.ws32(0x60FF,V) # target velocity
            else:
                node.ws32(0x60FF,-V) # target velocity
            node.wu16(0x6040,0xF) # controlword (switch on)
            for i in range(20):
                time.sleep(0.2)
                print('Actual Velocity: {0}'.format(node.rs32(0x606C)))
            node.wu16(0x6040,0x10F) # stop
        node.wu16(0x6040,0x6) # stop
