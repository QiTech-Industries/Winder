import Tooltip from "./Tooltip"

const Label = ({label, tooltip}) => {
    return (
        <label class='block tracking-wide text-md mb-2 text-left'>
        {label}
        {
            tooltip
                ? <Tooltip text={tooltip}>
                    <i class="icon-question text-sm cursor-pointer"></i>
                </Tooltip>
                : null
        }
    </label>
    )
}

export default Label
