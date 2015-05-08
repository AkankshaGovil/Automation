##############################################################################################
#
# Notice:
# This document contains information that is proprietary to RADVISION LTD..
# No part of this publication may be reproduced in any form whatsoever without
# written prior approval by RADVISION LTD..
#
# RADVISION LTD. reserves the right to revise this publication and make changes
# without obligation to notify any person of such revisions or changes.
#
##############################################################################################

##############################################################################################
#                                 TAP_channels.tcl
#
#   Channels handling.
#   This file holds all the GUI procedures for channels.
#   This includes new outgoing channel, incoming channel, channel information, etc.
#
#
#
#       Written by                        Version & Date                        Change
#      ------------                       ---------------                      --------
#  Oren Libis & Ran Arad                    04-Mar-2000
#
##############################################################################################


##############################################################################################
#
#   CHANNEL operations (general)
#
##############################################################################################


# channel:createChannelsInfo
# Creation procedure for the channels information INSIDE other windows.
# This function is called for the main window and for the call information windows.
# input  : base       - The base frame to create the channels information in
#                       Each call should have a different base window handle
#          selectable - Indicates if items in listboxes can be selected
# output : none
# return : none
proc channel:createChannelsInfo {base selectable} {
    global tmp app

    ###################
    # CREATING WIDGETS
    ###################
    listbox $base.inList -tooltip "Incomming Channels" \
        -yscrollcommand "$base.scrl set" -background White -height 1 -width 1 -exportselection 0 -selectmode single
    listbox $base.outList -tooltip "Outgoing Channels" \
        -yscrollcommand "$base.scrl set" -background White -height 1 -width 1 -exportselection 0 -selectmode single
    scrollbar $base.scrl -command "channel:yviewScrl $base"

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid columnconf $base 2 -weight 1
    grid rowconf $base 0 -minsize 4
    grid rowconf $base 1 -weight 1
    grid $base.inList -in $base -column 0 -row 1 -sticky nesw -pady 1
    grid $base.outList -in $base -column 2 -row 1 -sticky nesw -pady 1
    grid $base.scrl -in $base -column 1 -row 1 -sticky ns -pady 1

    ###########
    # BINDINGS
    ###########
    if {$selectable} {
        bind $base.inList <<ListboxSelect>> "channel:duplicateSelection $base.inList $base.outList"
        bind $base.outList <<ListboxSelect>> "channel:duplicateSelection $base.outList $base.inList"
    } else {
        # Make sure we can't select anything
        bind $base.inList <<ListboxSelect>> "$base.inList selection clear 0 end"
        bind $base.outList <<ListboxSelect>> "$base.outList selection clear 0 end"
    }
    placeHeader $base Channels:In/Out
}


# channel:duplicateSelection
# Make sure that a selected item in one of the channel listboxes will be set in the other
# one as well.
# input  : selectedListbox      - The listbox where the selection event occured
#          duplicatedListbox    - The listbox to duplicate the selection to
# output : none
# return : none
proc channel:duplicateSelection {selectedListbox duplicatedListbox} {
    set item [$selectedListbox curselection]
    if {$item != ""} {
        $duplicatedListbox selection clear 0 end
        $duplicatedListbox selection set $item
    }
}


# channel:yviewScrl
# Set the listboxes view according to the scrollbar changes
# input  : base - Base frame widget holding the channel lists
#          args - Arguments added to the procedure when a scrollbar event occurs
# output : none
# return : none
proc channel:yviewScrl {base args} {
    eval "$base.inList yview $args"
    eval "$base.outList yview $args"
}

