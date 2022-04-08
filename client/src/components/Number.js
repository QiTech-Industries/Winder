import Label from "./Label"

/**
 * 
 * @param {Function(value)} onChange function to be called with new value on value change
 * @param {Number} value initial value
 * @param {String} unit unit of value to be displayed (for example meters)
 * @param {String} label text of optional label
 * @param {String} tooltip tooltip of optional label
 * @param {Number} step step-size for changing the value
 */
const Number = ({ onChange, value, unit, label, tooltip, step = 1 }) => {
    value = parseFloat(value);

    /**
     * Call onChange-function on 'enter'-key
     * @param {Event} keyEvent 
     */
    const _handleKeyEvent = (keyEvent) => {
        if(keyEvent.key === 'Enter'){
            onChange(keyEvent.target.value);
        };
    };

    return (
        <>
            <Label tooltip={tooltip} label={label} />
            <div class="flex h-10">
                <input step={step} onKeyPress={e => _handleKeyEvent(e)} onBlur={e => onChange(e.target.value)} type="number" value={value} maxlength="5" class="min-w-0 pl-3 py-2 rounded-l text-left border-t border-b border-l border-gray-400 focus:outline-none" />
                <div class="h-10 bg-white px-1 border-t border-b border-gray-400">
                    <p class="pt-2">
                        {unit}
                    </p>
                </div>
                <div class="w-5 text-2xl leading-3">
                    <button class="bg-gray-400 h-5 w-5 rounded-tr border-b border-gray-600 transition duration-200 focus:bg-gray-500" onClick={e => onChange(value + 1)}>+</button>
                    <button class="bg-gray-400 h-5 w-5 rounded-br border-t border-gray-600 transition duration-200 focus:bg-gray-500" onClick={e => onChange(value - 1)}>-</button>
                </div>
            </div>
        </>
    )
}

export default Number
