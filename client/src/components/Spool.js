import Card from "./Card"
import { useStats } from "../utils/StatsProvider"
import { useConfig } from "../utils/ConfigProvider"

const grammPerMeter = 2.956;
const filamentDiameter = 1.9;

const Spool = () => {
    const { stats } = useStats();
    const { config } = useConfig();

    const spoolWidth = config.ferrari_max - config.ferrari_min;
    const windings = stats.w;
    const meters = stats.w;
    const time = Date.now() - stats.t;
    const windingsPerLayer = spoolWidth/filamentDiameter;

    const layerCount = Math.floor(windings / windingsPerLayer);
    const weight = (meters * grammPerMeter).toFixed(0);

    return (
        <div class="bg-gray-200 w-full h-30 rounded-xl p-4 mb-4 flex flex-wrap md:flex-nowrap max-w-4xl mx-auto">
            <div class="flex justify-center place-items-center flex-wrap">
            <h3 class="text-2xl mb-2 mx-3 font-bold text-center">Status</h3>
            <img class="h-12 md:h-64 col-span-2" src="/assets/spool.svg" />
            </div>
            <div>
            <div class="flex flex-wrap">
                <Card icon="ruler" name="Meters Wound" value={meters}/>
                <Card icon="layer" name="Layers Wound" value={layerCount}/>
                <Card icon="timer" name="Running Time" value={time + "s"}/>
                <Card icon="rotations" name="Total Rotations" value={windings}/>
                <Card icon="weight" name="Estimated Weight" value={weight + "g"}/>
            </div>
            </div>
        </div>
    )
}

export default Spool
