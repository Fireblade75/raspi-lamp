
/**
 * 
 * @param {string} peripheral the address of the peripheral
 * @param {string[]} services 
 */
function logService(peripheral, services) {
    console.log(`${peripheral} is hosting the following services`)
    services.forEach(service => {
        console.log(' - ' + service)
    })
    console.log('')
}

module.exports = logService