import Label from "./Label"

const Text = ({ placeholder, tooltip, label, unit, value, onInput }) => {
    return (
        <div class='w-full mb-6'>
            <Label tooltip={tooltip} label={label} />
            <div class="flex">
                <input class={`
                min-w-0 appearance-none block w-full bg-white text-ternary border border-gray-400 shadow-inner py-3 px-4 leading-tight focus:outline-none  focus:border-gray-500
                ${unit ? "rounded-l-md" : "rounded-md"}
                `} type='text' placeholder={placeholder} value={value} onBlur={e => onInput(e.target.value)}/>
                {
                    unit
                        ?
                        <div class="bg-gray-400 px-1 rounded-r-md flex items-center text-black">
                            <p>
                                {unit}
                            </p>
                        </div>
                        :
                        null
                }
            </div>
        </div>
    )
};

export default Text
