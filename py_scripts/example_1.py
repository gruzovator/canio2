###############################################################################
# Example: 
# Send/Receive CAN message via IXXAT CAN interface
###############################################################################
import canio2

# CAN module (CAN iface type) initialization
module = canio2.make_module('ixxat') 
# CAN iface creation. Pass iface id and bitrate to make_iface method.
iface = module.make_iface('HW104122','1000K')
# CAN msg
m = canio2.CANMsg()
m.id = 0x1
m.rtr = 0
m.len = 2
m.data0 = 1
m.data1 = 2
# Send m, timeout - 100 ms
iface.send(m, 100)
# Recv CAN msg, timeout 3 s
if iface.recv(m, 3000) == True:
    print(m)
else:
    print('RX timeout')
