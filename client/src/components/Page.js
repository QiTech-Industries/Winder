const Page = ({ children }) => {
    return (
        <div class={`bg-gray-900 min-h-screen font-sans flex flex-col text-ternary`}>
            {children}
        </div>
    )
}

export default Page
