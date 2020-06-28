
//const noble = require('@abandonware/noble')
const noble = require('noble')
const i2c = require('i2c-bus')
const convertColor = require('./convertColor')

const lampService = 'd9298dad-5480-4c92-a494-28a6cc02b786'
const colorCharacteristics = '06adef88-c695-43ad-8c90-eeff63982939'
const brightnessCharacteristics = '346c99c9-f69f-48df-90d0-40d28de06407'
const inputToggleCharacteristics = 'd87cf5f5-096f-4a6e-a952-44a676ece268'
const sensorCharacteristics = '6c00e224-f795-4b61-9c84-3b77d83ec099'

const sensorTreshold = Number(process.env.SENSOR_TRESHOLD); 
const lampAddress = Number(process.env.LAMP_ADDRESS)

const lampState = {
    color: 'red',
    brightness: 128,
    toggleOverride: false,
    sensorValue: 0
}

function updateLeds() {
    let brightness = 0
    const colors = convertColor(lampState.color)
    if (lampState.toggleOverride || lampState.sensorValue >= sensorTreshold) {
        brightness = lampState.brightness
    }

    let message = [
        Buffer.from([0x10, brightness]),
        Buffer.from([0x1, (colors.red ? 0x1 : 0x0)]),
        Buffer.from([0x2, (colors.green ? 0x1 : 0x0)]),
        Buffer.from([0x3, (colors.blue ? 0x1 : 0x0)])
    ];
    i2c.openPromisified(1).then(i2c1 => {
        message.forEach((buff, index) => {
            setTimeout(() => {
                i2c1.i2cWrite(lampAddress, buff.length, buff)
            }, index * 100)
        })
    })
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
                        } else if (characteristic.uuid == inputToggleCharacteristics) {
                            characteristic.read((err, data) => {
                                if (err) throw err
                                lampState.toggleOverride = data.toString("ascii") === 'true'
                            })
                        } else if (characteristic.uuid == sensorCharacteristics) {
                            characteristic.read((err, data) => {
                                if (err) throw err
                                lampState.sensorValue = parseInt(data.toString("hex"), 16)
                            })
                        }
                        resolve()
                    })

                })
                Promise.all(characteristicHandlers).then(() => {
                    updateLeds()
                })
            })
        })
    }
})