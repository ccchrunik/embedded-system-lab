from bluepy import btle
import time

addr = 'e9:64:4f:e1:21:11'
conn = btle.Peripheral(addr, btle.ADDR_TYPE_RANDOM)

services_dic = conn.getServices()
for service in services_dic:
    print(service.uuid)

charac_dic = service.getCharacteristics()
# for charac in charac_dic:
#    print(charac.uuid)
#    data = charac.read()
#    print('Get data:', data)

stu_id = charac_dic[0].read()
print('Student ID:', stu_id)

button = charac_dic[1]
button_state = button.read()
print('Button State:', button_state)

led = charac_dic[2]
led_state = led.read()

if led_state != b'\x00':
    led_state = b'\x01'

print('LED State:', led_state)

while True:

    # turn led off
    print()
    print('Turn LED off')
    led.write(b'\x00')
    led_state = led.read()

    if led_state != b'\x00':
        led_state = b'\x01'

    print('LED State:', led_state)
    time.sleep(0.1)

    # turn led on
    print()
    print('Turn LED on')
    led.write(b'\x01')
    led_state = led.read()

    if led_state != b'\x00':
        led_state = b'\x01'

    print('LED State:', led_state)
    time.sleep(0.1)
