import { useToast } from '../utils/ToastProvider';

const Toast = () => {
    const { toast: {current: toast}} = useToast();

    return (
        toast ?
            <div class="w-full bottom-0 flex items-center fixed z-50">
                <div class={`
                mx-auto mb-0 md:mb-5 w-full m-5 p-4 md:w-auto md:rounded text-white text-center
                ${toast.type === "error"
                        ? "bg-red-500"
                        : (toast.type === "success"
                            ? "bg-green-500"
                            : (toast.type === "warning"
                                ? "bg-yellow-500"
                                : "bg-blue-500"))
                    }
                    `}>
                    {toast.message}
                </div>
            </div>
            : null
    )
}

export default Toast