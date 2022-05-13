import { useState } from "preact/hooks"
import Loading from "./Loading"
import WifiItem from "./WifiItem";
import Button from "./Button";
import { useEffect } from "preact/hooks"
import { useSocket } from '../utils/SocketProvider';
import { useToast } from '../utils/ToastProvider'

/**
 * Page of the connect-view
 */
const Wifi = () => {
    const [networks, setNetworks] = useState([]);
    const [loading, setLoading] = useState(true);

    const [showInput, setShowInput] = useState(null);
    const [connecting, setConnecting] = useState(null);
    const [connected, setConnected] = useState(null);
    const [password, setPassword] = useState("");

    const { socket } = useSocket();
    const addToast = useToast(state => state.addToast)

    const security = [
        "Open", "WEP", "WPA/PSK", "WPA2/PSK", "WPA2/PSK", "unknown", "unknown", "unknown"
    ];

    useEffect(() => {
        scan();

        return () => { socket.off("scan") };
    }, [])

    const scan = () => {
        setLoading(true);
        setNetworks([]);
        setShowInput(null);
        socket.buffer("scan", undefined, data => {
            let newNetworks = data.networks;
            if (newNetworks.length != 0) {
                // filter duplicate netwoks by ssid
                newNetworks = newNetworks.filter((thing, index, self) =>
                    index === self.findIndex((t) => (
                        t.ssid === thing.ssid
                    ))
                );
                // transform networks into new format
                newNetworks = newNetworks.map((network, index) => {
                    if(data.current == network.ssid ) setConnected(index + 1);
                    return {
                        id: index + 1,
                        name: network.ssid != "" ? network.ssid : "Hidden Network",
                        type: security[network.secure],
                        strength: network.rssi > -70 ? 3 : (network.rssi > -85) ? 2 : 1
                    };
                });
                setNetworks(newNetworks);
                setLoading(false);
            }
            else scan();
        });
    }

    const connect = (id) => {
        setConnecting(id);
        const name = networks.find(network => {
            return network.id === id
        }).name;

        socket.buffer("connect", { name, password }, data => {
            setConnecting(null);
            if (data === "connected") {
                setLoading(false);
                setShowInput(null);
                addToast("success", `Successfully connected to network '${name}'`);
                setConnected(id);
            }
            else if (data === "failed") {
                addToast("error", `Connection to network '${name}' failed.`);
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
                                            <WifiItem connected={connected === network.id} name={network.name} connecting={network.id === connecting} setPassword={setPassword} setConnecting={e => connect(network.id)} strength={network.strength} type={network.type} showInput={network.id === showInput} setShowInput={e => setShowInput(network.id)} />
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
