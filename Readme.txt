Halite: A Free Open-Source BitTorrent client

Version: 0.2.7

WWW: http://www.binarynotions.com/halite.php
Forum: http://www.binarynotions.com/forum.php

Developer: Eóin O'Callaghan

Halite (named after the mineral) is a BitTorrent client based on Arvid Norberg's
excellent libtorrent library from Rasterbar Software. The program also relies on
the Boost libraries and on my own libHTTP for the still to be implemented remote
control interface from Java-enabled (MIDP 1.0) mobile phones.

Features:

While still at an early state of development Halite is a functional BitTorrent 
client. So far its list of features is pretty standard but that is because I
want to ensure it does the basic stuff well.

That said, it supports:

  + Multiple downloads
  + Displays more detailed information for the selected torrent
  + Connection limits both global and per-torrent
  + Transfer rate limits again global and per-torrent
  + Minimize to tray with transfer rate summary
  + Association with ".torrent" files
  + Option to limit the number of instances to one
  + Estimated time remaining indicator

Planned in near future:

Listed here is what is being worked on at the moment; for a more detailed list
see [http://www.binarynotions.com/halite/roadmap.php].

  + Support for ipfilter.dat IP blocking
  + Per-torrent user selected save directory
  + DHT support

Requirements:
Windows 2000 or XP (Halite is written in Unicode)

Known Issues/Problems:
None!

Changes:

 - from v 0.2.6 to 0.2.7:
1) By default Halite will only allow one instance to be launched
2) Halite can be set as the default program for '.torrent' files.
3) ETA indicator for torrents.
4) Minor GUI tweaks.

 - from v 0.2.5 to 0.2.6:
1) Fixed a silly bug (i.e. I should have caught it sooner) whereby exiting 
   Halite from the tray would screw up the window layout next time running it.
2) Added auto-selection to Torrents list to make it more intuitive.

 - from v 0.2 to 0.2.5:
1) The GUI is slightly altered so that it looks consistent and neat for a number
   of visual styles which I was able to test it on.
2) The ability to drag and drop torrent files into the app has been added.
3) It will minimize to the tray instead of task bar and hovering mouse for the
   tray icon displays the current up and down rate.
4) Where previously the various limits boxes didn't validate properly now they 
   do and any invalid input is interpreted as no limit and a nice infinity 
   symbol is displayed.
5) Individual torrent info is saved and loaded when the program is closed and 
   reopened allowing torrent limits to be remembered.
6) If the program has to wait after the user clicking close for any active 
   connections to shutdown down 'cleanly' an optional small dialog is displayed 
   to alert the user that the program is still running.

Subversion:
svn://svn.geekisp.com/halite

In addition you can check out the Trac site (http://trac.geekisp.com/halite) but
I haven't really done much with it...yet.

Thanks:

It can be hard to remember to thank everyone but that is no excuse for leaving
someone out. If I have done so let me know.

  + Arvid Norberg- As the author of libtorrent he has made the single biggest
    contribution to this project.
    http://www.rasterbar.com/products/libtorrent/index.html

  + Christopher Kohloff- The man behind Boost.ASIO a crucial element of
    libtorrent and my own libhttp library.
    http://asio.sourceforge.net/

  + nudone (Nick Pearson)- The excellent artist who designed the icon for
    Halite.
    wtfcody.com / nudsville.com

  + Austin- Very kindly made the Iss install script for Halite.

  + Everyone who has been in touch with me through email or the forum and have 
    shared their ideas and feature requests for the client. They have given the 
    project direction and purpose.
