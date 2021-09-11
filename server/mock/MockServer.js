const WebSocket = require('ws')

const wss = new WebSocket.Server({ port: 5000 });

wss.on('connection', function connection(ws) {
    console.log("connection established");

    const send = (event, data, delay) => {
        setTimeout(() => {
            ws.send(JSON.stringify({ event, data }));
        }, delay);
    }

    ws.on('message', function incoming(message) {
        console.log(`recieved: ${message}`);

        const json = JSON.parse(message);
        if (!json.event) {
            console.log("JSON format error");
            return;
        }

        switch (json.event) {
            case "scan":
                send(json.event, [
                    { id: 1, name: "test", strength: 1, type: "WPA/PSK" },
                    { id: 2, name: "test", strength: 3, type: "Open" },
                    { id: 3, name: "test", strength: 2, type: "Open" },
                    { id: 4, name: "test", strength: 4, type: "Open" },
                ], 1000)
                break;
            
            case "connect":
                send(json.event, "connected", 1000)
                break;
                
            default:
                console.log("unknown event");
                break;
        }
    });
});