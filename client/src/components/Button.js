const Button = ({ secondary, onClick, children, disabled, submit }) => {
    const handleClick = e => {
        console.log("click");
        e.stopPropagation();
        e.preventDefault();
        if (onClick) onClick(e);
    }

    return (
        <button onClick={handleClick} class={`
        ${secondary
                ?
                "bg-blue-300 hover:bg-blue-400 focus:bg-blue-400 focus:ring-2 focus:ring-blue-600"
                :
                "bg-gray-200 hover:bg-gray-300 focus:bg-gray-300 focus:ring-2 focus:ring-gray-500"
            }
        ${disabled
                ?
                "cursor-not-allowed"
                :
                "cursor-pointer"
            }
        focus:outline-none w-full h-full transition duration-200 p-2 rounded-lg focus:ring-opacity-50 ring-inset`
        }
            disabled={disabled}
        >
            {children}
        </button>
    )
}

export default Button
