import { createContext } from 'preact';
import { useContext, useEffect, useState } from 'preact/hooks'
import { useSocket } from './SocketProvider';

const ConfigContext = createContext();

export const useConfig = () => {
    return useContext(ConfigContext);
}

export const ConfigProvider = ({ children }) => {
    const { socket } = useSocket();
    const [config, setConfig] = useState({
        wifi: {
            ssid: "",
            password: "",
            ap_ssid: "",
            ap_password: "",
            friendly_name: "",
            mdns_name: "",
            ap_enabled: true,
        },
        ferrari_min: 65,
        ferrari_max: 130,
        software: {
            spiffs: {
                version: "0.0.0",
                build: "unknown",
                date: "-"
            },
            firmware: {
                version: "0.0.0",
                build: "unknown",
                date: "-"
            }
        },
    });

    useEffect(() => {
        socket.buffer("config", undefined, data => {
            setConfig(data);
        });
    }, []);

    return (
        <ConfigContext.Provider value={{ config, setConfig }}>
            {children}
        </ConfigContext.Provider>
    )
}