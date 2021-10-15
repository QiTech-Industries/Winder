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
        {
            name: "Start Puller",
            action: () => socket.emit("pull", { mpm: speed }),
            active: stats.m != "pulling" && stats.m != "winding"
        },
        {
            name: "Start Winding",
            action: () => socket.emit("wind", { mpm: speed }),
            active: stats.m == "pulling"
        },
        {
            name: "Winding",
            action: () => socket.emit("power"),
            active: stats.m == "winding"
        },
        {
            name: "Unwind",
            action: () => socket.emit("unwind", { mpm: speed }),
            active: stats.m != "unwinding"
        },
        {
            name: "Unwinding",
            action: () => socket.emit("power"),
            active: stats.m == "unwinding"
        },
        {
            name: "Off",
            action: () => socket.emit("standby"),
            active: stats.m != "standby"
        },
        {
            name: "Power",
            action: () => socket.emit("power"),
            active: stats.m == "standby"
        }
    ];

    const changeSpeed = (mpm) => {
        socket.emit("speed", { mpm });
        setSpeed(mpm);
    }

    return (
        <div class="h-full text-black mx-4">
            <div class="py-2 w-full md:w-1/2 z-50 sticky top-0">
                <Menu enabled={stats.m != "standby"}>
                    {
                        menu.map(item => {
                            if (item.active) {
                                return <Button onClick={e => item.action()}>
                                    {item.name}
                                </Button>
                            }
                        })
                    }
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
        </div >
    )
}

export default Winder
