import { useModal } from "../utils/ModalProvider";

/**
 * Display for messages to the user, that need to be closed by the user (such as the changelog)
 */
const Modal = () => {
    const modal = useModal();

    return (
        modal.state.hidden
            ?
            null :
            <div class="fixed h-screen top-0 left-0 w-full z-10 flex justify-center items-center">
                <div onClick={e => modal.hide()} class="fixed inset-0 backdrop-filter backdrop-blur-2xl"></div> {/* only close on backdrop click */}
                <div class="flex flex-col relative w-full h-full md:w-3/5 md:h-auto text-left bg-white mx-auto rounded-lg shadow-lg overflow-hidden">
                    <div class="flex justify-between items-center w-full py-4 px-6 bg-gray-400">
                        <p class="text-2xl font-bold text-black">{modal.state.title}</p>
                        <div onClick={e => modal.hide()} class="rounded-lg h-10 w-10 cursor-pointer hover:scale-110 transform text-2xl pt-2 text-primary bg-ternary text-center">
                            <div class="icon-close" />
                        </div>
                    </div>
                    <div class="py-4 px-6 text-black flex-grow overflow-y-scroll h-full md:max-h-96">
                        {modal.state.children}
                    </div>
                </div>
            </div>
    )
}

export default Modal
