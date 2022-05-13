import Card from "./Card"
import { useStats } from "../utils/StatsProvider"
import { useConfig } from "../utils/ConfigProvider"

const grammPerMeter = 2.956; // filament-weight per length in gram
const filamentDiameter = 1.9; // diameter of filament in mm

/**
 * Display the winding-related statistics such as running time and meters wound
 */
const Spool = () => {
    const time = useStats(state => state.stats.t);
    const windings = useStats(state => state.stats.w);
    const meters = useStats(state => state.stats.l);
    const { config } = useConfig();

    const spoolWidth = config.ferrari_max - config.ferrari_min;
    const windingsPerLayer = spoolWidth / filamentDiameter;

    const layerCount = Math.floor(windings / windingsPerLayer);
    const weight = (meters * grammPerMeter).toFixed(0);

    return (
        <div class="flex flex-wrap mx-auto py-4">
            <div class="font-bold text-3xl text-center w-full bg-white rounded-lg p-4 mb-2">Statistics</div>
            <Card icon="ruler" name="Length Wound" value={meters + "m"} />
            <Card icon="layer" name="Layers Wound" value={layerCount} />
            <Card icon="timer" name="Running Time" value={time + "s"} />
            <Card icon="rotations" name="Total Rotations" value={windings} />
            <Card icon="weight" name="Estimated Weight" value={weight + "g"} />
        </div>
    )
}

export default Spool
