import { createContext } from 'preact';
import { useContext, useState } from 'preact/hooks'

const ModalContext = createContext();

export const useModal = () => {
    return useContext(ModalContext);
}

export const ModalProvider = ({ children }) => {
    const [modalState, setModalState] = useState(
        { title: null, children: null, hidden: true }
    );

    const modal = {
        state: modalState,
        hide: () => {
            setModalState({ ...modalState, hidden: true })
            document.querySelector("body").style.overflow = 'auto'
        },
        show: (title, children) => {
            setModalState({ title, children, hidden: false })
            document.querySelector("body").style.overflow = 'hidden'
        }
    }

    return (
        <ModalContext.Provider value={modal}>
            {children}
        </ModalContext.Provider>
    )
}