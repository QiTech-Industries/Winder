import Card from "./Card"

const Spool = () => {
    return (
        <div class="bg-gray-200 w-full h-30 rounded-xl p-4 m-4 grid grid-cols-5">
            <img class="h-72 col-span-2" src="/assets/spool.svg" />
            <div class="grid grid-cols-2 col-span-3">
                <Card icon="ruler" name="Winded Meters" value="1647"/>
                <Card icon="layer" name="Winded Layers" value="1647"/>
                <Card icon="timer" name="Running Time" value="1647"/>
                <Card icon="rotations" name="Total Rotations" value="1647"/>
                <Card icon="weight" name="Estimated Weight" value="1647"/>
            </div>
        </div>
    )
}

export default Spool
