/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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

#ifndef MBED_BLE_LIGHT_SERVICE_H__
#define MBED_BLE_LIGHT_SERVICE_H__

#if BLE_FEATURE_GATT_SERVER

#include "platform/mbed_assert.h"
#include "ble/BLE.h"

#define UUID_LAMP_SERVICE "d9298dad-5480-4c92-a494-28a6cc02b786"
#define UUID_COLOR_CHARACTERISTICS "06adef88-c695-43ad-8c90-eeff63982939"
#define UUID_BRIGHTNESS_CHARACTERISTICS "346c99c9-f69f-48df-90d0-40d28de06407"
#define UUID_INPUT_TOGGLE_CHARACTERISTICS "d87cf5f5-096f-4a6e-a952-44a676ece268"
#define UUID_SENSOR_CHARACTERISTICS "6c00e224-f795-4b61-9c84-3b77d83ec099"


class LightService {
public:
    /**
     * Instantiate a battery service.
     *
     * The construction of a LightService adds a GATT battery service in @p
     * _ble GattServer and sets the initial charge level of the battery to @p
     * level.
     *
     * @param[in] _ble BLE device which will host the battery service.
     * @param[in] level Initial charge level of the battery. It is a percentage
     * where 0% means that the battery is fully discharged and 100% means that
     * the battery is fully charged.
     */
    LightService(BLE &_ble, uint8_t level = 100) :
        ble(_ble),
        brightnessLevel(level),
        // colorLevelCharacteristic(
        //     UUID_COLOR_CHARACTERISTICS,
        //     &batteryLevel,
        //     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        // ),
        // brightnessLevelCharacteristic(
        //     UUID_BRIGHTNESS_CHARACTERISTICS,
        //     &batteryLevel,
        //     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        // ),
        // inputToggleLevelCharacteristic(
        //     UUID_INPUT_TOGGLE_CHARACTERISTICS,
        //     &batteryLevel,
        //     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        // ),
        sensorCharacteristic(
            UUID_SENSOR_CHARACTERISTICS,
            &brightnessLevel,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        )
    {
        MBED_ASSERT(level <= 100);
        GattCharacteristic *charTable[] = { &sensorCharacteristic };
        GattService lightService(
            UUID_LAMP_SERVICE,
            charTable,
            sizeof(charTable) / sizeof(GattCharacteristic *)
        );

        ble.gattServer().addService(lightService);
    }

    /**
     * Update the battery charge level that the service exposes.
     *
     * The server sends a notification of the new value to clients that have
     * subscribed to the battery level characteristic updates, and clients
     * reading the charge level after the update obtain the updated value.
     *
     * @param newLevel Charge level of the battery. It is a percentage of the
     * remaining charge between 0% and 100%.
     *
     * @attention This function must be called in the execution context of the
     * BLE stack.
     */
    void updateBrightnessLevel(uint8_t newLevel)
    {
        MBED_ASSERT(newLevel <= 100);
        brightnessLevel = newLevel;
        ble.gattServer().write(
            sensorCharacteristic.getValueHandle(),
            &brightnessLevel,
            1
        );
    }

protected:
    /**
     * Reference to the underlying BLE instance that this object is attached to.
     *
     * The services and characteristics are registered in the GattServer of
     * this BLE instance.
     */
    BLE &ble;

    uint8_t brightnessLevel;

    //ReadOnlyGattCharacteristic<uint8_t> colorLevelCharacteristic;
    //ReadOnlyGattCharacteristic<uint8_t> brightnessLevelCharacteristic;
    //ReadOnlyGattCharacteristic<uint8_t> inputToggleLevelCharacteristic;
    ReadOnlyGattCharacteristic<uint8_t> sensorCharacteristic;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef MBED_BLE_LIGHT_SERVICE_H__*/