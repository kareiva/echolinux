echolinux
=========

EchoLink port to Linux
Readme for echolinux 0.17a alpha

This is an alpha distribution of echolinux and you attempt this at
your own risk.

Please read the "Using Echolinux" file as it tells about echolinux in more
detail. This readme is to tell you how to build echoLinux and the sample 
gui, echogui.

Version 0.17a Release Note:
February 24, 2006
Corrected keep alive bug where the client would become disconnected after 15 minutes of no activity (thanks Diane va3db). Also updated the desktop file to conform to freedesktop.orgs standards.

Version 0.16a Release Note:
February 15, 2004
Now using Jeff's threaded server code. The gui will respond to mouse events while updating the server list. Also, you can now have multiple servers in the servers.txt file. A '#' in the first column means the line is a comment. An example servers.txt file.

nasouth.echolink.org
# this is a comment
naeast.echolink.org 

Prevented resizing of the main window (thanks Thomas, dl9sau). 

Configuration files can now be kept in one of two directories. The search order is first ~/.echoLinux and then /etc/echolinux. There is now a new top level makefile. To build, change directory to the top level of the source tree, as user do a make and then make copy_defaults. As root do a make install. Be sure to edit ~/echoLinux/userdata.txt to include your call and echolink password.

With this release, source and binary RPMs are available for Redhat 9 and Fedora Core 1. You will need to install the xforms package before you install echolinux. It is available from http://download.fedora.us/fedora/fedora/1/i386/RPMS.stable/ for FC1 and http://download.fedora.us/fedora/redhat/9/i386/RPMS.stable/ for Redhat 9. If you are building from the source RPM, you will need the xforms-devel package as well. With the RPM packages, configuration files are stored in /etc/echolinux. Be sure to edit /ect/echolinux/userdata.txt to include your call and echolink password. You will find a launcher in main menu->internet->more internet applications to run echolinux. 

Version 0.14a Release Note:
December 14, 2003
New maintainer....
Ron Patterson W6FM
w6fm@qsl.net

Changed echogui to handle 6 digit node ids correctly. Changed echogui 
and echolinux to give a descriptive error message in the event that 
execlp fails due to echolinux or echoaudio not being in the users path.

Version 0.13a Release Note:
Jan. 10, 2003
Noticed that the address servers stopped sending active users in alphabetical
order. So, I had to change the way the users lists were created. I had to
start inserting stations in alphabetical order, rather then just appending 
each new entry read from the server on to the end of the list.

This change ONLY effects echogui. No changes were made to echolinux or 
echoaudio, so if you have 0.12a, you need not rebuild these two. 

Version 0.12a Release Note:

Just a minor change concerning address server logon and update.
It really only effects echogui, the change was in servercode.c. So, you
only have to rebuild and install echogui and NOT echoliunx and echoaudio.

Version 0.11a Release Note:

MAJOR CHANGES........

Removed the Mic & Vol control sliders and all audio volume setting code. 
This was done because of differences in sound cards and drivers in Linux. 
Especially the OSS vs. ALSA drivers and the new AC-97 codecs.
You must now use an external mixer program (gmix, alsamixer, etc) to set and 
control the audio settings.  

Added vox capability, a prelim to sysop version. You now can have a vox mode 
and set the threshold. When  connecting to a station you start in "normal" 
ptt mode. You can then enable vox and disable vox anytime while connected. 
You can change the vox threshold by moving the slider next to the vox button. 
It's scale is the same as the audio stength meter and it can be used as a 
reference. 

Made it so connect beep actually beeps on connection success rather than when
starting the attempt.

Added a four minute, 240 sec, tx timeout timer in vox.c.

Changed the nodes list colors. Instead of white-on-blue it is black-on-white.
Plus, the background changes to red when the list is being updated.

Xforms has issued a new release, version 1.0. The 0.89 version is no longer
available. There are no incompatabilities between the two. The software runs
just fine under 0.89 or 1.0. So, if you already have 0.89 installed, you don't
have to upgrade to 1.0 unless you want to. It might be a good idea though to
do it sometime. 

