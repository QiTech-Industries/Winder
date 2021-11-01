const Card = ({ name, value, icon, color }) => {
    return (
        <div class="w-full md:w-1/2">
        <div class="flex bg-white rounded-lg p-4 border-l-8 mx-4 my-2 border-primary">
            <div class="flex items-center justify-center h-12 w-12 rounded-xl bg-ternary text-primary text-xl">
                <i class={`icon-${icon}`} />
            </div>
            <div class="flex flex-col ml-4">
                <div class="text-sm text-gray-500">{name}</div>
                <div class="font-bold text-lg">{value}</div>
            </div>
        </div>
        </div>
    )
}

export default Card
