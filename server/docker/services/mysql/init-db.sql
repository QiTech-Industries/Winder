-- phpMyAdmin SQL Dump
-- version 5.1.1
-- https://www.phpmyadmin.net/
--
-- Host: qi-db
-- Erstellungszeit: 15. Okt 2021 um 01:08
-- Server-Version: 5.7.34
-- PHP-Version: 7.4.20

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Datenbank: `qitech`
--
CREATE DATABASE IF NOT EXISTS `qitech` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE `qitech`;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `devices`
--

CREATE TABLE `devices` (
  `mac` varchar(36) COLLATE utf8mb4_unicode_ci NOT NULL,
  `name` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `update_start` time NOT NULL DEFAULT '00:00:00',
  `update_end` time NOT NULL DEFAULT '23:59:59',
  `current_spiffs` int(11) DEFAULT NULL,
  `current_firmware` int(11) DEFAULT NULL,
  `target_spiffs` int(11) DEFAULT NULL,
  `target_firmware` int(11) DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `status` enum('online','offline','updating_spiffs','updating_firmware') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'online'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `firmware`
--

CREATE TABLE `firmware` (
  `id` int(11) NOT NULL,
  `type` enum('spiffs','firmware') COLLATE utf8mb4_unicode_ci NOT NULL,
  `version` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `path` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `logs`
--

CREATE TABLE `logs` (
  `id` int(11) NOT NULL,
  `mac` varchar(36) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `ip` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `action` enum('invalid','check','register','success') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'invalid',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Indizes der exportierten Tabellen
--

--
-- Indizes für die Tabelle `devices`
--
ALTER TABLE `devices`
  ADD PRIMARY KEY (`mac`),
  ADD KEY `firmware_firmware` (`current_firmware`),
  ADD KEY `firmware_spiffs` (`current_spiffs`),
  ADD KEY `target_firmware` (`target_firmware`),
  ADD KEY `target_spiffs` (`target_spiffs`);

--
-- Indizes für die Tabelle `firmware`
--
ALTER TABLE `firmware`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `type` (`type`,`version`),
  ADD UNIQUE KEY `path` (`path`);

--
-- Indizes für die Tabelle `logs`
--
ALTER TABLE `logs`
  ADD PRIMARY KEY (`id`),
  ADD KEY `mac` (`mac`);

--
-- AUTO_INCREMENT für exportierte Tabellen
--

--
-- AUTO_INCREMENT für Tabelle `firmware`
--
ALTER TABLE `firmware`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT für Tabelle `logs`
--
ALTER TABLE `logs`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- Constraints der exportierten Tabellen
--

--
-- Constraints der Tabelle `devices`
--
ALTER TABLE `devices`
  ADD CONSTRAINT `devices_ibfk_1` FOREIGN KEY (`current_firmware`) REFERENCES `firmware` (`id`),
  ADD CONSTRAINT `devices_ibfk_2` FOREIGN KEY (`current_spiffs`) REFERENCES `firmware` (`id`),
  ADD CONSTRAINT `devices_ibfk_3` FOREIGN KEY (`target_firmware`) REFERENCES `firmware` (`id`),
  ADD CONSTRAINT `devices_ibfk_4` FOREIGN KEY (`target_spiffs`) REFERENCES `firmware` (`id`);

--
-- Constraints der Tabelle `logs`
--
ALTER TABLE `logs`
  ADD CONSTRAINT `logs_ibfk_1` FOREIGN KEY (`mac`) REFERENCES `devices` (`mac`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
