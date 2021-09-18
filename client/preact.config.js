const preactCliPostCSS = require('preact-cli-postcss');

export default (config, env, helpers) => {
	preactCliPostCSS(config, helpers);
	if (env.isProd) {
		config.devtool = false; // disable sourcemaps
	}
	// change name of fonts to make filenames spiffs compatible
	config.module.rules[7] = {
		test: /\.(woff|svg|ttf)$/,
		use: [
			{
				loader: 'file-loader',
				options: {
					name: '[name].[hash:5].[ext]'
				},
			}
		]
	};
	// console.log(config.module.rules);
}

import {
	Chart,
	ArcElement,
	LineElement,
	BarElement,
	PointElement,
	BarController,
	BubbleController,
	DoughnutController,
	LineController,
	PieController,
	PolarAreaController,
	RadarController,
	ScatterController,
	CategoryScale,
	LinearScale,
	LogarithmicScale,
	RadialLinearScale,
	TimeScale,
	TimeSeriesScale,
	Decimation,
	Filler,
	Legend,
	Title,
	Tooltip,
	SubTitle
} from 'chart.js';

Chart.register(
	ArcElement,
	LineElement,
	BarElement,
	PointElement,
	BarController,
	BubbleController,
	DoughnutController,
	LineController,
	PieController,
	PolarAreaController,
	RadarController,
	ScatterController,
	CategoryScale,
	LinearScale,
	LogarithmicScale,
	RadialLinearScale,
	TimeScale,
	TimeSeriesScale,
	Decimation,
	Filler,
	Legend,
	Title,
	Tooltip,
	SubTitle
);