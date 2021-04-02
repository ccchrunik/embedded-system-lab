/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GattCharacteristic.h"
#include "GattService.h"
#include "PinNames.h"
#include "PinNamesTypes.h"
#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "pretty_printer.h"
#include <cstdint>
#include <cstdio>
#include <mbed.h>
#include <functional>

static BufferedSerial serial_port(USBTX, USBRX);

FileHandle *mbed::mbed_override_console(int fd)
{
    return &serial_port; 
}

using mbed::callback;
using namespace std::literals::chrono_literals;


/**
 * A Clock service that demonstrate the GattServer features.
 *
 * The clock service host three characteristics that model the current hour,
 * minute and second of the clock. The value of the second characteristic is
 * incremented automatically by the system.
 *
 * A client can subscribe to updates of the clock characteristics and get
 * notified when one of the value is changed. Clients can also change value of
 * the second, minute and hour characteristric.
 */
class ButtonService : public ble::GattServer::EventHandler {
public:
    ButtonService() :
        _stu_id_char("12345678-bc75-4741-8a26-264af75807de", *STU_ID),
        _stu_id_service(
            /* uuid */ "A000",
            /* characteristics */ _stu_id_characteristics,
            /* numCharacteristics */ sizeof(_stu_id_characteristics) /
                                     sizeof(_stu_id_characteristics[0])
        ),
        _led1(LED1, 1),
        _button(USER_BUTTON, PullUp),
        _button_state("87654321-bc75-4741-8a26-264af75807de", false),
        _button_service(
            /* uuid */ "A001",
            /* characteristics */ _button_characteristics,
            /* numCharacteristics */ sizeof(_button_characteristics) /
                                     sizeof(_button_characteristics[0])
        ), 
        _led_state("55555555-bc75-4741-8a26-264af75807de", true),
        _led_service(
            /* uuid */ "A002",
            /* characteristics */ _led_characteristics,
            /* numCharacteristics */ sizeof(_led_characteristics) /
                                     sizeof(_led_characteristics[0])
        ),
        _general_service(
            /* uuid */ "A003",
            /* characteristics */ _general_characteristics,
            /* numCharacteristics */ 3
        )
    {
        /* update internal pointers (value, descriptors and characteristics array) */
        _stu_id_characteristics[0] = &_stu_id_char;
        _button_characteristics[0] = &_button_state;
        _led_characteristics[0] = &_led_state;

        // set up general characteristics
        _general_characteristics[0] = &_stu_id_char;
        _general_characteristics[1] = &_button_state;
        _general_characteristics[2] = &_led_state;
        /* setup authorization handlers */
        _led_state.setWriteAuthorizationCallback(this, &ButtonService::led_client_write);
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        _server = &ble.gattServer();
        _event_queue = &event_queue;
        ble_error_t err;

        printf("Registering demo service\r\n");
        // err = _server->addService(_button_service);
        // err = _server->addService(_led_service);
        // err = _server->addService(_stu_id_service);
        err = _server->addService(_general_service);

        if (err) {
            printf("Error %u during demo service registration.\r\n", err);
            return;
        }

        /* register handlers */
        _server->setEventHandler(this);

        printf("button service registered\r\n");
        _event_queue->call_every(1000ms, callback(this, &ButtonService::send_std_id));
        // _event_queue->call_every(500ms, this, &ButtonService::blink);
        _button.fall(Callback<void()>(this, &ButtonService::button_pressed));
        _button.rise(Callback<void()>(this, &ButtonService::button_released));
    }

    void updateButtonState(bool newState) {
        ble_error_t err = _button_state.set(*_server, newState);
        if (err) {
            printf("write of the second value returned error %u\r\n", err);
            return;
        }
    }

    void button_pressed(void) {
        // updateButtonState(true);
        _event_queue->call(this, &ButtonService::updateButtonState, true);
    }

    void button_released(void) {
        // updateButtonState(false);
        _event_queue->call(this, &ButtonService::updateButtonState, false);
    }

    void blink(void) {
        _led1 = !_led1;
    }

    void led_turn_on(void) {
        _led1 = 1;
    }

    void led_turn_off(void) {
        _led1 = 0;
    }

    // void send_led_state()

    /* GattServer::EventHandler */
private:
    /**
     * Handler called when a notification or an indication has been sent.
     */
    void onDataSent(const GattDataSentCallbackParams &params) override
    {
        printf("sent updates\r\n");
    }

    /**
     * Handler called after an attribute has been written.
     */
    void onDataWritten(const GattWriteCallbackParams &params) override
    {
        printf("data written:\r\n");
        printf("connection handle: %u\r\n", params.connHandle);
        printf("attribute handle: %u", params.handle);
        printf("\r\n");
    }

    /**
     * Handler called after an attribute has been read.
     */
    void onDataRead(const GattReadCallbackParams &params) override
    {
        printf("data read:\r\n");
        printf("connection handle: %u\r\n", params.connHandle);
        printf("attribute handle: %u", params.handle);
    }

    /**
     * Handler called after a client has subscribed to notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) override
    {
        printf("update enabled on handle %d\r\n", params.attHandle);
    }

    /**
     * Handler called after a client has cancelled his subscription from
     * notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) override
    {
        printf("update disabled on handle %d\r\n", params.attHandle);
    }

    /**
     * Handler called when an indication confirmation has been received.
     *
     * @param handle Handle of the characteristic value that has emitted the
     * indication.
     */
    void onConfirmationReceived(const GattConfirmationReceivedCallbackParams &params) override
    {
        printf("confirmation received on handle %d\r\n", params.attHandle);
    }

private:

