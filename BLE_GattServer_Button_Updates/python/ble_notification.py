from bluepy import btle

class MyDelegate(btle.DefaultDelegate):
    def __init__(self):
        btle.DefaultDelegate.__init__(self)
        # ... initialise here

    def handleNotification(self, cHandle, data):
        print('from handle:', cHandle)
        print('data received:', data)

# Initialisation  -------
addr = 'e9:64:4f:e1:21:11'
conn = btle.Peripheral(addr, btle.ADDR_TYPE_RANDOM)
conn.withDelegate( MyDelegate() )

services_dic = conn.getServices()
for service in services_dic:
    print(service.uuid)

charac_dic = service.getCharacteristics()

# Setup to turn notifications on, e.g.
ch = charac_dic[1]
cccd = ch.getHandle() + 1
conn.writeCharacteristic(cccd, b"\x01\x00")
# Main loop --------

while True:
    if conn.waitForNotifications(1.0):
        # handleNotification() was called
        continue

    print("Waiting...")
    # Perhaps do something else here