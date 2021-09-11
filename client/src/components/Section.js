const Section = ({ title, children }) => {
    return (
        <>
            <h2 class="text-2xl text-white pt-8">{title}</h2>
            <div class="w-full my-6 border-t-2 border-gray-500"></div>
            <div class="px-5 flex">
                {children}
            </div>
        </>
    )
}

export default Section
