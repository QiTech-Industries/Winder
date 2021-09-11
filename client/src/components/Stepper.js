import { useState } from "preact/hooks"
import Chart from "./Chart";

const Stepper = ({ rpm = 0, name, active, data }) => {
    const [showDiagram, setShowDiagram] = useState(false);
    rpm = rpm.toFixed(1);
    active = active == 0 ? false : true;

    return (
        <div class={`bg-white py-4 ${showDiagram ? "" : "px-6"} rounded-xl w-72 text-center mt-8 m-4 relative`}>
            {
                showDiagram
                    ?
                    <div class="w-full h-full">
                        <Chart data={data} />
                    </div>
                    :
                    <>
                        <h3 class="text-3xl">{name}</h3>
                        <div class="relative py-2 mx-2">
                            <svg width="100%" height="100%" viewBox="0 0 194 128" version="1.1" xmlns="http://www.w3.org/2000/svg" style="fill-rule:evenodd;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;stroke-miterlimit:1.5;"><g><rect x="156.077" y="56.808" width="37.021" height="14.043" style="fill:rgb(214,208,204);" /><path d="M157.13,127.659l-0,-127.659l-139.149,0l-0,127.659l139.149,0Z" style="fill:rgb(11,0,0);" /><g><path d="M17.062,81.747c0.507,-0 0.919,-0.412 0.919,-0.919l-0,-33.997c-0,-0.507 -0.412,-0.919 -0.919,-0.919l-7.349,0c-0.507,0 -0.918,0.412 -0.918,0.919l-0,33.997c-0,0.507 0.411,0.919 0.918,0.919l7.349,-0Z" /><path d="M8.795,68.87l-0,-2.541l-8.795,-0l0,2.541l8.795,-0Z" style="fill:rgb(0,219,19);" /><path d="M8.795,75.175l-0,-2.541l-8.795,-0l0,2.541l8.795,0Z" style="fill:rgb(255,63,36);" /><path d="M8.795,61.742l-0,-2.541l-8.795,-0l0,2.541l8.795,0Z" style="fill:rgb(255,217,18);" /><path d="M8.795,55.026l-0,-2.542l-8.795,0l0,2.542l8.795,-0Z" style="fill:rgb(0,118,219);" /></g><path d="M157.13,127.659l-0,-127.659l-14.043,0l0,127.659l14.043,0Z" style="fill:rgb(214,208,204);" /><path d="M32.023,127.659l0,-127.659l-14.042,0l-0,127.659l14.042,0Z" style="fill:rgb(214,208,204);" /><path d="M59.039,108.848c-0,6.243 -5.069,11.312 -11.312,11.312c-6.244,-0 -11.312,-5.069 -11.312,-11.312c-0,-6.244 5.068,-11.312 11.312,-11.312c6.243,-0 11.312,5.068 11.312,11.312Z" class={`${active || rpm > 0 ? "text-green-500" : "text-red-500"} fill-current`} /></g><rect x="35.866" y="31.445" width="103.999" height="39.406" style="fill:white;stroke:rgb(255,0,0);stroke-width:2.5px;" /><text x="40.637px" y="57.59px" style="font-family:'ArialMT', 'Arial', sans-serif;font-size:18px;fill:rgb(255,0,0);">RPM</text><text x="85.973px" y="58.213px" style="font-family:'Arial-BoldMT', 'Arial', sans-serif;font-weight:700;font-size:20px;fill:rgb(255,0,0);">{rpm > 0 ? rpm : "0.000"}</text></svg>
                        </div>
                    </>

            }
            <div class="bg-gray-400 rounded-tl-xl rounded-br-xl h-10 w-10 bottom-0 right-0 absolute cursor-pointer hover:scale-110 transform text-2xl pt-2" onClick={e => setShowDiagram(!showDiagram)}>
                {
                    showDiagram
                        ? <div class="icon-object" />
                        : <div class="icon-chart" />
                }
            </div>
        </div>
    )
}

export default Stepper
