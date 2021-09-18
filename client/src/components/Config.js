import { useState } from "preact/hooks"
import Range from "./Range"
import Section from "./Section"
import Text from "./Text"
import Switch from "./Switch"
import InfoItem from "./InfoItem"
import { useConfig } from "../utils/ConfigProvider"
import { useSocket } from "../utils/SocketProvider"
import { useToast } from '../utils/ToastProvider'

const Config = () => {
    const { config } = useConfig();
    const { socket } = useSocket();
    const { toast } = useToast();

    const calibrate = (position, startPos) => {
        console.log({ position, startPos });
        socket.emit("calibrate", { position, startPos }, () => {
        });
    }

    const updateWifi = (key, val) => {
        console.log("change config");
        const wifi = {
            mdns_name: config.wifi.mdns_name,
            ap_ssid: config.wifi.ap_ssid,
            ap_enabled: config.wifi.ap_enabled,
        }
        wifi[key] = val;
        socket.emit("modify", wifi, () => { });
        toast.success("Configuration successfully updated.");
    }

    return (
        <div class="items-center w-full md:w-3/4 mx-auto h-full px-4 text-white">
            <Section title="WIFI">
                <Text onInput={val => updateWifi("mdns_name", val.toLowerCase())} placeholder="winder" value={config.wifi.mdns_name} tooltip="This will change the URL of the webinterface." label="Device Name" unit=".local" />
                <Text onInput={val => updateWifi("ap_ssid", val)} placeholder="Jarvis Winder" value={config.wifi.ap_ssid} tooltip="Name of the network the winder creates." label="Access Point Name" />
                <Switch on={config.wifi.ap_enabled} left="No" right="Yes" />
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
