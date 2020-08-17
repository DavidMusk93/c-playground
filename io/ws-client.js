const WebSocket = require('ws')

let ws = new WebSocket('ws://tencent.guohuasun.com:9002')

ws.onopen = function () {
    ws.send('Hi,there!')
}

ws.onmessage = function (event) {
    console.log(event.data)
}
