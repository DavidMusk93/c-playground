const WebSocket = require('ws')

let ws = new WebSocket('ws://tencent.guohuasun.com:9002/?w1=182&w2=184')

ws.onopen = function () {
    ws.send('Hi,there!')
}

ws.onmessage = function (event) {
    if(event.data.startsWith('{')){
        const obj = JSON.parse(event.data)
        console.log(obj.topic, obj.weight, obj.id)
    }else{
        console.log(event.data)
    }
}
