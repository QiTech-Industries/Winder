import Menu from "./Menu"
import Button from "./Button"
import Stepper from "./Stepper"
import Spool from "./Spool"
import { useState, useEffect } from "preact/hooks"
import { useSocket } from '../utils/SocketProvider';
import { useStats } from '../utils/StatsProvider';
import Slider from "./Slider"

const Winder = () => {
    const [speed, setSpeed] = useState(3);

    const { socket } = useSocket();
    const { stats, charts } = useStats();

    const menu = [
        { inactive: "Wind", active: "Winding", action: "wind", mode: "winding" },
        { inactive: "Unwind", active: "Unwinding", action: "unwind", mode: "unwinding" },
    ]

    const mode2Action = (mode) => {
        return menu.find(item => item.mode === mode).action;
    }

    // update current speed in meter/minute
    const changeSpeed = (mpm) => {
        setSpeed(mpm);
        if (stats.m === "winding" || stats.m === "unwinding") {
            socket.emit(mode2Action(stats.m), { mpm }, () => {});
        }
    }

    return (
        <div class="h-full text-black mx-4">
            <div class="py-2 w-full md:w-1/2 z-50 sticky top-0">
                <Menu enabled={stats.m != "standby"}>
                    {
                        menu.map(item => {
                            // stop if clicked mode is active, start if inactive
                            return <Button secondary={item.mode === stats.m} onClick={e => socket.emit(item.action, { mpm: item.mode === stats.m ? 0 : speed })}>
                                {
                                    item.mode === stats.m
                                        ? item.active
                                        : item.inactive
                                }
                            </Button>
                        })
                    }
                    <Button onClick={e => socket.emit("power")}>
                        {stats.m != "standby" ? "Power Off" : "Power On"}
                    </Button>
                </Menu>
            </div>
            <div class="max-w-6xl mx-auto">
                <div class="flex flex-wrap items-center justify-around md:flex-nowrap md:space-x-8">
                    <Stepper stats={stats.p} chart={charts.p} name="Puller" />
                    <Stepper stats={stats.f} chart={charts.f} name="Ferrari" />
                    <Stepper stats={stats.s} chart={charts.s} name="Spool" />
                </div>
                <div class="col-span-3 w-full text-center py-3">
                    <h3 class="text-white text-2xl pb-2">Speed Adjust</h3>
                    <Slider min={0.5} max={14} minThumb={speed} step={0.1} onChange={mpm => changeSpeed(mpm)} value={speed} />
                </div>
                <Spool />
            </div>
        </div>
    )
}

export default Winder
