#!/bin/sh

##############################################
##############################################
####	FILE:	utils.sh
####
####	Contains utility functions.
####
##############################################
##############################################



##
## ClearScreen
##
ClearScreen ()
{
	clear
}


##
## AreYouSure
##
## New calling convention: AreYouSure $1 $2
## where $1 is the prompt
## and   $2 is the default answer (one of y or n)
##
AreYouSure ()
{
	# Initialize yesorno
	yesorno=$2

	echo ""
	$ECHON "		$1 ?[${BOLD}$yesorno${OFFBOLD}]:"

	read resp
	if [ ! -z "$resp" ]; then
		yesorno=`echo $resp | cut -c 1 -`
	fi

	if [ "$yesorno" = "y" ] || [ "$yesorno" = "Y" ]; then
		return 1
	else
		return 0
	fi
}


##
## AreYouSure2 (Experimental)
## $1 => prompt
## $2 => default answer (y or n)
##
Are_You_Sure_2 ()
{
	## This works only on Solaris!!
	CKYORN=/usr/bin/ckyorn

	# Initialize yesorno
##	yesorno=n
	yesorno=$2

##	echo ""
##	$ECHON "		$1 ?[${BOLD}$yesorno${OFFBOLD}]:"
##	read yesorno

	resp=`$CKYORN -Q -p "$1" -d "$2" `

	yesorno=`echo $resp | cut -c 1 -`

	if [ "$yesorno" = "y" ] || [ "$yesorno" = "Y" ]; then
		return 1
	else
		return 0
	fi

}


##
## Signal handler for user interrupt.
##
SigHandler ()
{
	AreYouSure "Are you sure you want to exit" "n"
	if [ $? = 1 ]; then
		exit 2
	fi
}

trap 'SigHandler' 1 2 15


##
## Pause
##
Pause ()
{
	$ECHO ""
	$ECHON "		Hit [CR] to continue.... "
	read cr
}


##
## SelectChoice
##
SelectChoice ()
{
	$ECHON "		Select choice by number: "
}

##
## prompts for user input and returns the user's choice. $1 is the prompt, 
## $2 is the default value, the result is stored in _retval
##
AskUserInput ()
{
    $ECHON "$1 [$2]: "
    read resp

    if [ -z "$resp" ]
    then
	_retval=$2
    else
	_retval=$resp
    fi
}

