import { createContext } from 'preact';
import { useContext, useState, useReducer, useEffect } from 'preact/hooks'

const ToastContext = createContext();

export const useToast = () => {
    return useContext(ToastContext);
}

export const ToastProvider = ({ children }) => {
    const reducer = (state, action) => {
        switch (action.type) {
            case 'add':
                if (!state.some(object => JSON.stringify(object) === JSON.stringify(action.toast))) {
                    if (state.length === 0) {
                        clearInterval(timer);
                        setTimer(setInterval(() => setToastQueue({ type: "remove" }), showForMicros));
                    }
                    return [...state, action.toast];
                }
                return state;
            case 'remove':
                if (state.length === 1) {
                    clearInterval(timer);
                }
                return state.slice(1);
            default:
                throw new Error();
        }
    }

    const showForMicros = 5e3;
    const [toastQueue, setToastQueue] = useReducer(reducer, []);
    const [timer, setTimer] = useState();

    useEffect(() => {
        return () => clearInterval(timer);
    }, []);

    const setToast = type => message => {
        setToastQueue({ type: "add", toast: { message, type } });
    };

    const toast = {
        error: setToast("error"),
        success: setToast("success"),
        warning: setToast("warning"),
        info: setToast("info"),
        current: toastQueue ? toastQueue[0] : null,
    }

    return (
        <ToastContext.Provider value={{ toast }}>
            {children}
        </ToastContext.Provider>
    )
}