Changed duplex.* to vox.*. Duplex never made sense anyhow.

-------------------------------------------------------

Version 0.8a Release Note: 
This release fixes a problem when building on a SUSE 8.1 Linux distribution
that was discovered by Gary, w7ntf. It dealt with a macro not behaving
correctly. It behaved ok on my Slackware and several Red Hat 7.3 & 8.0 
distros. Just the SUSE had problems. So, I changed the macro to a function
to solve it on all distributions.
Thanks to Gary for allowing me to log into his sytem to debug.

   ------------------------------------

There are two directories, echolinux and echogui. The echolinux directory is
the most important. The echogui directory contains code to run a sample gui
I threw together. It is nothing more then an interface that uses the
echolinux app.


Building echoLinux:

To build echolinux change to the echoLinux/echolinux directory and do a make.
You will get a warning from each file about a pointer reference, ignore them.

The result is two executables, echolinux and echoaudio. 

NOTE: If you have previously installed echolinux for this user, you need not
do the following as you have alreday previously created teh user info files.

There are also two textfiles, userdata.txt and info.txt. Use a text editor on 
userdata.txt file and put your callsign, name, location and password 
on different lines in that order.
 
Callsign needs to be in uppercase since it is used by echolink clients to 
verify  you on a server. This is the data used for logging to the server and 
sending on connects to other nodes. Just the same as echolink uses. 
Replace the "<.....>" with the information stated between the <>'s. Delete the
'<' and '>'. Example, my userdata.txt contains:
WD4NMQ
Jeff / echoLinux
Testing echoLinux
MY_PASSWORD 

The info.txt file is the data that is displayed in the info area on the
other node.

These files need to be put in a place in your user id area where echolinux and 
any gui programs can find them. I decided to put them in a directory called 
$HOME/.echoLinux.

To do this simply run 'make copy_defaults'. This will create ~/.echoLinux and 
copy the default .txt files and .wav files there.
After you do 'make copy_defaults' any achnges you wish to make to 
userdata.txt or info.txt must beamde to the file in the ~\.echoLinux
directory.

NOTE: End of user files install, you must do teh following on all upgrades.

You then need to change to root user and execute `make install'. This copies 
the two executables, echolinux and echoaudio to /usr/local/bin directory, 
which should be in your path. 

echolinux is now ready to run from the command line. Read Using EchoLinux file 
to learn the commands.

Remember, echoLinux DOES NOT make any connection to a user server like 
serve1.echolink.org. It expects a REAL internet hostname or an IP address. 
Also, since newer versions of echolink VERIFY a connection request by making 
sure your callsign matches the current IP address on the server, you may 
have problems connecting to echolink clients. 

You can overcome this one or two ways. One is to run an older version of 
echolink, like  1.1.603, or an older version of iLink on another machine on 
your internal network and use it as a test target. Or, you can get my echolink 
server access app from http://home.earthlink.net/~wd4nmq and use it in a 
separate window to logon to the server. Plus, you can use it to get the nodes 
list and get the IP address from the returned list to cut-&-paste with the 
echolinux 'C' command.

Build echoGui:
WARNING: As of Dec 21, 2002 xform 0.89 is no longer available. See
version 11a Release Note.

In order to build and run echogui you must have xforms installed. 
You can get it from:
ftp://ncmir.ucsd.edu/pub/xforms/linux-i386/xforms-1.0-release.tgz

IF YOU HAVE PREVIOUSLY INSTALLED XFORMS, INCLUDING VERSION 0.89, YOU
DO NOT HAVE TO DO IT AGAIN!!

Un-tar it, read it's README file and follow it's directions.

RED HAT USERS BEWARE!!!!!!!
When running the 'make install' you could get an error when it attempts
to install the man pages. Ignore this error. The reason you get it is because
Red Hat decided that they would put the man pages in a totally different
directory then where all other distros put it. All others put them in 
/usr/man and RH puts them in /usr/share/man. Why? I guess Red Hat feels they
are smarter than everybody else. 
Thanks to Will, w4wwm, for being my guinea pig and finding this out. I use
the Slackware distro.

Remember, like the Using Echoliux says, you DON'T have to have a gui to run
echolinux. It can be run from the command line. But, you have to know the IP,
or the actual hostname of the target node. Plus, you must have an external
mixer app to set the audio play and record levels.
echogui is a sample only gui I wrote using xforms in order to test echoLinux. 
If you wish to use it you must have successfully installed xforms, see above. 

If you have installed xforms, go to the echoLinux/echogui directory and 
type make.

NOTE: If you have previously installed echogui as this user, you do not have
to do the servers.txt or 'make copy_defaults' again. 

In echohgui/ there is also a file servers.txt. This is where you put the
echolink user server name, i.e. server1.echolink.org . I only try one server
at this time. Place the server name you want to try here and copy the file to 
the ~/.echoLinux directory by executing `make copy_defaults'.

