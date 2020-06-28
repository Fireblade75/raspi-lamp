
function convertColor(str) {
    switch(str) {
        case 'red': return { red: true, blue: false, green: false }
        case 'green': return { red: false, blue: false, green: true }
        case 'blue': return { red: false, blue: true, green: false }
        case 'orange': return { red: true, blue: false, green: true }
        case 'purple': return { red: true, blue: true, green: false }
        case 'cya ': return { red: false, blue: true, green: true }
        case 'white': return { red: true, blue: true, green: true }
        default: return { red: false, blue: false, green: false }
    }
}

module.exports = convertColor