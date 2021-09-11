import Spool from "./Spool"

const scale = 1/8;
const Machine = () => {
    return (
        <svg class="w-2/3 h-2/3">
            <Spool windings={100}></Spool>
        </svg>
    )
}

export default Machine
