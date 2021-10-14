import { Link as PreactLink } from 'preact-router/match';

const Link = ({ to, children }) => {
    return (
        <li>
            <PreactLink href={to} activeClassName="bg-gray-200" class="hover:bg-gray-300 focus:bg-gray-300 focus:ring-2 focus:ring-gray-500 focus:outline-none w-full h-full transition duration-200 p-2 cursor-pointer rounded-lg focus:ring-opacity-50 ring-inset">
                {children}
            </PreactLink>
        </li>
    )
}

export default Link
