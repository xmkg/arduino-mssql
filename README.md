arduino-mssql
=============

what is this?
-------------
A TDS 9.0 implementation for Arduino (using UIPEthernet or Ethernet), which I've made back in days (very obsolete). 

why you did this?
-------------
At first, this was a bet between my friend and I back in college, he was working on a project involving Arduino, which he needed to access MSSQL databases later on. He throughy searched the web, but could not find any implementations regarding connecting MSSQL via Arduino. (only MYSQL drivers was available back then). Then he mentioned it to me and claimed that this could not be done, and I said "We'll see about that", so there we are. Although very obsolete, it is a basic implementation which you can use to run queries like "SELECT, INSERT, UPDATE and DELETE". I did not push it to it's limits though. 

how can I use it?
-------------
Take a look into `sqlard-test.cpp` file. It might give you an idea.

does it work?
-------------
Well, hard to say, it's been a long time since I last time mess with it. Back then, I successfully connected to MSSQL 2014-2016 databases with Arduino Nano and Arduino Mega. Currently, I do not have an environment to test it, but it is on my list. Therefore, the answer is 
`worth a shot`.

limitations
-------------
The main constraint was the memory size of the arduino, which was prevented me to retrieve query results for tables with many columns on arduino nano (which has only 2kb of memory). And bear in mind that SQL strings are wide-character, which means each character takes 2 bytes of space, queries, strings in result sets will take `twice as more` space. 

license
-------------
This project is released under MIT license.
