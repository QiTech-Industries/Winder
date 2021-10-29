import { useToast } from '../utils/ToastProvider';

const Toast = () => {
    const { toast: {current: toast}} = useToast();

    return (
        toast ?
            <div class="left-1/2 transform -translate-x-1/2 bottom-0 flex items-center fixed z-50">
                <div class={`
                mx-auto mb-0 md:mb-4 w-full m-5 p-4 md:w-auto md:rounded text-white text-center
                ${toast.type === "error"
                        ? "bg-red-700"
                        : (toast.type === "success"
                            ? "bg-green-700"
                            : (toast.type === "warning"
                                ? "bg-primary"
                                : "bg-secondary"))
                    }
                    `}>
                    {toast.message}
                </div>
            </div>
            : null
    )
}

export default Toast