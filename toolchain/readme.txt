
 ----------------
 OPEN2X TOOLCHAIN
 ----------------

 This is the official toolchain of the open2x project. It consists of a
 makefile for easy building and source code for various gnu tools modified
 specifically for the gp2x handheld gaming system.

 The latest version can always be found in the subversion repository:

  svn checkout https://svn.sourceforge.net/svnroot/open2x/trunk/toolchain

 For more information and other useful software, please visit the open2x
 project page on sourceforge.

 ----------------
 CONFIG AND USAGE
 ----------------

 1) Add this to your startup configuration:

  ## OPEN2X SETTINGS
  export OPEN2X="/usr/local/open2x"
  export PATH="$PATH:$OPEN2X/bin"

 2) Execute the makefile to build the toolchain:

  make

 3) Verify the installation:

  open2x-gcc -v

 4) Enjoy!
