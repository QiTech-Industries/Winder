import Label from "./Label"

const Switch = ({on, onChange, left, right}) => {
    return (
        <div class='mb-6'>
            <Label label="AP active" tooltip="Enable or disable the own wifi network of the winder." />
            <div class="flex justify-center items-center text-white cursor-pointer" onClick={e => onChange(!on)}>
                <span class="">
                    {left}
                </span>
                <div class={`
                        w-14 h-7 flex items-center rounded-full mx-3 my-2 px-1
                        ${on
                        ? "bg-green-700"
                        : "bg-gray-400"
                    }
                        `}>
                    <div class={`
                            bg-white w-5 h-5 rounded-full shadow-md transform duration-150
                            ${on
                            ? "translate-x-7"
                            : null
                        }
                            `}></div>
                </div>
                <span class="">
                    {right}
                </span>
            </div >
        </div>
    )
}

export default Switch
