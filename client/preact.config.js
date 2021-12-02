import preactCliPostCSS from 'preact-cli-postcss';
import envVars from 'preact-cli-plugin-env-vars';

export default (config, env, helpers) => {
	preactCliPostCSS(config, helpers);
	envVars(config, env, helpers);
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