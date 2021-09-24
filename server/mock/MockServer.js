const WebSocket = require('ws')

const wss = new WebSocket.Server({ port: 5000 });
let status;
let network = "Mein Netzwerk";

const wind = () => {
    status = {
        s: {//spool
            r: Math.floor(Math.random() * 50),//rpm
            s: 0,//stall
            a: true,//active
        },
        p: {//puller
            r: Math.floor(Math.random() * 50),
            s: 0,
            a: true,
        },
        f: {//ferrari
            r: Math.floor(Math.random() * 50),
            s: 0,
            a: true,
        },
        m: "winding",//mode
        w: status.w + 1, // total windings
        l: status.w + 1, // total length
        e: null//error
    };
}

const power = () => {
    status = {
        s: {//spool
            r: 0,//rpm
            s: 0,//stall
            a: true,//active
        },
        p: {//puller
            r: 0,
            s: 0,
            a: true,
        },
        f: {//ferrari
            r: 0,
            s: 0,
            a: true,
        },
        m: "power",//mode
        w: status.w, // total windings
        l: status.w, // total length
        e: null//error
    };
}

const unwind = () => {
    status = {
        s: {//spool
            r: 0,//rpm
            s: 0,//stall
            a: false,//active
        },
        p: {//puller
            r: Math.floor(Math.random() * 50),
            s: 0,
            a: true,
        },
        f: {//ferrari
            r: 0,
            s: 0,
            a: false,
        },
        m: "unwinding",//mode
        w: 0, // total windings
        l: 0, // total length
        e: null//error
    };
}

const standby = () => {
    status = {
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
}

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
                send(json.event, {
                    networks: [
                        { rssi: -72, ssid: "Mein Netzwerk", secure: 1 },
                        { rssi: -90, ssid: "Dein Netzwerk", secure: 0 },
                        { rssi: -5, ssid: "Unser Netzwerk", secure: 2 },
                        { rssi: -72, ssid: "Mein Netzwerk", secure: 1 },
                        { rssi: -110, ssid: "Das Netzwerk", secure: 3 }],
                    current: network
                }, 1000);
                break;

            case "connect":
                if (Math.random() < 0.5) {
                    send(json.event, "connected", 1000);
                    network = json.data.name;
                    break;
                }
                send(json.event, "failed", 1000)
                break;

            case "wind":
                console.log(status.m);
                if(json.data.mpm != 0){
                    status.m = "winding"      
                }
                else{
                    status.m = "power";    
                }
                break;

            case "unwind":
                console.log(status.m);
                if(json.data.mpm != 0){
                    status.m = "unwinding"      
                }
                else{
                    status.m = "power";    
                }
                break;

            case "power":
                if (status.m != "power") status.m = "power";
                else status.m = "standby";
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
        switch (status.m) {
            case "winding":
                wind();
                break;

            case "unwinding":
                unwind();
                break;

            case "power":
                power();
                break;

            default:
                standby();
                break;
        }
        send("stats", status);
    }, 1000);
});

standby();

// Add hosts entry and create por forwarding with
// netsh interface portproxy add v4tov4 listenport=80 listenaddress=127.65.43.21 connectport=5000 connectaddress=127.0.0.1