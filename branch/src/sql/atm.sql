-- MySQL dump 10.11
--
-- Host: localhost    Database: atm
-- ------------------------------------------------------
-- Server version	5.0.67-0ubuntu6

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
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `category` (
  `id` int(11) NOT NULL default '0',
  `name` varchar(40) NOT NULL,
  `type` bit(2) NOT NULL,
  `sources` bigint(20) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

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
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `compatibility_rules` (
  `category_id1` int(11) NOT NULL default '0',
  `category_id2` int(11) NOT NULL default '0',
  `type` bit(3) default NULL,
  `sources` bigint(20) default NULL,
  PRIMARY KEY  (`category_id1`,`category_id2`),
  KEY `category_id2` (`category_id2`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `compatibility_rules`
--

LOCK TABLES `compatibility_rules` WRITE;
/*!40000 ALTER TABLE `compatibility_rules` DISABLE KEYS */;
/*!40000 ALTER TABLE `compatibility_rules` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `prefix`
--

DROP TABLE IF EXISTS `prefix`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `prefix` (
  `id` int(11) NOT NULL default '0',
  `name` char(5) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

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
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `prefix_category` (
  `prefix_id` int(11) NOT NULL default '0',
  `category_id` int(11) NOT NULL default '0',
  `sources` bigint(20) NOT NULL,
  PRIMARY KEY  (`prefix_id`,`category_id`),
  KEY `category_id` (`category_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `prefix_category`
--

LOCK TABLES `prefix_category` WRITE;
/*!40000 ALTER TABLE `prefix_category` DISABLE KEYS */;
/*!40000 ALTER TABLE `prefix_category` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `semantical_category`
--

DROP TABLE IF EXISTS `semantical_category`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `semantical_category` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(256) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `semantical_category`
--

LOCK TABLES `semantical_category` WRITE;
/*!40000 ALTER TABLE `semantical_category` DISABLE KEYS */;
INSERT INTO `semantical_category` VALUES (3,'Ø§Ø³Ù…');
/*!40000 ALTER TABLE `semantical_category` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `source`
--

DROP TABLE IF EXISTS `source`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `source` (
  `id` int(11) NOT NULL auto_increment,
  `description` varchar(1000) default NULL,
  `normalization_process` varchar(1000) default NULL,
  `creator` varchar(50) default NULL,
  `date_start` date default NULL,
  `date_last` date default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `source`
--

LOCK TABLES `source` WRITE;
/*!40000 ALTER TABLE `source` DISABLE KEYS */;
INSERT INTO `source` VALUES (1,'jad',NULL,NULL,NULL,'2009-12-03'),(2,'hamza','','','2009-12-03','2009-12-03'),(3,'hamza','','','2009-12-03','2009-12-03');
/*!40000 ALTER TABLE `source` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `stem`
--

DROP TABLE IF EXISTS `stem`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `stem` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `name` varchar(30) NOT NULL,
  `grammar_stem_id` bigint(20) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `grammar_stem_id` (`grammar_stem_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

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
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `stem_category` (
  `stem_id` bigint(20) NOT NULL default '0',
  `category_id` int(11) NOT NULL default '0',
  `semantical_categories` bigint(20) NOT NULL,
  `sources` bigint(20) default NULL,
  `raw_data` varchar(60) default NULL,
  `description` varchar(200) default NULL,
  PRIMARY KEY  (`stem_id`,`category_id`),
  KEY `category_id` (`category_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

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
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `suffix` (
  `id` int(11) NOT NULL default '0',
  `name` char(5) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

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
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `suffix_category` (
  `suffix_id` int(11) NOT NULL default '0',
  `category_id` int(11) NOT NULL default '0',
  `sources` bigint(20) NOT NULL,
  PRIMARY KEY  (`suffix_id`,`category_id`),
  KEY `category_id` (`category_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

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

-- Dump completed on 2009-12-11 22:38:30
