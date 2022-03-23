import { Chart as Chartjs, LinearScale, CategoryScale, LineController, LineElement, PointElement} from 'chart.js';
Chartjs.register(LinearScale,CategoryScale,LineController,PointElement,LineElement);
import { useEffect, useRef, useState } from 'preact/hooks'

const config = {
    type: 'line',
    data: {
        labels: Array.from({ length: 50 }, (_, i) => i + "s"),
        datasets: [
            {
                data: [],
                borderColor: [
                    'rgba(113, 170, 204, 1)',
                ],
                borderWidth: 3,
                yAxisID: 'y',
                pointRadius: 0,
            },
            /*
            // TODO: Implement load-display
            {
                data: [],
                borderColor: [
                    'rgba(255, 99, 132, 1)',
                ],
                borderWidth: 3,
                yAxisID: 'y1',
                pointRadius: 0,
                pointHitRadius: 0
            },
            */
        ]
    },
    options: {
        maintainAspectRatio: false,
        responsive: true,
        interaction: {
            mode: 'index',
            intersect: false,
        },
        stacked: false,
        scales: {
            y: {
                type: 'linear',
                display: true,
                position: 'left',
                title: {
                    display: true,
                    text: 'RPM'
                },
            },
            /*
            // TODO: Implement load-display
            */
            y1: {
                type: 'linear',
                display: false,
                position: 'right',
                title: {
                    display: true,
                    text: 'Load'
                },
                // grid line settings
                grid: {
                    drawOnChartArea: false, // only want the grid lines for one axis to show up
                },
            },
        },
        animation: {
            duration: 0
        }
    },
};

/**
 * Display of motor-related data (like rotations) on a chart
 * Used by Stepper
 */
const Chart = ({ data }) => {
    const chartContainer = useRef(null);
    const [chartInstance, setChartInstance] = useState(null);

    useEffect(() => {
        if (!chartInstance) return;
        chartInstance.data.datasets[0].data = data;
        chartInstance.update();
    }, [data]);

    useEffect(() => {
        if (!chartContainer || !chartContainer.current) return;
        const newChartInstance = new Chartjs(chartContainer.current, config);
        setChartInstance(newChartInstance);
    }, [chartContainer]);

    return (
        <canvas ref={chartContainer}></canvas>
    )
}

export default Chart
