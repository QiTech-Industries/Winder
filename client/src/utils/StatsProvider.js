import { createContext } from 'preact';
import { useContext, useEffect, useState, useRef } from 'preact/hooks'
import { useSocket } from './SocketProvider';

const StatsContext = createContext();

export const useStats = () => {
    return useContext(StatsContext);
}

export const StatsProvider = ({ children }) => {
    const { socket } = useSocket();
    const [charts, setCharts] = useState({
        p: [],
        s: [],
        f: []
    });

    const stats = useRef({
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
            data.t = stats.current.t;
            stats.current = data;
        });
    }, []);

    useEffect(() => {
        const interval = setInterval(() => {
            // Update chart data every second
            const newCharts = {};

            newCharts.p = [stats.current.p.r, ...charts.p].slice(0, 50);
            newCharts.f = [stats.current.f.r, ...charts.f].slice(0, 50);
            newCharts.s = [stats.current.s.r, ...charts.s].slice(0, 50);

            setCharts(newCharts);

            // Increment timer while winding
            if (stats.current.m == "winding") {
                stats.current.t++;
            }
            // Reset timer if windings are reset
            else if (stats.current.w == 0) {
                stats.current.t = 0;
            }
        }, 1000);
        return () => { clearInterval(interval) };
    }, [charts])

    return (
        <StatsContext.Provider value={{ stats: stats.current, charts }}>
            {children}
        </StatsContext.Provider>
    )
}