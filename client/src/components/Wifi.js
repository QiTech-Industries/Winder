import { useState } from "preact/hooks"
import Loading from "./Loading"
import WifiItem from "./WifiItem";
import Button from "./Button";
import { useEffect } from "preact/hooks"
import { useSocket } from '../utils/SocketProvider';
import { useToast } from '../utils/ToastProvider'

const Wifi = () => {
    const [networks, setNetworks] = useState([]);
    const [loading, setLoading] = useState(true);

    const [showInput, setShowInput] = useState(null);
    const [connecting, setConnecting] = useState(null);
    const [password, setPassword] = useState();

    const { socket } = useSocket();
    const { toast } = useToast();

    const security = [
        "Open", "WEP", "WPA/PSK", "WPA2/PSK", "WPA2/PSK", "unknown", "unknown", "unknown"
    ];

    useEffect(() => {
        console.log("firstr scan");
        scan();

        return () => { socket.off("scan") };
    }, [])

    const scan = () => {
        setLoading(true);
        setNetworks([]);
        setShowInput(null);
        socket.emit("scan", undefined, data => {
            if (data.length != 0) {
                data = data.filter((thing, index, self) =>
                index === self.findIndex((t) => (
                  t.ssid === thing.ssid
                ))
              );
                data = data.map((network, index) => {
                    return {
                        id: index + 1,
                        name: network.ssid,
                        type: security[network.secure],
                        strength: network.rssi > -70 ? 3 : (network.rssi > -85) ? 2 : 1
                    };
                });
                setNetworks(data);
                setLoading(false);
            }
            else {
                console.log("rescan");
                scan();
            }
        });
    }

    const connect = (id) => {
        setConnecting(id);
        const name = networks.find(network => {
            return network.id === id
        }).name;

        socket.emit("connect", { name, password }, data => {
            setConnecting(null);
            if (data === "connected") {
                setLoading(false);
                setShowInput(null);
                toast.success(`Successfully connected to network '${name}'`);
            }
            else if (data === "failed") {
                toast.error(`Connection to network '${name}' failed.`);
            }
        });
    }

    return (
        <div class="flex items-center flex-grow mx-4">
            <div class="bg-white md:w-1/3 rounded-md w-full mx-auto my-auto py-5 md:px-5 text-center">
                <h2 class="text-4xl mb-3">Connect to Wifi</h2>
                {
                    loading
                        ?
                        <Loading className="text-9xl" text="Searching for Networks..." />
                        :
                        <>
                            <div class={`${networks.length > 4 ? "overflow-y-scroll" : ""} h-96 px-2 my-2`}>
                                {
                                    networks.map((network) => {
                                        return (
                                            <WifiItem name={network.name} connecting={network.id === connecting} setPassword={setPassword} setConnecting={e => connect(network.id)} strength={network.strength} type={network.type} showInput={network.id === showInput} setShowInput={e => setShowInput(network.id)} />
                                        );
                                    })
                                }
                            </div>
                            <div class="flex">
                                <div class="flex items-center m-auto">
                                    <p>{networks.length} {networks.length === 1 ? "Network" : "Networks"} found</p>
                                    <div class="h-8 w-8 ml-3">
                                        <Button onClick={scan}><div class="icon-refresh" /></Button>
                                    </div>
                                </div>
                            </div>
                        </>
                }
            </div>
        </div>
    )
}

export default Wifi