    void send_std_id(void) {
        const static uint8_t stu_id[10] = "B07901184";
        ble_error_t err = _stu_id_char.set(*_server, stu_id);
        if (err) {
            printf("write of the second value returned error %u\r\n", err);
            return;
        }
    }

    void led_client_write(GattWriteAuthCallbackParams *e)
    {
        printf("characteristic %u write authorization\r\n", e->handle);

        if (e->offset != 0) {
            printf("Error invalid offset\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
            return;
        }

        if (e->len != 1) {
            printf("Error invalid len\r\n");
            e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
            return;
        }

        if (e->data[0] != 0) {
            led_turn_on();
        }
        else led_turn_off();

        e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
    


private:
    /**
     * Read, Write, Notify, Indicate  Characteristic declaration helper.
     *
     * @tparam T type of data held by the characteristic.
     */
    template<typename T>
    class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic {
    public:
        /**
         * Construct a characteristic that can be read or written and emit
         * notification or indication.
         *
         * @param[in] uuid The UUID of the characteristic.
         * @param[in] initial_value Initial value contained by the characteristic.
         */
        ReadWriteNotifyIndicateCharacteristic(const UUID & uuid, const T& initial_value) :
            GattCharacteristic(
                /* UUID */ uuid,
                /* Initial value */ &_value,
                /* Value size */ sizeof(_value),
                /* Value capacity */ sizeof(_value),
                /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
                /* Descriptors */ nullptr,
                /* Num descriptors */ 0,
                /* variable len */ false
            ),
            _value(initial_value) {
        }

        /**
         * Get the value of this characteristic.
         *
         * @param[in] server GattServer instance that contain the characteristic
         * value.
         * @param[in] dst Variable that will receive the characteristic value.
         *
         * @return BLE_ERROR_NONE in case of success or an appropriate error code.
         */
        ble_error_t get(GattServer &server, T& dst) const
        {
            uint16_t value_length = sizeof(dst);
            return server.read(getValueHandle(), &dst, &value_length);
        }

        /**
         * Assign a new value to this characteristic.
         *
         * @param[in] server GattServer instance that will receive the new value.
         * @param[in] value The new value to set.
         * @param[in] local_only Flag that determine if the change should be kept
         * locally or forwarded to subscribed clients.
         */
        ble_error_t set(GattServer &server, const uint8_t &value, bool local_only = false) const
        {
            return server.write(getValueHandle(), &value, sizeof(value), local_only);
        }

        uint8_t get_value(void) {
            return _value;
        }

    private:
        uint8_t _value;
    };

    /**
     * ReadOnly Characteristic declaration helper.
     *
     * @tparam T type of data held by the characteristic.
     */
    template<typename T>
    class ReadOnlySTUIDCharacteristic : public GattCharacteristic {
    public:
        /**
         * Construct a characteristic that can be read or written and emit
         * notification or indication.
         *
         * @param[in] uuid The UUID of the characteristic.
         * @param[in] initial_value Initial value contained by the characteristic.
         */
        ReadOnlySTUIDCharacteristic(const UUID & uuid, const T& initial_value) :
            GattCharacteristic(
                /* UUID */ uuid,
                /* Initial value */ &_value,
                /* Value size */ 10,
                /* Value capacity */ 10,
                /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                /* Descriptors */ nullptr,
                /* Num descriptors */ 0,
                /* variable len */ false
            ),
            _value(initial_value) {
        }

        /**
         * Get the value of this characteristic.
         *
         * @param[in] server GattServer instance that contain the characteristic
         * value.
         * @param[in] dst Variable that will receive the characteristic value.
         *
         * @return BLE_ERROR_NONE in case of success or an appropriate error code.
         */
        ble_error_t get(GattServer &server, T& dst) const
        {
            uint16_t value_length = sizeof(dst);
            return server.read(getValueHandle(), &dst, &value_length);
        }

        /**
         * Assign a new value to this characteristic.
         *
         * @param[in] server GattServer instance that will receive the new value.
         * @param[in] value The new value to set.
         * @param[in] local_only Flag that determine if the change should be kept
         * locally or forwarded to subscribed clients.
         */
        ble_error_t set(GattServer &server, const uint8_t* value, bool local_only = false) const
        {
            return server.write(getValueHandle(), value, 10, local_only);
        }

    private:
        uint8_t _value;
    };

private:
    GattServer *_server = nullptr;
    events::EventQueue *_event_queue = nullptr;

    // student id service and characteristic
    uint8_t STU_ID[10] = "B07901184";
    GattService _stu_id_service;
    GattCharacteristic* _stu_id_characteristics[1];

    ReadOnlySTUIDCharacteristic<uint8_t> _stu_id_char;

    // button service and characteristic
    InterruptIn _button;
    DigitalOut  _led1;
    GattService _button_service;
    GattCharacteristic* _button_characteristics[1];

    ReadWriteNotifyIndicateCharacteristic<bool> _button_state;

    // led service and characteristic
    GattService _led_service;
    GattCharacteristic* _led_characteristics[1];

    ReadWriteNotifyIndicateCharacteristic<bool> _led_state;

    // try to combine three charateristic into one service
    GattService _general_service;
    GattCharacteristic* _general_characteristics[3];

    

};

int main() {
    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;
    ButtonService demo_service;

    /* this process will handle basic ble setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo */
    ble_process.on_init(callback(&demo_service, &ButtonService::start));

    ble_process.start();

    return 0;
}
