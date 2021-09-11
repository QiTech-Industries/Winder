import { useState } from "preact/hooks"
import Range from "./Range"
import Section from "./Section"
import Text from "./Text"
import Switch from "./Switch"
import { useConfig } from "../utils/ConfigProvider"
import { useSocket } from "../utils/SocketProvider"

const Config = () => {
    const { config } = useConfig();
    const { socket } = useSocket();

    const calibrate = (position, startPos) => {
        console.log({position, startPos});
        socket.emit("calibrate", {position, startPos}, () => {

        });
    }

    return (
        <div class="items-center w-full h-full m-x-auto px-48 text-white">
            <Section title="WIFI">
                <Text placeholder="winder" value={config.wifi.mdns_name} tooltip="This will change the URL of the webinterface." label="Device Name" unit=".local"/>
                <Text placeholder="Jarvis Winder" value={config.wifi.ap_ssid} tooltip="Name of the network the winder creates." label="Access Point Name" />
                <Switch on={config.wifi.ap_enabled} left="No" right="Yes" />
            </Section>
            <Section title="SPOOL CALIBRATION">
                <div class="w-full text-black">
                <Range min={0} max={150} span={10} minThumb={config.ferrari_min} maxThumb={config.ferrari_max}
                 step={1} minChange={val => calibrate(val, true)} maxChange={val => calibrate(val, false)}/>
                </div>
            </Section>
            <Section title="INFO">
                <div class=" flex justify-between w-full">
                <div>
                        <div class="mb-2">Hardware Info:</div>
                        <div class="grid grid-cols-2 gap-x-4">
                            <span>Board</span>
                            <span>ESP 32</span>
                            <span>Revision</span>
                            <span>2</span>
                        </div>
                    </div>
                    <div>
                        <div class="mb-2">ESP Firmware:</div>
                        <div class="grid grid-cols-2 gap-x-4">
                            <span>Version</span>
                            <span>1.0.0</span>
                            <span>Build</span>
                            <span>123456</span>
                            <span>Last Update</span>
                            <span>06/23/2021</span>
                        </div>
                    </div>
                    <div>
                        <div class="mb-2">Interface:</div>
                        <div class="grid grid-cols-2 gap-x-4">
                            <span>Version</span>
                            <span>1.0.0</span>
                            <span>Build</span>
                            <span>123456</span>
                            <span>Last Update</span>
                            <span>06/23/2021</span>
                        </div>
                    </div>
                </div>
            </Section>
        </div>
    )
}

export default Config