# channel:SetButtonsState
# This procedure makes sure channel buttons are disabled in the main window when no channel is selected.
# input  : none
# output : none
# return : none
proc channel:SetButtonsState {base} {
    # Check if any channels are selected from the lists
    set inSel [.test.chan.inList curselection]
    set outSel [.test.chan.outList curselection]
    if { $inSel != "" } {set inSel [.test.chan.inList get $inSel]}
    if { $outSel != "" } {set outSel [.test.chan.outList get $outSel]}

    if { $inSel != ""} {
        set stateStr "normal"
        $base.chanBut.answer configure -state "normal"
    } else {
        set stateStr "disable"
        $base.chanBut.answer configure -state "disable"
    }
    $base.chanBut.dropIn configure -state $stateStr
    if { $outSel != ""} {
        $base.chanBut.loop configure -state $stateStr
        $base.chanBut.dropOut configure -state "normal"
        set stateStr "normal"
        $base.chanBut.replace configure -state $stateStr
        $base.chanBut.same configure -state "disable"
    } else {
        # activate SameSession only if there is an incoming channle without an outgoing one
        $base.chanBut.same configure -state $stateStr
        $base.chanBut.dropOut configure -state "disable"
        $base.chanBut.replace configure -state "disable"
        $base.chanBut.loop configure -state "disable"
    }
    # set drop to active if either is selected
    $base.chanBut.drop configure -state $stateStr
}




##############################################################################################
#
#   INCOMING CHANNEL operations
#
##############################################################################################


# inchan:create
# Creation procedure for an incoming channel window
# This window is opened whenever there's a new incoming channel for a call and the mode is not
# automatic.
# input  : none
# output : none
# return : none
proc inchan:create {args} {
    global tmp app

    set vars [llength $args]
    if {$vars == 0} {
        set base .inchan
        set title "Incoming Channel"
        set tmp(inchan,cmd) "Channel.ResponseForOLC"
    } else {
        set base [lindex $args 0]
        set title [lindex $args 1]
        set tmp(inchan,cmd) [lindex $args 2]
    }

    set tmp(inchan,base) $base

    if {[winfo exists $base]} {
        wm deiconify $base; return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel
    wm focusmodel $base passive
    wm geometry $base 400x180+208+188
    wm maxsize $base 1030 755
    wm minsize $base 106 2
    wm overrideredirect $base 0
    wm resizable $base 0 0
    wm title $base $title
    wm transient $base .dummy
    wm protocol $base WM_DELETE_WINDOW "
        Window close $base
        focus .test
    "

    label $base.callLabel -borderwidth 0 -padx 10 -text "Channel:"
    label $base.callInfo -borderwidth 0 ; #will actually hold the channel handle
    frame $base.chanInfo -height 60 -width 80
    scrollbar $base.chanInfo.scrl -command "$base.chanInfo.txt yview"
    text $base.chanInfo.txt -height 10 -width 60  -yscrollcommand "$base.chanInfo.scrl set"
    button $base.accept -highlightthickness 0 -text Accept -tooltip "Accept (Return)" \
            -command {$tmp(inchan,cmd) [$tmp(inchan,base).callInfo cget -text] Accept}
    button $base.reject -highlightthickness 0 -text Reject -tooltip "Reject (Escape)" \
            -command {$tmp(inchan,cmd) [$tmp(inchan,base).callInfo cget -text] Reject}

    ###########
    # BINDINGS
    ###########
    bind $base <Return> "$base.accept invoke"
    bind $base <Escape> "Window close $base"

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid $base.callLabel -in $base -column 0 -row 0 -sticky w
    grid $base.callInfo -in $base -column 1 -row 0 -sticky w
    grid $base.chanInfo -in $base -column 0 -row 1 -columnspan 3 -sticky new
    grid rowconf $base.chanInfo 0 -weight 1
    grid $base.chanInfo.scrl -in $base.chanInfo -column 1 -row 0 -sticky nesw
    grid $base.chanInfo.txt -in $base.chanInfo -column 0 -row 0 -pady 1  -sticky nesw
    grid $base.accept -in $base -column 2 -row 2 -padx 2
    grid $base.reject -in $base -column 1 -row 2 -padx 2 -pady 2
}

# inchan:terminate
# Terminates the incoming/dropping channel window
# Called when a channel is connected or timed out.
# input  : application channel handle
# output : none
# return : none
proc inchan:terminate { chanInfo } {
    if {[winfo exists .inchan]} {
        if { $chanInfo == [.inchan.callInfo cget -text] } {
            Window close .inchan
        }
    }
    if {[winfo exists .dropchan]} {
        if { $chanInfo == [.dropchan.callInfo cget -text] } {
            Window close .dropchan
        }
    }
}
