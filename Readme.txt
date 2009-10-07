Halite: A Free Open-Source BitTorrent client

Version: 0.3.2 RC released 4th October 2009

WWW:   http://www.binarynotions.com/halite-bittorrent-client 
Forum: http://www.binarynotions.com/forum.php

Developer: Eóin O'Callaghan

Halite (named after the mineral) is a BitTorrent client based on Arvid
Norberg's excellent libtorrent library from Rasterbar Software. The
program also relies on the Boost libraries.

Features:

While still at an early state of development Halite is a functional
BitTorrent client. So far its list of features is pretty standard but
that is because I want to ensure it does the basic stuff well.

That said, it supports:

  + File selection and/or file priority specifications.
  + 'Managed torrent' queue system.
  + Disk cache support.
  + Trackerless torrents (using the Mainline kademlia DHT protocol).
  + IP filtering with eMule style ipfilter.dat import.
  + Protocol Encryption support.
  + Translations of the UI into a number of languages through the help 
    of volunteers. (if you wish to help with a translation then please do)
  + Minimise to tray with transfer rate summary.
  + Full Unicode support through UTF-8 and native Windows wide-char 
    strings.
  + Login support where tracker requires it.
  + Ability to edit trackers specified in the torrent.
  + Transfer rate and connection limits both global and per-torrent.
  + Torrent file creation.
  + Port randomisation and forwarding (UnPlug and Play or NAT-PMP)
  + Compatible with UAC.
  + Shutdown scheduler.

Requirements: Windows 2000, XP, Server 2003, Vista, Server 2008 or Win7*

* I run Win 7 myself but still support is not 100%. Notably ipfilter 
cause the UI to appear to freeze briefly after just starting Halite. 
Also the shutdown scheduler can't actually switch off the PC unless 
started with Admin privileges which isn't necessarily recommended 
unless the feature is really needed.

Important note for people upgrading!

When installing through the MSI file download, Halite will make a
registry entry pointing it to use the local Application Data directory
for all configuration files. Users should manually copy any existing
files from an older Halite version to that directory. The recommended
files and folder to copy are:

