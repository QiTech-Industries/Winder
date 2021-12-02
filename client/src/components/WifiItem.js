import Button from "./Button"
import WifiIcon from "./WifiIcon"
import Loading from "./Loading"

const WifiItem = ({ name, strength, type, setShowInput, showInput, connecting, setConnecting, setPassword, connected }) => {
    return (
        <div class="w-full h-20 my-2">
            <Button onClick={setShowInput} custom={connected ? "border-b-8 border-green-700" : null}>
                {
                    showInput
                        ?
                        <form class="flex justify-between items-center">
                            <div class="mx-1 text-2xl">
                            <input onInput={e => setPassword(e.target.value)} class="w-full transition duration-200 p-2 h-10 bg-transparent border-b border-ternary focus:outline-none focus:border-b-2" type="text" placeholder="Password" />
                            </div>
                            <div class="w-1/3">
                                {
                                    connecting
                                        ? <Button secondary disabled><Loading className="text-2xl"/></Button>
                                        : <Button type="submit" secondary onClick={setConnecting}>Connect</Button>
                                }
                            </div>
                        </form>
                        :
                        <div class="flex justify-between items-center">
                            <div class="mx-2 text-2xl">
                                {name}
                            </div>
                            <div class="w-1/4">
                                <div>
                                    <WifiIcon strength={strength} />
                                </div>
                                <div>
                                    {type}
                                </div>
                            </div>
                        </div>
                }
            </Button>
        </div>
    )
}

export default WifiItem
