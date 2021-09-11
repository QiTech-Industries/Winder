const Loading = ({ text, className }) => {
    return (
        <>
            <div class={`h-full ${className}`}>
                <div class="icon-load animate-spin" />
            </div>
            {
                text
                    ? <p class="text-md my-2">{text}</p>
                    : null
            }
        </>
    )
}

export default Loading
