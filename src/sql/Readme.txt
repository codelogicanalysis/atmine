This folder sontains 2 .sql files which represent the database (one empty and one filled with Buckwalter dictionaries). in order to have them added to local mysql databases. do the following:
1- sudo mysql
2- > create database atm;
3- > exit;
4- cd (folder containing the atm_filled.sql)
5- sudo mysql atm < atm_filled.sql
--> At this point all information must have been filled into the databases...


