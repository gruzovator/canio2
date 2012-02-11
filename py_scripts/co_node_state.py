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
# init
#------------------------------------------------------------------------------
iface = get_iface('ixxat')
io_service = canio2.IOService(iface)

#------------------------------------------------------------------------------
# Main 
#------------------------------------------------------------------------------
NODE_ID = 1
node = CANopenNode(NODE_ID, io_service) 

print 'Node state:',node.state

print 'Setting state to:', CANopenNode.STATES.STOPPED
node.state = CANopenNode.STATES.STOPPED
print 'Node state: ', node.state

print 'Setting state to:', CANopenNode.STATES.PRE_OPERATIONAL
node.state = CANopenNode.STATES.PRE_OPERATIONAL
print 'Node state:', node.state

print 'Setting state to:' ,CANopenNode.STATES.OPERATIONAL

node.state = CANopenNode.STATES.OPERATIONAL
print 'Node state: ',node.state

for i in range(10):
    print 'Resetting node...' 
    if node.reset(5000) == False:
        print 'Error: Boot Timeout'
    else:
        print 'Boot OK'
    print 'Node state after reset:', node.state
#------------------------------------------------------------------------------


