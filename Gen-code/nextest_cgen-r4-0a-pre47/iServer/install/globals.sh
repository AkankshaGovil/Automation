#!/bin/sh


######################################
######################################
###  File:	globals.sh
###
###	Contains global variables.
###	(with reasonable defaults)
######################################
######################################


## Shell variables to import.

ROOT=$PROJECT
export ROOT

USER_ROOT=$PROJECT
export USER_ROOT

USER_NAME=$LOGNAME
export USER_NAME

TARGETS=targets
export TARGETS

FORCE=OFF
export FORCE

VERBOSE=ON
export VERBOSE

ADMINDIR=$ROOT/admin
#ADMINDIR=$PWD
export ADMINDIR

LOGFILE=$ADMINDIR/build.log
export LOGFILE

CURR_TARGET=Netoid500
export CURR_TARGET

TARGET_DIR=302
export TARGET_DIR

MAKETARGET=500
export MAKETARGET

CURR_PROC=68EN302
export CURR_PROC

CURR_IMAGE=ALL
export CURR_IMAGE

MENU=1
export MENU

TOOLDIR=$ROOT/utils/build
#TOOLDIR=$PWD
export TOOLDIR

Usage="Usage:  install.sh [-F -h -v -V -m] [-e userid] [-r root] [-o optfile] <target> [image]"
export Usage

BOLD=`/usr/bin/tput smso`
export BOLD

OFFBOLD=`/usr/bin/tput rmso`
export OFFBOLD

COMMANDS_FILE=commands.$$
export COMMANDS_FILE

TARGET_FILE=targets.$$
export TARGET_FILE

Version=0.5
export Version

ProgDescription="NexTone Development Setup Utility"
export ProgDescription

ProgDate="October 1999"
export ProgDate

HELP="  \n
$ProgDescription  v$Version of $ProgDate \n
\n
$Usage \n
\t	where \n
\t	-h \t\tprint this message \n
\t	-v \t\tprint out version information \n
\t	-V \t\tverbose mode \n
\t	-m \t\tdo not present menu based interface \n
"


BUILD_STATUS=failure
export BUILD_STATUS


##################################################
##
## Configuration Section.
##
##################################################

CONFIG_H_DIR=$ROOT/include
export CONFIG_H_DIR

CONFIG_H=$CONFIG_H_DIR/config.h
export CONFIG_H

LASTBUILD=$ADMINDIR/lastbuild
export LASTBUILD

## Type of processor.
PROCESSOR=68302
export PROCESSOR

GNUMAKE=/usr/local/bin/gnumake
export GNUMAKE

PREP=$TOOLDIR/prep
export PREP

SDUMP=$TOOLDIR/sdump
export SDUMP

MAIL=/usr/ucb/mail
export MAIL

## Target environment -- target or environment
TARGENV=TARGET
export TARGENV

## Name of software update file
UPDATE_FILE=b12.hex
export UPDATE_FILE

## Version of Software.
SOFTWARE_REV="1.01A01_`date +%d%h%y`"
export SOFTWARE_REV

## Internal version of Software.
INTERNAL_VER="1.00_`date +%d%h%y`"
export INTERNAL_VER

MC=`uname | grep Sun`
MC=${MC:-linux}
export MC

if [ "$MC" = "SunOS" ];
	then 
	ECHON="/usr/ucb/echo -n"
else 
	ECHON="/bin/echo -n" 
fi
export ECHON

ECHO=/bin/echo
export ECHO


## end of globals.sh
