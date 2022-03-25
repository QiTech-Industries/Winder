import Range from "./Range"
import Section from "./Section"
import Text from "./Text"
import Switch from "./Switch"
import InfoItem from "./InfoItem"
import { useRef, useEffect } from "preact/hooks"
import { useConfig } from "../utils/ConfigProvider"
import { useSocket } from "../utils/SocketProvider"
import { useToast } from "../utils/ToastProvider"

/**
 * Page of the configure-view
 */
const Config = () => {
    const { config, setConfig } = useConfig();
    const { socket } = useSocket();
    const addToast = useToast(state => state.addToast)

    /**
     * Sends a message to the backend to save / apply the new calibration-position
     * @param {Integer} position new position (distance from home in mm)
     * @param {Boolean} startPos true = start-position(left), false = end-position(right)
     */
    const calibrate = (position, startPos) => {
        const newConfig = {...config};

        if(startPos){
            newConfig.ferrari_min = position;
        }
        else{
            newConfig.ferrari_max = position;
        }
        setConfig(newConfig);
        socket.emit("calibrate", { position, startPos }, () => {
        });
    }

    /**
     * Sends a message to the backend to save / apply the new wifi-configuration (for creating an accesspoint)
     * @param {*} key TODO: Comment
     * @param {*} val TODO: Comment
     */
    const updateWifi = (key, val) => {
        const newConfig = {...config};
        newConfig.wifi[key] = val;
        setConfig(newConfig);
        const wifi = {
            mdns_name: newConfig.wifi.mdns_name,
            ap_ssid: newConfig.wifi.ap_ssid,
            ap_enabled: newConfig.wifi.ap_enabled
        }
        socket.emit("modify", wifi, () => {
            addToast("success","Configuration successfully updated. Please restart.");
        });
    }

    return (
        <div class="max-w-6xl w-full mx-auto py-4 px-4 text-white">
            <Section title="WIFI">
                <Text onInput={val => updateWifi("mdns_name", val.toLowerCase())} placeholder="winder" value={config.wifi.mdns_name} tooltip="This will change the URL of the webinterface." label="Device Name" unit=".local" />
                <Text onInput={val => updateWifi("ap_ssid", val)} placeholder="Jarvis Winder" value={config.wifi.ap_ssid} tooltip="Name of the network the winder creates." label="Access Point Name" />
                <Switch on={config.wifi.ap_enabled} onChange={val => updateWifi("ap_enabled", val)} left="No" right="Yes" />
            </Section>
            <Section title="SPOOL CALIBRATION">
                <div class="w-full text-black">
                    <Range min={0} max={150} span={10} minThumb={config.ferrari_min} maxThumb={config.ferrari_max}
                        step={1} minChange={val => calibrate(val, true)} maxChange={val => calibrate(val, false)} />
                </div>
            </Section>
            <Section title="INFO">
                <div class="flex justify-around w-full flex-wrap">
                    <InfoItem data={{Board: "ESP 32", Revision: "2"}} headline="Hardware Info" />
                    <InfoItem data={{Version: config.software.firmware.version, Build: config.software.firmware.build, "Last Update": config.software.firmware.date}} headline="ESP Firmware" />
                    <InfoItem data={{Version: config.software.spiffs.version, Build: config.software.spiffs.build, "Last Update": config.software.spiffs.date}} headline="Interface" />
                </div>
            </Section>
        </div>
    )
}

export default Config
