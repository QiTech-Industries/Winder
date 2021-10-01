import Label from "./Label"

const Number = ({ onChange, value, unit, label, tooltip, step = 1 }) => {
    value = parseFloat(value);
    return (
        <div class='w-full'>
            <Label tooltip={tooltip} label={label} />
            <div class="flex h-10">
                <input step={step} onChange={e => onChange(e.target.value)} type="number" value={value} maxlength="5" class="pl-3 py-2 rounded-l w-full text-left border-t border-b border-l border-gray-400 focus:outline-none" />
                <div class="h-10 bg-white px-1 border-t border-b border-gray-400">
                    <p class="pt-2">
                        {unit}
                    </p>
                </div>
                <div class="flex flex-wrap w-5 text-2xl leading-3">
                    <button class="bg-gray-400 h-5 w-5 rounded-tr border-b border-gray-600 transition duration-200 focus:bg-gray-500" onClick={e => onChange(value + 1)}>+</button>
                    <button class="bg-gray-400 h-5 w-5 rounded-br border-t border-gray-600 transition duration-200 focus:bg-gray-500" onClick={e => onChange(value - 1)}>-</button>
                </div>
            </div>
        </div>
    )
}

export default Number
