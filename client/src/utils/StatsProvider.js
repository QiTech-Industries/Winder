import { createContext } from 'preact';
import { useContext, useEffect, useState } from 'preact/hooks'
import { useSocket } from './SocketProvider';

const StatsContext = createContext();

export const useStats = () => {
    return useContext(StatsContext);
}

export const StatsProvider = ({ children }) => {
    const { socket } = useSocket();
    const [history, setHistory] = useState({
        spool: [],
        ferrari: [],
        puller: []
    });

    const [stats, setStats] = useState({
        s: {//spool
            r: 0,//rpm
            s: 0,//stall
            a: false,//active
        },
        p: {//puller
            r: 0,
            s: 0,
            a: false,
        },
        f: {//ferrari
            r: 0,
            s: 0,
            a: false,
        },
        m: "off",//mode
        e: null//error
    });

    useEffect(() => {
        if (!socket) return;
        socket.on("stats", data => {
            setStats(data);

            const newHistory = { ...history };
            newHistory.spool.push(data.s.r);
            newHistory.ferrari.push(data.f.r);
            newHistory.puller.push(data.p.r);
            setHistory(newHistory);
        })
    }, []);

    return (
        <StatsContext.Provider value={{ stats, setStats, history }}>
            {children}
        </StatsContext.Provider>
    )
}