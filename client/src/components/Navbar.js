import Link from './Link'

const links = [
    { text: "Connect", to: "/connect" },
    { text: "Control", to: "/control" },
    { text: "Configure", to: "/configure" },
];

const Navbar = () => {
    return (
        <div class="flex flex-col">
            <nav class="flex items-center justify-between bg-white h-16 border-b-2">

                <a href="/">
                    <img
                        class="w-16 ml-1"
                        src="assets/icon.png"
                        alt="Industries Icon"
                    />
                </a>

                <h1 class="text-gray-700 font-bold hidden md:block">JARVIS Winder</h1>

                <ul class="w-full flex justify-around max-w-sm">
                    {
                        links.map(link => {
                            return (
                                <Link to={link.to}>{link.text}</Link>
                            );
                        })
                    }
                </ul>

            </nav>
        </div>
    )
}

export default Navbar
