import create from "zustand"

export const useStats = create((set, get) => {
    return {
        charts: {
            p: [],
            s: [],
            f: [],
            update: () => set(prev => {
                return {
                    charts: {
                        ...prev.charts,
                        p: [prev.stats.p.r, ...prev.charts.p].slice(0, 50),
                        s: [prev.stats.s.r, ...prev.charts.s].slice(0, 50),
                        f: [prev.stats.f.r, ...prev.charts.f].slice(0, 50)
                    },
                    stats: {
                        // Increment timer while winding, Reset timer if windings are reset
                        ...prev.stats,
                        t: prev.stats.m == "winding" ?
                            prev.stats.t + 1 :
                            (prev.stats.w == 0 ? 0 : prev.stats.t)
                    }
                }
            }),
        },
        stats: {
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
            e: null, //error
            update: (next) => set(prev => ({stats: {...next, t: prev.stats.t}}))
        }
    }
});

setInterval(() => {
    useStats.getState().charts.update();
}, 1000);