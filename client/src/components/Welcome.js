import { useModal } from "../utils/ModalProvider";
import Changelog from "./Changelog";

/**
 * Page of the main view
 */
const Welcome = () => {
    const modal = useModal();

    return (
        <>
            <div class="flex-grow mx-auto w-full h-full text-white px-5 md:w-2/3 py-5 text-xl text-center">
                <div class="flex space-x-2 justify-center flex-wrap">
                    <div class="text-5xl pt-5 hidden md:block">Welcome to the</div>
                    <div class="text-5xl pt-5 md:hidden">Welcome to</div>
                    <img src="/assets/jarvis.svg" class="h-10 mt-6" />
                    <div class="text-5xl pt-5">Winder!</div>
                </div>
                <br></br>
                <img class="w-full md:w-2/3 xl:w-1/2 mx-auto my-10" src="/assets/winder.svg" />
                <div class="bottom-0 flex space-x-6 text-2xl justify-center">
                    <a class="underline" target="_blank" href="https://uploads-ssl.webflow.com/5e4d3f0eab4c8c5b404b8ce5/6148de6b6fed0a3ba5772119_Datenblatt-JARVIS%20Winder.pdf">Winder Datasheet</a>
                    <br></br>
                    <a class="underline" target="_blank" href="https://youtu.be/XvAvS83-MGY">Web-Interface Video Tutorial</a>
                </div>
            </div>
            <div class="flex w-full items-center justify-between bottom-0 pb-5 px-5 mt-6 text-white">
                <div onClick={e => modal.show("Changelog", <Changelog/>)} class="border-white border-4 rounded-full h-16 w-16 text-4xl flex items-center justify-center font-bold cursor-pointer">
                    <p>i</p>
                </div>
                <div class="flex space-x-2 items-center">
                    <p class="hidden md:block">A product by</p>
                    <a href="https://www.qitech.de"><img src="/assets/logo.svg" class="h-12" /></a>
                </div>
            </div>
        </>
    )
}

export default Welcome
