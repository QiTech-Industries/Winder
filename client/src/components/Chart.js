import { Chart as Chartjs, LinearScale, BarElement, BarController, CategoryScale, LineController, LineElement, PointElement, Decimation, Filler, Legend, Title, Tooltip } from 'chart.js';
Chartjs.register(
    LinearScale,
    BarElement,
    BarController,
    CategoryScale,
    Decimation,
    LineController,
    PointElement,
    LineElement
);
import { useEffect, useRef, useState } from 'preact/hooks'

const config = {
    type: 'line',
    data: {
        labels: Array.from({length: 50}, (_, i) => i + "s"),
        datasets: [
            {
                data: [],
                borderColor: [
                    'rgba(75, 192, 192, 1)',
                ],
                borderWidth: 3,
                yAxisID: 'y',
                pointRadius: 0,
            },
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
        ]
    },
    options: {
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
                    text: 'RPS'
                },
            },
            y1: {
                type: 'linear',
                display: true,
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

const Chart = ({data}) => {
    const chartContainer = useRef(null);
    const [chartInstance, setChartInstance] = useState(null);

    useEffect(() => {
        if(!chartInstance) return;
        chartInstance.data.datasets[0].data = data;
        chartInstance.update();
    }, [data[0], chartInstance]);

    useEffect(() => {
        if (!chartContainer || !chartContainer.current) return;
        const newChartInstance = new Chartjs(chartContainer.current, config);
        setChartInstance(newChartInstance);
    }, [chartContainer]);

    return (
        <canvas ref={chartContainer} height="260" class="h-52 w-full"></canvas>
    )
}

export default Chart
