import { createContext } from 'preact';
import { useContext, useEffect } from 'preact/hooks'
import { socket } from "./socket"
import { useToast } from './ToastProvider';
import { useStats } from './StatsProvider';

const SocketContext = createContext();

export const useSocket = () => {
    return useContext(SocketContext);
}

export const SocketProvider = ({ children }) => {
    const addToast = useToast(state => state.addToast)
    const updateStats = useStats(state => state.stats.update)

    useEffect(() => {
        socket.on('error', () => {
            addToast("error", "An error occured while communicating with the Winder.");
        });

        // if the connection opens too fast this callback might not be registered yet
        socket.on('open', () => {
            addToast("success", "Successfully connected to the Winder.");
        });

        socket.on('close', () => {
            addToast("error", "Connection to Winder failed.");
        });

        socket.on('corrupt', () => {
            addToast("error", "Message body could not be read.");
        });

        socket.on('reload', () => {
            addToast("error", "Maximum reconnects reached. Please reload the page manually.");
        });

        socket.on("stats", data => {
            if(!updateStats) return;
            updateStats(data);
        });
    }, []);

    return (
        <SocketContext.Provider value={{ socket }}>
            {children}
        </SocketContext.Provider>
    )
}