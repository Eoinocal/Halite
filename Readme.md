# Halite: A Free Open-Source BitTorrent client

Version: 0.4.0.3 dev 1302 released 20th August 2015

WWW: [http://www.binarynotions.com/halite-bittorrent-client](http://www.binarynotions.com/halite-bittorrent-client) 

## Developer: Eóin O'Callaghan

Halite (named after the mineral) is a BitTorrent client based on Arvid
Norberg's excellent libtorrent library from Rasterbar Software. The
program also relies on the Boost libraries.

## Features:

While still at an early state of development Halite is a functional
BitTorrent client. So far its list of features is pretty standard but
that is because I want to ensure it does the basic stuff well.

That said, it supports:

  + File selection and/or file priority specifications.
  + 'Managed torrent' queue system.
  + Disk cache support.
  + Trackerless torrents (using the Mainline kademlia DHT protocol).
  + Magnet URIs.
  + Super-seeding.
  + uTorrent style multi-tracker announcement. 
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
  + Webseeds

Requirements: 32 bit supports Windows XP and higher
              64 bit supports Vista and higher

Important note for people upgrading!

Old configuration files need to be reset. They will be backup by the 
installer, unless you choose not too.

Changes:

 + from v 0.4.0.2 to 0.4.0.3:

   1. Re-fixed Unicode.
   2. Fixed a bug with gigabyte sizes.
   3. Compiling with VS 2015 community.

 + from v 0.4.0.1 to 0.4.0.2:

   1. Updated to libtorrent 1.05
   1. Fixed a bug with Unicode.
   2. Displayed byte values auto format to KB, MB, etc.
   3. Global setting for specifying defult allocation.
   4. Big Russian translation update. Thanks maxdeepfield!

 + from v 0.4.0 to 0.4.0.1:

   1. Fixed a bug with portable mode.

 + from v 0.3.4 to 0.4.0:

   1. Updated to libtorrent 1.03
      - This is basically the monster change! Full credits to Arvid 
        Norberg here for all the real work.
   3. Added support for Web seeds
   4. Updated to OpenSSL 1.0.1j
   5. Updated to Boot 1.57
   6. Compiled with Visual C++ 2013 
   7. Lots of bug fixes

 + from v 0.3.3 to 0.3.4:

   1. Updated to libtorrent 0.16.6
      - This is basically the monster change! Full credits to Arvid 
        Norberg here for all the real work.
   2. Dropped file renaming and those "hash" folders as they were causing undue trouble.
   3. Added support for LT Trackers
   4. Updated to OpenSSL 1.0.1c
   5. Updated to Boot 1.52
   6. Compiled with Visual C++ 2012 
   
 + from v 0.3.2.1 to 0.3.3.0:

   1. Announce to all trackers.
   2. Magnet URIs support added.
   3. Download complete balloon notification.
   4. Right aligned numeric columns.
   5. Super-seeding.
   6. File renaming (not fully exposed by UI).
   7. Updated to libtorrent 0.15.1
      - brings significant optimizations and bug fixes.
   
 + from v 0.3.2 to 0.3.2.1:
   1. Open folder behaviour changed to open torrent subfolder.
   2. Queue positions correctly restored between sessions.
   3. Right-click menu crash when no tracker selected fixed.
   4. Fixed issue with displayed transfer payloads.

 + from v 0.3.2 RC to 0.3.2:
   1. Firewall exception option added to installer.
   2. 'Remove files and torrent' bug fixed.
   3. Right-click menu crash when no torrents selected fixed.

 + from v 0.3.1.8 to 0.3.2 RC:
   1. Peer details exception fixed.
   2. Updated to OpenSSL 0.98k
   3. Updated to Boot 1.40
   4. Updated to libtorrent 0.14.6
      - UPnP bug fixed.
      - fixed bug when setting unlimited upload or download rates for 
        torrents.
      - replaces invalid filename characters with '.'
      - fixed bug where web seeds would not disconnect if being resolved 
        when the torrent was paused.
   5. Contributed language translations updated.
   6. Numerous other small bug fixes and improvements.

 + from v 0.3.1 to 0.3.1.8:
   1. Shutdown scheduler.
   2. Updated to libtorrent 0.14.4
      - Potential libtorrent vulnerability fixed.
   3. Numerous small bug fixes and improvements.

 + from v 0.3.1 to 0.3.1.6:
   1. Queue View Fixed
   2. Sorting fixed.
   3. libtorrent 0.14 integration finished.

 + from v 0.3.1 to 0.3.1.6:
   1. Based on libtorrent 0.14 which brings with it:
      - Disk cache support
      - 'Managed torrent' queue system
      - Improved seeding and choking behavior
      - Resource usage optimizations
      - Many more fixes and improvements 
   (code.rasterbar.com/libtorrent/browser/tags/libtorrent-0_14/ChangeLog)

   2. Optional port randomizations (both main and DHT).
   3. Numerous internal settings are now exposed by the GUI.
   4. Plays nice by Vista/Server 2008 UAC rules. See note above. 
   2. Rewritten XML fileformat (again) with greatly improved robustness.

 + from v 0.3.0.7 to 0.3.1: 
   1. Bug fixes.

 + from v 0.3.0.5 to 0.3.0.7: 
   1. Adds ability to create torrent files.
   2. A lot of small improvements and bug fixes.

 - from v 0.3.0.2 to 0.3.0.5: 
   1. Right-click context menu option to 'Open download folder'.
   2. Right-click context menu option to force a file recheck.
   3. Option to move seeding torrent to another folder.
   4. Fixed a bug with tracker logins.

 + from v 0.3 to 0.3.0.2: 
   1. Fixed a bug with Tray Icon remaining after window was restored.
   2. Torrent Connection and Transfer setting mapped to incorrect 
      editbox.
   3. Subtle bug where some settings didn't register straight away.

 + from v 0.2.9 to 0.3: 
   1. File selection and/or priority settings.
   2. Protocol Encryption.
   3. New tabbed interface makes better use of screen real estate.
   4. Updated to recent versions of Boost (1.34.1) and libtorrent.
   5. Numerous bug fixes.
   
 + from v 0.2.8 to 0.2.9: 
   1. Full Unicode support!
   2. Rewitten XML fileformat with greatly improved robustness.
   3. New tabbed interface makes better use of screen real estate.
   4. Ability to edit Trackers and set login details.
   5. Comprehensive logging for diagnosing problems.
   6. Windows 2000 and Server 2003 supported fully.
   7. Switched to MSVC 2005 compiler (8.0) for improved reliability
      though at the cost of slightly bigger executables.
   
+ from v 0.2.7 to 0.2.8: 
   1. IP filtering support eMule style ipfilter.dat files.
   2. DHT support, thought it is turned off by default.
   3. Can select alternate save directory for torrents..
   4. New icon!

 + from v 0.2.6 to 0.2.7:
   1. By default Halite will only allow one instance to be launched 
   2. Halite can be set as the default program for '.torrent' files. 
   3. ETA indicator for torrents. 4) Minor GUI tweaks.

 + from v 0.2.5 to 0.2.6: 
   1. Fixed a silly bug (i.e. I should have caught it sooner) whereby
      exiting Halite from the tray would screw up the window layout
      next time running it.
   2. Added auto-selection to Torrents list to make it more intuitive.

 + from v 0.2 to 0.2.5: 
   1. The GUI is slightly altered so that it looks consistent and neat
      for a number of visual styles which I was able to test it on. 
   2. The ability to drag and drop torrent files into the app has been
      added.
   3. It will minimise to the tray instead of task bar and hovering
      mouse for the tray icon displays the current up and down rate. 
   4. Where previously the various limits boxes didn't validate
      properly now they do and any invalid input is interpreted as no
      limit and a nice infinity symbol is displayed. 
   5. Individual torrent info is saved and loaded when the program is
      closed and reopened allowing torrent limits to be remembered.
   6. If the program has to wait after the user clicking close for any
      active connections to shutdown down 'cleanly' an optional small
      dialog is displayed to alert the user that the program is still
      running.

Subversion: svn://svn.geekisp.com/halite

In addition you can check out the Trac site (trac.geekisp.com/halite)
but I haven't really done much with it ...yet.

## Thanks

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
    - Otome - otome08@gmail.com - Translation to Portuguese and Japanese.
    - Prome – prome.lwaku.info – Avtor slovenskega prevoda.
    - Tw@in 28 – twain_28@hotmail.com - Traduttore Italiano.
    - u!^DEV  - germanpg.hacker.to - deutsche Übersetzung.
    - webdr - www.langturk.com - Türkçe çeviri.
    - Čagalj - www.zmijuga.6x.to - Preveo sve na srpski (latinicu i ćirilicu).
    - AlexKrycek -alexkrycek0@gmail.com - μετάφραση στα ελληνικά.
    - Globa – www.raptor.yamasster.net – Перевод на русский.
    - khagaroth - khronos@seznam.cz - Autor českého překladu.
    - TaperHarley – Traducere în limba Româna.
    - Aмбасадор - www.ambasadorforum.co.cc – превод на makedonski (ћирилицу).
