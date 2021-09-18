import { useState, useEffect, useRef } from "preact/hooks"
import Number from "./Number"

const Range = (props) => {

    const minRange = props.min;
    const maxRange = props.max;

    const [min, setMin] = useState({
        value: props.minThumb,
        thumb: 0
    });

    // needed for debouncing slider inside Timeout
    const valRef = useRef();

    const minTrigger = (input = min.value) => {
        let value = input < minRange ? minRange : (input > maxRange ? maxRange : input);
        let thumb = ((value - minRange) / (maxRange - minRange)) * 100;
        setMin({ ...min, value, thumb });

        valRef.current = value;
        setTimeout(value => {
            if (value == valRef.current) props.onChange(value);
        }, 500, value);
    };

    useEffect(() => {
        minTrigger();
        valRef.current = null;
    }, []);

    return (
        <div class="relative w-full">
            <div>
                <input type="range"
                    min={minRange}
                    max={maxRange}
                    value={min.value}
                    onInput={e => minTrigger(e.target.value)}
                    step={props.step}
                    class="absolute pointer-events-none appearance-none z-20 h-2 w-full cursor-pointer left-0 opacity-0" />

                <div class="relative z-10 h-2">

                    <div class="absolute z-10 left-0 right-0 bottom-0 top-0 rounded-md bg-gray-200"></div>

                    <div class="absolute z-20 top-0 bottom-0 bg-red-300 rounded-full" style={{ left: 0, width: `${min.thumb}%` }}></div>

                    <div class="absolute z-30 w-6 h-6 top-0 right-0 bg-green-300 rounded-full -mt-2" style={{ left: `calc(${min.thumb}% - (${min.thumb * 0.24}px))` }}></div>

                </div>
            </div>

            <div class="pt-2 pb-4 w-full md:w-44">
                <Number value={min.value} onChange={minTrigger} unit="m/min" />
            </div>
        </div>
    )
};

export default Range
