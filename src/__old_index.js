const raspi = require('raspi')
const pwm = require('raspi-pwm')
const noble = require('@abandonware/noble')
//const noble = require('noble')

const lampService = 'abcdefg'
const colorCharacteristics = 'bcdef'
const brightnessCharacteristics = 'jbafa'

const lampState = {
    color: 'red',
    brightness: 128
}

let redLed, greenLed

raspi.init(() => {
    redLed = new pwm.PWM('P1-12')
    greenLed = new pwm.PWM('P1-33')
    redLed.write(1)
    greenLed.write(0)
})

function updateLeds() {
    const brightness = lampState.brightness / 128.0
    if(lampState.color === 'red' || lampState.color === 'orange') {
        redLed.write(brightness)
    }
    if(lampState.color === 'green' || lampState.color === 'orange') {
        greenLed.write(brightness)
    }
}

noble.startScanning()

noble.on('discover', function (peripheral) {
    const services = peripheral.advertisement.serviceUuids
    if (services.indexOf(lampService) != -1) {
        peripheral.connect((err) => {
            if (err) throw err
            //const lampService = peripheral.services[0]
            const lampService = peripheral.services.find(service => service.uuid === lampService)
            lampService.discoverCharacteristics([colorCharacteristics, brightnessCharacteristics], (err, characteristics) => {
                if (err) throw err
                const characteristicHandlers = characteristics.map(characteristic => {
                    return new Promise((resolve, reject) => {
                        if (characteristic.uuid == brightnessCharacteristics) {
                            characteristic.read((err, data) => {
                                if (err) throw err
                                lampState.brightness = parseInt(data.toString("hex"), 16)
                            })
                        } else if (characteristic.uuid == colorCharacteristics) {
                            characteristic.read((err, data) => {
                                if (err) throw err
                                lampState.color = data.toString("ascii")
                            })
                        }
                    })

                })
                Promise.all(characteristicHandlers).then(() => {
                    updateLeds()
                })
            })
        })
    }
})