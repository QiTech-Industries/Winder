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
    const { stats, history } = useStats();

    const menu = [
        {inactive: "Start Winding", active: "Stop Winding", event: "wind", mode: "winding"},
        {inactive: "Start Unwinding", active: "Stop Unwinding", event: "unwind", mode: "unwinding" },
    ]

    const wind = (mpm) => {
        setSpeed(mpm);
        socket.emit("wind", {mpm, start: false}, () => {

        });
    }

    return (
        <div class="h-full text-black">
            <div class="pl-6 pt-6 w-1/2 z-10 absolute">
                <Menu enabled={stats.m != "standby"}>
                    {
                        menu.map(item => {
                            return <Button secondary={item.mode === stats.m} onClick={e =>  socket.emit(item.event, {mpm: speed, start: true})}>
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
            <div class="grid grid-cols-3 px-32 place-items-center">
            <Stepper rpm={stats.p.r} active={stats.p.a} data={history.puller} name="Puller"/>
            <Stepper rpm={stats.f.r} active={stats.f.a} data={history.ferrari} name="Ferrari"/>
            <Stepper rpm={stats.s.r} active={stats.s.a} data={history.spool} name="Spool"/>
            <div class="col-span-3 flex items-center">
            <Spool />
            </div>
            <div class="col-span-3 w-full text-center">
            <h3 class="text-white text-2xl py-4">Speed Adjust</h3>
            <Slider min={0.5} max={14} minThumb={speed} step={0.1} onChange={mpm => wind(mpm)}/>
            </div>
            <Button>Finished Speeding? Let's roll!</Button>
            </div>
        </div>
    )
}

export default Winder
