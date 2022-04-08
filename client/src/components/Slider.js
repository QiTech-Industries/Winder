import { useState, useEffect } from "preact/hooks"
import Number from "./Number"

/**
 * Slider-Input combo for the speed-input (in meters per minute)
 * Used in Control-view
 * @param {*} props
 */
const Range = (props) => {

    const minRange = props.min;
    const maxRange = props.max;

    /**
     * Checks (and fixes) new input-values to fit within the specified range of valid values
     * @param {Number} input new value
     */
    const minTrigger = (input = props.minThumb) => {
        let value = input < minRange ? minRange : (input > maxRange ? maxRange : input);
        value = Math.round(value * 10) / 10; // Get rid of those odd float-glitches
        props.onChange(value);
    };

    /**
     * Calculates the thumb-value for displaying the bar correctly
     * @returns {Number}
     */
    const calcThumb = () => {
        let input = props.minThumb;
        let value = input < minRange ? minRange : (input > maxRange ? maxRange : input);
        let thumb = ((value - minRange) / (maxRange - minRange)) * 100;
        return thumb;
    };

    return (
        <div class="flex items-center space-x-2">
        <div class="relative w-full">
            <div>
                <input type="range"
                    min={minRange}
                    max={maxRange}
                    value={props.minThumb}
                    onInput={e => minTrigger(e.target.value)}
                    step={props.step}
                    class="absolute pointer-events-none appearance-none z-20 h-2 w-full cursor-pointer left-0 opacity-0" />

                <div class="relative z-10 h-2">

                    <div class="absolute z-10 left-0 right-0 bottom-0 top-0 rounded-md bg-gray-400"></div>

                    <div class="absolute z-20 top-0 bottom-0 bg-primary rounded-full" style={{ left: 0, width: `${calcThumb()}%`}}></div>

                    <div class="absolute z-30 w-6 h-6 top-0 right-0 rounded-full -mt-2 bg-ternary" style={{ left: `calc(${calcThumb()}% - (${calcThumb() * 0.24}px))`}}></div>

                </div>
            </div>
        </div>
        <div class="w-32">
        <Number value={props.minThumb} onChange={minTrigger} step={props.step} unit="m/min" />
        </div>
        </div>
    )
};

export default Range
