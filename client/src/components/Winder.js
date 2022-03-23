import Button from "./Button"
import Stepper from "./Stepper"
import Spool from "./Spool"
import { useState } from "preact/hooks"
import { useSocket } from '../utils/SocketProvider';
import { useStats } from "../utils/StatsProvider";
import Slider from "./Slider"

/**
 * Page of the control-view
 */
const Winder = () => {
    const [speed, setSpeed] = useState(3);
    
    const { socket } = useSocket();
    const mode = useStats(state => state.stats.m)

    const menu = [
        {
            name: "Start Pulling",
            action: () => socket.emit("pull", { mpm: speed }),
            active: mode != "pulling" && mode != "winding" && mode != "changing"
        },
        {
            name: "Start Winding",
            action: () => socket.emit("wind", { mpm: speed }),
            active: mode == "pulling" || mode == "changing"
        },
        {
            name: "Stop Winding",
            action: () => socket.emit("power"),
            active: mode == "winding"
        },
        {
            name: "Change Spool",
            action: () => socket.emit("change"),
            active: mode == "winding"
        },
        {
            name: "Start Unwinding",
            action: () => socket.emit("unwind", { mpm: speed }),
            active: mode != "unwinding"
        },
        {
            name: "Stop Unwinding",
            action: () => socket.emit("power"),
            active: mode == "unwinding"
        },
        {
            name: "Turn Off",
            action: () => socket.emit("standby"),
            active: mode != "standby"
        },
        {
            name: "Turn On",
            action: () => socket.emit("power"),
            active: mode == "standby"
        }
    ];

    /**
     * 
     * @param {Number} mpm puller-speed in meters per minute
     */
    const changeSpeed = (mpm) => {
        socket.emit("speed", { mpm });
        setSpeed(mpm);
    }

    return (
        <>
            <div class="max-w-6xl mx-auto py-4 px-4">
                <div class="flex flex-wrap items-center justify-around md:flex-nowrap md:space-x-8">
                    <Stepper state="p" name="Puller" />
                    <Stepper state="f" name="Ferrari" />
                    <Stepper state="s" name="Spool" />
                </div>
                <div class="w-full text-center my-4">
                    <div class="rounded-t-lg w-48 right-0 px-2 py-1 bg-primary font-bold">Speed Adjust</div>
                    <div class="w-full bg-white rounded-lg rounded-tl-none px-2 py-3 shadow-inner">
                        <Slider min={0.5} max={14} minThumb={speed} step={0.1} onChange={mpm => changeSpeed(mpm)} />
                    </div>
                </div>
                <Spool />
            </div >
            <div class="sticky bottom-0 w-full bg-white z-50">
                <div class="w-full h-2 bg-primary"></div>
                <div class="w-full border-t-4">
                    <div class="flex mx-auto px-4 my-4 space-x-4 justify-center">
                        {
                            menu.map(item => {
                                if (item.active) {
                                    return (
                                        <div class="w-36">
                                            <Button secondary onClick={e => item.action()}>
                                                {item.name}
                                            </Button>
                                        </div>
                                    )
                                }
                            })
                        }
                    </div>
                </div>
            </div>
        </>
    )
}

export default Winder
