DROP TABLE IF EXISTS `ranking_bg_bomber`;
CREATE TABLE IF NOT EXISTS `ranking_bg_bomber` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_conquest`;
CREATE TABLE IF NOT EXISTS `ranking_bg_conquest` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_inferno`;
CREATE TABLE IF NOT EXISTS `ranking_bg_inferno` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_captureflag`;
CREATE TABLE IF NOT EXISTS `ranking_bg_captureflag` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_deathmatch`;
CREATE TABLE IF NOT EXISTS `ranking_bg_deathmatch` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_stonecontrol`;
CREATE TABLE IF NOT EXISTS `ranking_bg_stonecontrol` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_poringball`;
CREATE TABLE IF NOT EXISTS `ranking_bg_poringball` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_rush`;
CREATE TABLE IF NOT EXISTS `ranking_bg_rush` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_bossnia`;
CREATE TABLE IF NOT EXISTS `ranking_bg_bossnia` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_domination`;
CREATE TABLE IF NOT EXISTS `ranking_bg_domination` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;
DROP TABLE IF EXISTS `ranking_bg_eyeofstorm`;
CREATE TABLE IF NOT EXISTS `ranking_bg_eyeofstorm` (
`Char_ID` int(11) unsigned NOT NULL DEFAULT '0',
`PlayerName` varchar(255) NULL DEFAULT '',
`Kills` varchar(255) NULL DEFAULT '',
`Deaths` varchar(255) NULL DEFAULT '',
`Victory` varchar(255) NULL DEFAULT '',
`Defeat` varchar(255) NULL DEFAULT '',
PRIMARY KEY (`Char_ID`)
) ENGINE=MyISAM;