class Socket {
    constructor(url) {
        this.callbacks = [];
        this.connects = 0;
        this.reconnectMax = 10;
        this.reconnectInterval = 5e3;
        this.pingInterval = 4e3;
        this.timer;
        this.url = url;
        this.ws;
    }

    open() {
        this.ws = new WebSocket(this.url);
        this.ws.onopen = e => {
            this.connects = 0;
            this.#run("open");
        }
        this.ws.onclose = e => {
            e.code === 1000 || e.code === 1001 || e.code === 1005 || this.#reconnect(); // only reconnect if close was unintentional
            if (this.connects === 0) {
                this.#run("close");
            }
        }
        this.ws.onerror = e => {
            (e && e.code === 'ECONNREFUSED') ? this.#reconnect() : this.#run("error");
        }
        this.ws.onmessage = e => {
            const data = JSON.parse(e.data);
            if (data.event && data.data) {
                this.#run(data.event, data.data);
            }
            else {
                this.#run("corrupt");
            }
        }
    }

    on(event, callback) {
        console.log(this.callbacks);
        this.callbacks[event] = callback;
    }

    off(event) {
        if (typeof this.callbacks[event] === "undefined") return; // no callback for this event
        delete this.callbacks[event];
    }

    emit(event, data = {}, cb) {
        this.on(event, cb);
        if (this.ws && this.ws.readyState == 1) {
            this.ws.send(JSON.stringify({ event, data }));
        }
    }

    #run(event, data) {
        if (typeof this.callbacks[event] === "undefined") return; // no callback for this event
        this.callbacks[event](data);
    }

    #reconnect(e) {
        if (this.connects++ < this.reconnectMax) {
            this.timer = setTimeout(() => {
                this.#run("reconnect");
                this.open();
            }, this.reconnectInterval);
        } else {
            this.#run("reload");
        }
    }
}

//const socket = new Socket(`ws://${window.host}/ws`);
//const socket = new Socket('ws://winder.local/ws');
const socket = new Socket('ws://localhost:5000/ws');
socket.open();
export { socket };
