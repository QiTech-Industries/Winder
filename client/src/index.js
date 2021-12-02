import './style';
import Navbar from './components/Navbar'
import Router from 'preact-router';
import Wifi from "./components/Wifi"
import Winder from "./components/Winder"
import Config from './components/Config';
import Welcome from "./components/Welcome"
import Toast from './components/Toast';
import Modal from './components/Modal';
import Page from './components/Page';
import { SocketProvider } from './utils/SocketProvider'
import { ConfigProvider } from './utils/ConfigProvider'
import { ModalProvider } from './utils/ModalProvider'

export default function App() {
	return (
		<ModalProvider>
			<SocketProvider>
				<ConfigProvider>
					<Page>
						<Navbar />
						<Router>
							<Welcome path="/" />
							<Wifi path="connect" />
							<Winder path="control" />
							<Config path="configure" />
						</Router>
						<Toast />
						<Modal />
					</Page>
				</ConfigProvider>
			</SocketProvider>
		</ModalProvider>
	);
}
