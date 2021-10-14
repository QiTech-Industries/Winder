import { createContext } from 'preact';
import { useContext, useEffect } from 'preact/hooks'
import { socket } from "./socket"
import { useToast } from '../utils/ToastProvider';

const SocketContext = createContext();

export const useSocket = () => {
    return useContext(SocketContext);
}

export const SocketProvider = ({ children }) => {
    const { toast } = useToast();

    useEffect(() => {
        socket.on('error', () => {
            toast.error("An error occured while communicating with the Winder.");
        });

        // if the connection opens too fast this callback might not be registered yet
        socket.on('open', () => {
            toast.success("Successfully connected to the Winder.");
        });

        socket.on('close', () => {
            toast.error("Connection to Winder failed.");
        });

        socket.on('corrupt', () => {
            toast.error("Message body could not be read.");
        });

        socket.on('reload', () => {
            toast.error("Maximum reconnects reached. Please reload the page manually.");
        });
    }, []);

    return (
        <SocketContext.Provider value={{ socket }}>
            {children}
        </SocketContext.Provider>
    )
}