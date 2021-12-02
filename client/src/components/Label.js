import Tooltip from "./Tooltip"

const Label = ({ label, tooltip }) => {
    return (
        label || tooltip
            ? <label class='block tracking-wide text-md mb-2 text-left'>
                {label}
                {
                    tooltip
                        ? <Tooltip text={tooltip}>
                            <i class="icon-question text-sm cursor-pointer text-secondary"></i>
                        </Tooltip>
                        : null
                }
            </label>
            : null
    )
}

export default Label
