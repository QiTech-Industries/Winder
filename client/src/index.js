import './style';
import Navbar from './components/Navbar'
import Router from 'preact-router';
import Wifi from "./components/Wifi"
import Winder from "./components/Winder"
import Config from './components/Config';
import Welcome from "./components/Welcome"
import Toast from './components/Toast';
import { SocketProvider } from './utils/SocketProvider'
import { ToastProvider } from './utils/ToastProvider'
import { ConfigProvider } from './utils/ConfigProvider'
import { StatsProvider } from './utils/StatsProvider'

export default function App() {
	return (
		<div class="bg-gray-900 min-h-screen font-mono flex flex-col">
			<ToastProvider>
				<SocketProvider>
					<ConfigProvider>
						<StatsProvider>
							<Navbar />
							<Router>
								<Welcome path="/" />
								<Wifi path="connect" />
								<Winder path="control" />
								<Config path="configure" />
							</Router>
							<Toast />
						</StatsProvider>
					</ConfigProvider>
				</SocketProvider>
			</ToastProvider>
		</div>
	);
}
