-- MySQL dump 10.13  Distrib 5.1.41, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: atm
-- ------------------------------------------------------
-- Server version	5.1.41-3ubuntu12.10

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `category`
--

DROP TABLE IF EXISTS `category`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `category` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(40) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `type` bit(2) NOT NULL,
  `sources` char(16) NOT NULL,
  `abstract` bit(1) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `category`
--

LOCK TABLES `category` WRITE;
/*!40000 ALTER TABLE `category` DISABLE KEYS */;
/*!40000 ALTER TABLE `category` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `compatibility_rules`
--

DROP TABLE IF EXISTS `compatibility_rules`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `compatibility_rules` (
  `category_id1` int(11) NOT NULL,
  `category_id2` int(11) NOT NULL,
  `type` bit(3) NOT NULL,
  `sources` char(16) NOT NULL,
  `resulting_category` int(11) DEFAULT -1,
  `inflections` varchar(250) NOT NULL DEFAULT '',
  PRIMARY KEY (`category_id1`,`category_id2`, `resulting_category`),
  KEY `category_id2` (`category_id2`),
  KEY `resulting_category` (`resulting_category`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `compatibility_rules`
--

LOCK TABLES `compatibility_rules` WRITE;
/*!40000 ALTER TABLE `compatibility_rules` DISABLE KEYS */;
/*!40000 ALTER TABLE `compatibility_rules` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `description`
--

DROP TABLE IF EXISTS `description`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `description` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(250) NOT NULL,
  `type` bit(2) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`,`type`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `description`
--

LOCK TABLES `description` WRITE;
/*!40000 ALTER TABLE `description` DISABLE KEYS */;
/*!40000 ALTER TABLE `description` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `prefix`
--

DROP TABLE IF EXISTS `prefix`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `prefix` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` char(7) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `prefix`
--

LOCK TABLES `prefix` WRITE;
/*!40000 ALTER TABLE `prefix` DISABLE KEYS */;
/*!40000 ALTER TABLE `prefix` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `prefix_category`
--

DROP TABLE IF EXISTS `prefix_category`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `prefix_category` (
  `prefix_id` int(11) NOT NULL,
  `category_id` int(11) NOT NULL,
  `abstract_categories` char(16) NOT NULL,
  `sources` char(16) NOT NULL,
  `raw_data` varchar(60) NOT NULL DEFAULT '',
  `POS` varchar(250) NOT NULL DEFAULT '',
  `description_id` bigint(20) DEFAULT NULL,
  `reverse_description` bit(1) DEFAULT b'0',
  PRIMARY KEY (`prefix_id`,`category_id`,`raw_data`,`description_id`,`POS`),
  KEY `category_id` (`category_id`),
  KEY `description_id` (`description_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `prefix_category`
--

LOCK TABLES `prefix_category` WRITE;
/*!40000 ALTER TABLE `prefix_category` DISABLE KEYS */;
/*!40000 ALTER TABLE `prefix_category` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `source`
--

DROP TABLE IF EXISTS `source`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `source` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `description` varchar(1000) DEFAULT NULL,
  `normalization_process` varchar(1000) DEFAULT NULL,
  `creator` varchar(50) DEFAULT NULL,
  `date_start` date DEFAULT NULL,
  `date_last` date DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `source`
--

LOCK TABLES `source` WRITE;
/*!40000 ALTER TABLE `source` DISABLE KEYS */;
/*!40000 ALTER TABLE `source` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `stem`
--

DROP TABLE IF EXISTS `stem`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `stem` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(30) NOT NULL,
  `grammar_stem_id` bigint(20) DEFAULT NULL,
  `sources` char(16) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `grammar_stem_id` (`grammar_stem_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `stem`
--

LOCK TABLES `stem` WRITE;
/*!40000 ALTER TABLE `stem` DISABLE KEYS */;
/*!40000 ALTER TABLE `stem` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `stem_category`
--

DROP TABLE IF EXISTS `stem_category`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `stem_category` (
  `stem_id` bigint(20) NOT NULL,
  `category_id` int(11) NOT NULL,
  `abstract_categories` char(16) NOT NULL,
  `sources` char(16) NOT NULL,
  `raw_data` varchar(60) NOT NULL DEFAULT '',
  `POS` varchar(250) NOT NULL DEFAULT '',
  `lemma_ID` char(15) DEFAULT NULL,
  `description_id` bigint(20) NOT NULL,
  PRIMARY KEY (`stem_id`,`category_id`,`raw_data`,`description_id`,`POS`),
  KEY `category_id` (`category_id`),
  KEY `description_id` (`description_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `stem_category`
--

LOCK TABLES `stem_category` WRITE;
/*!40000 ALTER TABLE `stem_category` DISABLE KEYS */;
/*!40000 ALTER TABLE `stem_category` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `suffix`
--

DROP TABLE IF EXISTS `suffix`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `suffix` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` char(7) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `suffix`
--

LOCK TABLES `suffix` WRITE;
/*!40000 ALTER TABLE `suffix` DISABLE KEYS */;
/*!40000 ALTER TABLE `suffix` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `suffix_category`
--

DROP TABLE IF EXISTS `suffix_category`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `suffix_category` (
  `suffix_id` int(11) NOT NULL,
  `category_id` int(11) NOT NULL,
  `abstract_categories` char(16) NOT NULL,
  `sources` char(16) NOT NULL,
  `raw_data` varchar(60) NOT NULL DEFAULT '',
  `POS` varchar(250) NOT NULL DEFAULT '',
  `description_id` bigint(20) DEFAULT NULL,
  `reverse_description` bit(1) DEFAULT b'0',
  PRIMARY KEY (`suffix_id`,`category_id`,`raw_data`,`description_id`,`POS`),
  KEY `category_id` (`category_id`),
  KEY `description_id` (`description_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `suffix_category`
--

LOCK TABLES `suffix_category` WRITE;
/*!40000 ALTER TABLE `suffix_category` DISABLE KEYS */;
/*!40000 ALTER TABLE `suffix_category` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-12-08  0:04:14
