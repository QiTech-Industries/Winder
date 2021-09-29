const Card = ({ name, value, icon, color }) => {
    return (
        <div class="flex bg-white shadow-md rounded p-4 border-gray-500 border-l-4 m-2 w-full md:w-80">
            <div class="flex items-center justify-center h-12 w-12 rounded-xl bg-blue-100 text-blue-500 text-xl">
                <i class={`icon-${icon}`} />
            </div>
            <div class="flex flex-col ml-4">
                <div class="text-sm text-gray-500">{name}</div>
                <div class="font-bold text-lg">{value}</div>
            </div>
        </div>
    )
}

export default Card
