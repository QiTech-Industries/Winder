import { useState } from "preact/hooks"
import Chart from "./Chart";
import { useStats } from "../utils/StatsProvider";

/**
 * Displays the current state or chart of speed of a stepper motor
 * @param name Name of the motor to be displayed as a title
 * @param state State of the motor
 */
const Stepper = ({ name, state }) => {
    const [showDiagram, setShowDiagram] = useState(false);
    const stats = useStats(state => state.stats)
    const charts = useStats(state => state.charts)

    const status = stats[state];
    let rpm = status.r;
    if (Math.abs(status.r) > 100)
        rpm = status.r.toFixed(0);
    else
        rpm = status.r.toFixed(1);
    const active = status.a == 0 ? false : true;

    return (
        <div class="w-full py-4">
            <div class="rounded-t-lg bg-primary w-48 right-0 px-2 py-1 text-center font-bold z-10 h-8">{name}</div>
            <div class={`bg-white rounded-b-lg rounded-tr-lg text-center h-64 relative`}>
                {
                    showDiagram
                        ?
                        <Chart data={charts[state]} />
                        :
                        <>
                            <div class="p-4 h-full">
                                <svg width="100%" height="100%" viewBox="0 0 309 189" version="1.1" xmlns="http://www.w3.org/2000/svg" style="fill-rule:evenodd;clip-rule:evenodd;stroke-linejoin:round;stroke-miterlimit:2;"><g id="Ebene_2"><g id="Ebene_1-2"><path id="Indicator" d="M236.39,148.64l-0,-109.04l9.53,-0l3.14,3.33l0,40.74l54.89,-0l0,20.9l-54.04,-0l0,39.34l-2.85,4.73l-10.67,-0Z" style="fill-rule:nonzero;" class={`${active ? "text-green-700" : "text-red-700"} fill-current`} /><path d="M243.75,152.14l-10.86,-0l0,-116.04l10.86,-0c5.16,0.005 9.405,4.25 9.41,9.41l0,97.22c-0.005,5.16 -4.25,9.405 -9.41,9.41Zm-3.86,-7l3.86,-0c1.322,-0 2.41,-1.088 2.41,-2.41l0,-97.22c-0,-1.322 -1.088,-2.41 -2.41,-2.41l-3.86,-0l0,102.04Z" style="fill-rule:nonzero;" /><path d="M300.87,108.07l-54.49,-0l0,-27.91l54.49,-0c4.416,0.011 8.045,3.644 8.05,8.06l0,11.78c0,0.003 0,0.007 0,0.01c0,4.418 -3.632,8.055 -8.05,8.06Zm-47.49,-7l47.49,-0c0.017,0.001 0.033,0.001 0.05,0.001c0.549,0 1,-0.451 1,-1c0,-0 0,-0.001 0,-0.001l0,-11.85c0,-0.578 -0.472,-1.055 -1.05,-1.06l-47.49,-0l0,13.91Z" style="fill-rule:nonzero;" /><g id="Cable"><rect x="0" y="74.62" width="17.53" height="5.38" /><rect x="0" y="85.4" width="17.53" height="5.37" /><rect x="0" y="95.5" width="17.53" height="5.37" /><rect x="0" y="105.94" width="17.53" height="5.38" /></g><path id="Plug" d="M25.36,67.63l9.64,0l0,53l-9.6,0c-2.107,0 -3.84,-1.733 -3.84,-3.84l0,-45.32c-0,-2.092 1.709,-3.818 3.8,-3.84Z" style="fill-rule:nonzero;" /><g id="Main"><path d="M71.47,188.24l-29.79,0c-4.804,-0.005 -8.755,-3.956 -8.76,-8.76l0,-170.73c0.011,-4.8 3.96,-8.745 8.76,-8.75l29.79,-0l0,188.24Zm-29.79,-181.24c-0.962,-0 -1.755,0.788 -1.76,1.75l0,170.73c0,0.966 0.794,1.76 1.76,1.76l22.79,-0l0,-174.24l-22.79,0Z" style="fill-rule:nonzero;" /><path d="M230.1,188.24l-29.79,0l0,-188.24l29.79,0c4.8,0.005 8.749,3.95 8.76,8.75l0,170.73c-0.005,4.804 -3.956,8.755 -8.76,8.76Zm-22.79,-7l22.79,0c0.966,0 1.76,-0.794 1.76,-1.76l0,-170.73c-0.005,-0.962 -0.798,-1.75 -1.76,-1.75l-22.79,0l0,174.24Z" style="fill-rule:nonzero;" /><path d="M207.31,182.46l-142.84,0l0,-176.69l142.84,-0l0,176.69Zm-135.84,-7l128.84,0l0,-162.69l-128.84,-0l0,162.69Z" style="fill-rule:nonzero;" /></g><g id="RPM"><text x="106.85px" y="87.44px" style="font-family:'Nunito', 'Arial', sans-serif;font-size:26.19px;">RPM</text><text x="134px" y="118.86px" text-anchor="middle" style="font-family:'Nunito', 'Arial', sans-serif;font-size:26.19px;">{rpm != 0 ? rpm : "0.000"}</text></g><g id="Lines_B"><path d="M90,182.22c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-20.3c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,0 4.5,2.031 4.5,4.5l0,20.3c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M112.66,182.22c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-20.3c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,0 4.5,2.031 4.5,4.5l0,20.3c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M135.29,182.22c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-20.3c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,0 4.5,2.031 4.5,4.5l0,20.3c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M157.92,182.22c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l-0,-20.3c-0,-2.469 2.031,-4.5 4.5,-4.5c2.469,0 4.5,2.031 4.5,4.5l-0,20.3c-0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M180.55,182.22c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-20.3c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,0 4.5,2.031 4.5,4.5l0,20.3c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /></g><g id="Lines_T"><path d="M90,35.33c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-19c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,-0 4.5,2.031 4.5,4.5l0,19c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M112.66,35.33c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-19c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,-0 4.5,2.031 4.5,4.5l0,19c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M135.29,35.33c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-19c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,-0 4.5,2.031 4.5,4.5l0,19c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M157.92,35.33c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l-0,-19c-0,-2.469 2.031,-4.5 4.5,-4.5c2.469,-0 4.5,2.031 4.5,4.5l-0,19c-0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /><path d="M180.55,35.33c-2.467,-0.005 -4.495,-2.033 -4.5,-4.5l0,-19c0,-2.469 2.031,-4.5 4.5,-4.5c2.469,-0 4.5,2.031 4.5,4.5l0,19c0,2.469 -2.031,4.5 -4.5,4.5Z" style="fill-rule:nonzero;" /></g></g></g></svg>
                            </div>
                        </>

                }
                <div class="rounded-lg h-10 w-10 bottom-2 right-2 absolute cursor-pointer hover:scale-110 transform text-2xl pt-2 text-primary bg-ternary" onClick={e => setShowDiagram(!showDiagram)}>
                    {
                        showDiagram
                            ? <div class="icon-object" />
                            : <div class="icon-chart" />
                    }
                </div>
            </div>
        </div>
    )
}

export default Stepper
