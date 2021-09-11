import { useToast } from '../utils/ToastProvider';

const Toast = () => {
    const { toast: {current: toast}} = useToast();

    return (
        toast ?
            <div class="w-full bottom-0 flex items-center fixed z-50">
                <div class={`
                mx-auto h-auto m-5 p-4 w-auto rounded text-white
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