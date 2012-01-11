###############################################################################
# Example: 
# CANopen node reset
###############################################################################
import canio2
from canio2 import CANopenNode

# CAN module (CAN iface type) initialization
module = canio2.make_module('ixxat') 
# CAN iface creation. Pass iface id and bitrate to make_iface method.
iface = module.make_iface('HW104122','1000K')
# io_service object is a link between low and high levels
io_service = canio2.IOService(iface)

# CANopen Node object initialization
NODE_ID = 1
node = CANopenNode(NODE_ID, io_service) 
if node.reset(5000) == False:
    print 'Error: Boot Timeout'
else:
    print 'Boot OK'
print 'State:', node.state