Note: You must do teh following for all new upgrades and installs.

Now change to root user and execute `make install'. This copies echogui 
executable to the /usr/local/bin directory. Exit root back to normal user.

echogui spawns echolinux, so echolinux and echoaudio MUST be in your $PATH. 
That is why I copy them to /usr/local/bin. If you want them somewhere else 
you can copy them there. But, make sure they are in your $PATH. If you don't 
know what I am talking about, do your homework and find out. Everything you 
will ever do under Linux depends on these basics.

You can now execute the gui by entering 'echogui &'

Do to differences in fonts list in different X installations the fonts in
the nodes list and the infortion window are to small for easy viewing using
my font settings.

To increase the font size you need to edit the echogui/Makefile. In it you wil fin the following lines:

CARGS = -DNodeFont=FL_TINY_SIZE -DChatFont=FL_SMALL_SIZE \
        -DInfoFont=FL_TINY_FONT

#CARGS = -DNodeFont=FL_SMALL_SIZE -DChatFont=FL_SMALL_SIZE \
#       -DInfoFont=FL_SMALL_FONT

To increase font size comment out the first two linee and un-comment the
last two to this:

#CARGS = -DNodeFont=FL_TINY_SIZE -DChatFont=FL_SMALL_SIZE \
#        -DInfoFont=FL_TINY_FONT

CARGS = -DNodeFont=FL_SMALL_SIZE -DChatFont=FL_SMALL_SIZE \
       -DInfoFont=FL_SMALL_FONT

To rebuild, be in the echogui directory and do:

touch *.c
make

su to be root
make install
exit back to normal user.

The fonts will now be bigger in the node list and info window. 

This included gui was just written as a model showing what can be done with
echoLinux. I am not really going to support it. It is just a test bed. I am
concentrating on improving and adding functionality to echoLinux.

After doing the above installation and testing it out, you can delete the
echoLinux/ build tree as it is no longer needed. All executables come from
/usr/local/bin and the configuration files are in ~/.echoLinux directory.

USING ECHOGUI:

Using echogui is a lot like echolink, you are connected and logged on to the
echolink user server that is in servers.txt. To connect to another node you
simply double-click on the target node's entry line in the browser.

You can also directly connect to another node by using the "Remote Node" input
box. You simply enter either the target nodes IP address in "dot" format, 
example: 192.168.1.1, or it's DNS host name, example: wd4nmq.ampr.org (Not a
real address). Do NOT put the callsign, echolink ID number or conference
name in the "Remote Node" box. It does not use the echolink server.
This feature is handy for connecting two nodes on the same private LAN.

To toggle between tx and rx, click on the "PTT" button. Sorry, space bar 
doesn't work. That problem is due to a quirk in xforms I can do nothing 
about.  

Plans:
Start playing with full duplex Linux drivers in preparation for doing a sysop 
version.

Jeff Pierce
wd4nmq@earthlink.net
http://home.earthlink.net/~wd4nmq  
