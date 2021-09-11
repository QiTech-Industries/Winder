const Tooltip = ({text, children}) => {
    return (
        <div class='has-tooltip inline mx-2'>
            <span class='tooltip rounded p-1 bg-white transform text-black shadow-xl border border-gray-900 -translate-x-1/2 -translate-y-full text-sm w-60'>{text}</span>
            {children}
        </div>
    )
}

export default Tooltip