Halite.xml
BitTorrent.xml
IPFilter.bin
DHTState.bin
resume/*
torrents/*

These changes were necessary to make Halite fully compatible with User
Account Control. To revert to 'portable mode' whereby all config
information will be saved alongside the executable simply delete that
registry key (HKEY_CURRENT_USER\Software\Halite\path). If you only
ever used the standalone 'zipped' download then this will not be 
necessary.

Changes:

 + from v 0.3.1.8 to 0.3.2 RC:
   1) Peer details exception fixed.
   2) Updated to OpenSSL 0.98k
   3) Updated to Boot 1.40
   4) Updated to libtorrent 0.14.6
      - UPnP bug fixed.
      - fixed bug when setting unlimited upload or download rates for 
        torrents.
      - replaces invalid filename characters with '.'
      - fixed bug where web seeds would not disconnect if being resolved 
        when the torrent was paused.
   5) Contributed language translations updated.
   6) Numerous other small bug fixes and improvements.

 + from v 0.3.1 to 0.3.1.8:
   1) Shutdown scheduler.
   2) Updated to libtorrent 0.14.4
      - Potential libtorrent vulnerability fixed.
   3) Numerous small bug fixes and improvements.

 + from v 0.3.1 to 0.3.1.6:
   1) Queue View Fixed
   2) Sorting fixed.
   3) libtorrent 0.14 integration finished.

 + from v 0.3.1 to 0.3.1.6:
   1) Based on libtorrent 0.14 which brings with it:
      - Disk cache support
      - 'Managed torrent' queue system
      - Improved seeding and choking behavior
      - Resource usage optimizations
      - Many more fixes and improvements 
   (code.rasterbar.com/libtorrent/browser/tags/libtorrent-0_14/ChangeLog)

   2) Optional port randomizations (both main and DHT).
   3) Numerous internal settings are now exposed by the GUI.
   4) Plays nice by Vista/Server 2008 UAC rules. See note above. 
   2) Rewritten XML fileformat (again) with greatly improved robustness.

 + from v 0.3.0.7 to 0.3.1: 
   1) Bug fixes.

 + from v 0.3.0.5 to 0.3.0.7: 
   1) Adds ability to create torrent files.
   2) A lot of small improvements and bug fixes.

 - from v 0.3.0.2 to 0.3.0.5: 
   1) Right-click context menu option to 'Open download folder'.
   2) Right-click context menu option to force a file recheck.
   3) Option to move seeding torrent to another folder.
   4) Fixed a bug with tracker logins.

 + from v 0.3 to 0.3.0.2: 
   Mainly a bugfix release.
   1) Fixed a bug with Tray Icon remaining after window was restored.
   2) Torrent Connection and Transfer setting mapped to incorrect 
      editbox.
   3) Subtle bug where some settings didn't register straight away.

 + from v 0.2.9 to 0.3: 
   1) File selection and/or priority settings.
   2) Protocol Encryption.
   3) New tabbed interface makes better use of screen real estate.
   4) Updated to recent versions of Boost (1.34.1) and libtorrent.
   5) Numerous bug fixes.
   
 + from v 0.2.8 to 0.2.9: 
   1) Full Unicode support!
   2) Rewitten XML fileformat with greatly improved robustness.
   3) New tabbed interface makes better use of screen real estate.
   4) Ability to edit Trackers and set login details.
   5) Comprehensive logging for diagnosing problems.
   6) Windows 2000 and Server 2003 supported fully.
   7) Switched to MSVC 2005 compiler (8.0) for improved reliability
      though at the cost of slightly bigger executables.
   
+ from v 0.2.7 to 0.2.8: 
   1) IP filtering support eMule style ipfilter.dat files.
   2) DHT support, thought it is turned off by default.
   3) Can select alternate save directory for torrents..
   4) New icon!

 + from v 0.2.6 to 0.2.7:
   1) By default Halite will only allow one instance to be launched 
   2) Halite can be set as the default program for '.torrent' files. 
   3) ETA indicator for torrents. 4) Minor GUI tweaks.

 + from v 0.2.5 to 0.2.6: 
   1) Fixed a silly bug (i.e. I should have caught it sooner) whereby
      exiting Halite from the tray would screw up the window layout
      next time running it.
   2) Added auto-selection to Torrents list to make it more intuitive.

 + from v 0.2 to 0.2.5: 
   1) The GUI is slightly altered so that it looks consistent and neat
      for a number of visual styles which I was able to test it on. 
   2) The ability to drag and drop torrent files into the app has been
      added.
   3) It will minimise to the tray instead of task bar and hovering
      mouse for the tray icon displays the current up and down rate. 
   4) Where previously the various limits boxes didn't validate
      properly now they do and any invalid input is interpreted as no
      limit and a nice infinity symbol is displayed. 
   5) Individual torrent info is saved and loaded when the program is
      closed and reopened allowing torrent limits to be remembered.
   6) If the program has to wait after the user clicking close for any
      active connections to shutdown down 'cleanly' an optional small
      dialog is displayed to alert the user that the program is still
      running.

Subversion: svn://svn.geekisp.com/halite

In addition you can check out the Trac site (trac.geekisp.com/halite)
but I haven't really done much with it ...yet.

Thanks:

It can be hard to remember to thank everyone but that is no excuse for
leaving someone out. If I have done so let me know.

  + Arvid Norberg- As the author of libtorrent he has made the single
    biggest contribution to this project.
    http://www.rasterbar.com/products/libtorrent/index.html

  + Christopher Kohloff- The man behind Boost.ASIO a crucial element
    of libtorrent and my own libhttp library.
    http://asio.sourceforge.net/

  + nudone (Nick Pearson)- The excellent artist who designed the icon
    for Halite. 
        wtfcody.com / nudsville.com

  + Tw@in 28 for all the help moderating the forum.

  + Austin; very kindly made the Iss install script for Halite.

  + Everyone who has been in touch with me through email or the forum
    and have shared their ideas and feature requests for the client.
    They have given the project direction and purpose.

  + And of course all the translators:                    

    - devGod – Nederlandse vertaling.
    - MaikelChan - maikelchan88@gmail.com - Traducción en Español de España.
    - Prome – prome.lwaku.info – Avtor slovenskega prevoda.                  
    - Tw@in 28 – twain_28@hotmail.com - Traduttore Italiano.
    - u!^DEV  - germanpg.hacker.to - deutsche Übersetzung.
    - webdr - www.langturk.com - Türkçe çeviri.
    - Čagalj - cagalj@mail.ru - Preveo sve na srpski (latinicu i ćirilicu).
    - AlexKrycek - alexkrycek0@gmail.com - μετάφραση στα ελληνικά.
