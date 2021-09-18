const WebSocket = require('ws')

const wss = new WebSocket.Server({ port: 5000 });
let status = {
    s: {//spool
        r: 0,//rpm
        s: 0,//stall
        a: false,//active
    },
    p: {//puller
        r: 0,
        s: 0,
        a: false,
    },
    f: {//ferrari
        r: 0,
        s: 0,
        a: false,
    },
    m: "standby",//mode
    w: 0, // total windings
    l: 0, // total length
    e: null//error
};

wss.on('connection', function connection(ws) {
    console.log("connection established");

    const send = (event, data, delay = 0) => {
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
                    { rssi: -72, ssid: "Mein Netzwerk", secure: 1 },
                    { rssi: -90, ssid: "Dein Netzwerk", secure: 0 },
                    { rssi: -5, ssid: "Unser Netzwerk", secure: 2 },
                    { rssi: -72, ssid: "Mein Netzwerk", secure: 1 },
                    { rssi: -110, ssid: "Das Netzwerk", secure: 3 },
                ], 1000);
                break;

            case "connect":
                send(json.event, "connected", 1000)
                break;

            case "wind":
                console.log(status.m);
                if(status.m != "winding") status.m = "winding";
                else status.m = "power";
            break;

            case "config":
                send(json.event, {
                    wifi: {
                        ssid: "Mein Netzwek",
                        password: "secure",
                        ap_ssid: "Mein Winder",
                        ap_password: "asdasdasd",
                        friendly_name: "CoolerName",
                        mdns_name: "dsfsdfsdf",
                        ap_enabled: false,
                    },
                    ferrari_min: 50,
                    ferrari_max: 100,
                }, 500);
                break;

            default:
                console.log("unknown event");
                break;
        }
    });

    setInterval(() => {
        if(status.m == "winding"){
            status.f.a = true;
            status.f.r = Math.floor(Math.random() * 50);
            status.s.a = true;
            status.s.r = Math.floor(Math.random() * 50);
            status.p.a = true;
            status.p.r = Math.floor(Math.random() * 50);
            status.w = status.w + 1;
            status.l = status.l + 1;
        }
        send("stats", status);
    }, 1000);
});

// Add hosts entry and create por forwarding with
// netsh interface portproxy add v4tov4 listenport=80 listenaddress=127.65.43.21 connectport=5000 connectaddress=127.0.0.1