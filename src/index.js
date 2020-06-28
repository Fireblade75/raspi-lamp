const raspi = require('raspi')
const pwm = require('raspi-pwm')
const { DigitalOutput } = require('raspi-gpio')
//const noble = require('noble')
const noble = require('@abandonware/noble')
const convertColor = require('./convertColor')
const logService = require('./logService')

const lampServiceUUID = 'd9298dad54804c92a49428a6cc02b786'
const colorCharacteristics = '06adef88c69543ad8c90eeff63982939'
const brightnessCharacteristics = '346c99c9f69f48df90d040d28de06407'
const inputToggleCharacteristics = 'd87cf5f5096f4a6ea95244a676ece268'
const sensorCharacteristics = '6c00e224f7954b619c843b77d83ec099'

const sensorTreshold = 70
const lampState = {
    color: 'red',
    brightness: 30,
    toggleOverride: false,
    sensorValue: 0
}

let brightnessPin, redLed, greenLed, blueLed

raspi.init(() => {
    brightnessPin = new pwm.PWM('GPIO13')
    redLed = new DigitalOutput('GPIO5')
    greenLed = new DigitalOutput('GPIO6')
    blueLed = new DigitalOutput('GPIO26')
    updateLeds()
})

function updateLeds() {
    // Set brightness
    let brightness = 0
    if(lampState.toggleOverride || lampState.sensorValue > sensorTreshold) {
        brightness = lampState.brightness / 128.0
    }
    brightnessPin.write(brightness)

    // Set color
    const color = convertColor(lampState.color)
    redLed.write(!color.red ? 1 : 0)
    greenLed.write(!color.green ? 1 : 0)
    blueLed.write(!color.blue ? 1 : 0)
}

noble.startScanning()

noble.on('discover', function (peripheral) {
    const services = peripheral.advertisement.serviceUuids
    logService(peripheral.address, services)
    if (services.indexOf(lampServiceUUID) != -1) {
        peripheral.connect((err) => {
            if (err) throw err
            noble.stopScanning()
            peripheral.discoverServices([lampServiceUUID], (err, lampServices) => {
                if (err) throw err
                if(lampServices.length === 0) {
                    console.log('Filed to find the Lamp Service')
                    peripheral.disconnect((err) => {
                        if (err) throw err
                    })
                    return
                }
                console.log('Connected to Lamp Service')
                const lampService = lampServices[0]
                lampService.discoverCharacteristics([colorCharacteristics, brightnessCharacteristics], (err, characteristics) => {
                    if (err) throw err
                    console.log(characteristics)
                    const characteristicHandlers = characteristics.map(characteristic => {
                        return new Promise((resolve, reject) => {
                            console.log('Characteristic UUID: ' + characteristic.uuid)
                            if (characteristic.uuid == brightnessCharacteristics) {
                                characteristic.read((err, data) => {
                                    if (err) throw err
                                    lampState.brightness = parseInt(data.toString("hex"), 16)
                                })
                            } else if (characteristic.uuid == colorCharacteristics) {
                                characteristic.read((err, data) => {
                                    if (err) throw err
                                    lampState.color = data.toString("ascii")
                                    console.log("Setting color to " + lampState.color)
                                })
                            } else if (characteristic.uuid == inputToggleCharacteristics) {
                                characteristic.read((err, data) => {
                                    if (err) throw err
                                    lampState.toggleOverride = data.toString("ascii") === 'true'
                                    console.log("Setting toggle_override to " + lampState.toggleOverride)
                                })
                            } else if (characteristic.uuid == sensorCharacteristics) {
                                characteristic.read((err, data) => {
                                    if (err) throw err
                                    lampState.sensorValue = parseInt(data.toString("hex"), 16)
                                    console.log("Setting sensor_value to " + lampState.sensorValue)
                                })
                            }
                            resolve()
                        })
                    })
                    Promise.all(characteristicHandlers).then(() => {
                        console.log("Updating LED")
                        updateLeds()

                        peripheral.disconnect((err) => {
                            if (err) throw err
                            console.log("Disconected from peripheral")
                            noble.startScanning()
                        })
                    })
                })
            })
        })
    }
    console.log('Discover ended')
})

