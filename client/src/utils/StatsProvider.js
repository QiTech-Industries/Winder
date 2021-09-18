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
        m: "standby",//mode
        w: 0, // total windings
        l: 0, // total length
        t: 0, // time
        e: null//error
    });

    useEffect(() => {
        if (!socket) return;
        socket.on("stats", data => {
            if(stats.m != "winding" && data.m == "winding"){
                data.t = Date.now();
            }
            setStats(data);

            let newHistory = { ...history };
            newHistory.spool.push(data.s.r);
            newHistory.ferrari.push(data.f.r);
            newHistory.puller.push(data.p.r);

            if(newHistory.spool.length > 10){
                newHistory.spool.shift();
            }
            setHistory(newHistory);
        })
    }, []);

    useEffect(() => {
            if(stats.m == "winding"){
                setStats({...data, t: Date.now()});
            }
    }, [stats.m]);

    return (
        <StatsContext.Provider value={{ stats, setStats, history }}>
            {children}
        </StatsContext.Provider>
    )
}