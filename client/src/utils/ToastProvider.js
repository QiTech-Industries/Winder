import create from 'zustand'

const timeout = (ms) => { //pass a time in milliseconds to this function
    return new Promise(resolve => setTimeout(resolve, ms));
}
const showForMs = 3000;

export const useToast = create((set, get) => ({
    addToast: (type, message) => {
        const nextToast = { message, type };
        set(prev => ({ queue: [...prev.queue, nextToast] }));
        if (!get().toast) get().showNext();
    },
    showNext: () => {
        const queue = get().queue;
        set(prev => ({ toast: queue.length ? queue[0] : null, queue: prev.queue.slice(1) }))
        if(!queue.length) return;
        timeout(showForMs).then(() => {
            get().showNext();
        });
    },
    queue: [],
    toast: null
}));