import { useState, useEffect, useRef } from "preact/hooks"
import Number from "./Number"

/**
 * Slider-input combo for calibration input (start- and endposition for ferrari-oscillation)
 * Used in configure-view
 * @param {*} props TODO: Comment
 */
const Range = (props) => {

    const minSpan = props.span;
    const minRange = props.min;
    const maxRange = props.max;

    const [min, setMin] = useState({
        value: props.minThumb,
        thumb: 0
    });

    const [max, setMax] = useState({
        value: props.maxThumb,
        thumb: 0
    });

    // needed for debouncing slider inside Timeout
    const valRef = useRef();

    const minTrigger = (input = min.value) => {
        let value = Math.min(input >= minRange ? input : minRange, max.value - minSpan);
        let thumb = ((value - minRange) / (maxRange - minRange)) * 100;
        setMin({ ...min, value, thumb });


        valRef.current = value;
        setTimeout(value => {
            if (value == valRef.current) props.minChange(value);
        }, 500, value);
    };

    const maxTrigger = (input = max.value) => {
        let value = Math.max(input <= maxRange ? input : maxRange, min.value + minSpan);
        let thumb = 100 - (((value - minRange) / (maxRange - minRange)) * 100);
        setMax({ ...max, value, thumb });

        valRef.current = value;
        setTimeout(value => {
            if (value == valRef.current) props.maxChange(value);
        }, 500, value);
    };

    useEffect(() => {
        minTrigger(props.minThumb);
        maxTrigger(props.maxThumb);
        valRef.current = null;
    }, []);

    useEffect(() => {
        minTrigger(props.minThumb);
        maxTrigger(props.maxThumb);
        valRef.current = null;
    }, [props.minThumb, props.maxThumb]);

    return (
        <div class="relative w-full">
            <div>
                <input type="range"
                    min={minRange}
                    max={maxRange}
                    value={min.value}
                    onInput={e => minTrigger(e.target.value)}
                    step={props.step}
                    class="left-thumb absolute pointer-events-none appearance-none z-20 opacity-0 h-2 w-full cursor-pointer" />
                {/* comment opacity-0 for testing */}

                <input type="range"
                    min={minRange}
                    max={maxRange}
                    value={max.value}
                    onInput={e => maxTrigger(e.target.value)}
                    step={props.step}
                    class="right-thumb absolute pointer-events-none appearance-none opacity-0 z-20 h-2 w-full cursor-pointer" />

                <div class="relative z-10 h-2">

                    <div class="absolute z-10 left-0 right-0 bottom-0 top-0 rounded-md bg-gray-400"></div>

                    <div class="absolute z-20 top-0 bottom-0 bg-primary" style={{ right: `calc(${max.thumb}% - (${max.thumb * 0.24}px))`, left: `calc(${min.thumb}% - (${min.thumb * 0.24}px))` }}></div>

                    <div class="absolute z-30 w-6 h-6 top-0 left-0 bg-secondary rounded-full -mt-2 -ml-3" style={{ left: `calc(${min.thumb}% - (${min.thumb * 0.24}px))` }}></div>

                    <div class="absolute z-30 w-6 h-6 top-0 right-0 bg-secondary rounded-full -mt-2 -mr-3" style={{ right: `calc(${max.thumb}% - (${max.thumb * 0.24}px))` }}></div>

                </div>
            </div>

            <div class="flex justify-between items-center py-5">
                <div class="w-32">
                    <Number value={min.value} onChange={minTrigger} unit="mm" />
                </div>
                <div class="w-32">
                    <Number value={max.value} onChange={maxTrigger} unit="mm" />
                </div>
            </div>
        </div>
    )
};

export default Range
