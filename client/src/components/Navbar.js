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
                        class="h-14 pl-6"
                        src="assets/icon.svg"
                        alt="Industries Icon"
                    />
                </a>

                <h1 class="text-gray-700 font-bold">JARVIS Winder</h1>

                <ul class="flex">
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
