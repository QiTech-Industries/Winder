import { useState } from 'preact/hooks'

const Menu = (props) => {
    const toggleMenu = () => {
        if (slide === 0) setSlide(100);
        else setSlide(0);
    }

    const [slide, setSlide] = useState(100);

    return (
        <div>
            <li onClick={toggleMenu} onMouseEnter={() => props.hover ? setSlide(0) : null} className={`
            rounded-full z-10 absolute h-16 w-16 flex items-center cursor-pointer float-right
            ${
                props.enabled
                ? "bg-green-500"
                : "bg-red-500"
            }
                `}>
                <span className="m-auto font-bold"><i class="icon-power text-4xl"/></span>
            </li>
            <div class="ml-8 overflow-hidden">
                <div style={{ transform: `translateX(-${slide}%)` }} className="bg-primary pl-10 pr-6 h-16 py-2 rounded-r-full transform duration-700">
                    <ul className="flex space-x-2 whitespace-nowrap items-center py-1 h-full">
                        {props.children}
                    </ul>
                </div>
            </div>
        </div>
    )
}

export default Menu