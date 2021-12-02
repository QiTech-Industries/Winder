const Section = ({ title, children }) => {
    return (
        <>
            <h2 class="text-2xl py-3 md:py-5 text-center md:text-left text-primary">{title}</h2>
            <div class="w-full mb-6 border-t-2 border-ternary"></div>
            <div class="px-4 flex flex-wrap md:flex-nowrap md:space-x-4">
                {children}
            </div>
        </>
    )
}

export default Section
