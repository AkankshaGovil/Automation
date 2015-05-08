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
#                                 TAP_calls.tcl
#
#   Calls handling.
#   This file holds all the GUI procedures for calls: incoming call, outgoing call, overlapped
#   sending, call information, etc.
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
#   CALL INFORMATION operations
#
##############################################################################################


# call:create
# Creation procedure of a call information window.
# Such a window can be opened for each of the calls of the test application and it displays the
# current call status, along with previous messages, channel information, etc.
# input  : base     - The base handle of the window
#                     Each call should have a different base window handle
#          title    - Title string to display for window
# output : none
# return : none
proc call:create {base title} {
    global tmp app

    if {[winfo exists $base]} {wm deiconify $base; return}

    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel
    wm focusmodel $base passive
    wm geometry $base 400x300+600+150
    wm maxsize $base 1028 753
    wm minsize $base 106 2
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base $title
    wm transient $base .dummy
    wm protocol $base WM_DELETE_WINDOW "destroy $base
            focus .test"

    frame $base.status -borderwidth 1 -relief groove
    label $base.status.q931StatusLab -borderwidth 0 -text "Q931 Status"
    label $base.status.q931Status -borderwidth 1 -relief sunken
    label $base.status.h245StatusLab -borderwidth 0 -text "H245 Status"
    label $base.status.h245Status -borderwidth 1 -relief sunken
    label $base.status.bwLab -borderwidth 0 -text "Band Width"
    label $base.status.bw -borderwidth 1 -relief sunken
    listbox $base.status.callInfo -yscrollcommand "$base.status.scrl set" -height 1 -width 0 -background White
    scrollbar $base.status.scrl -command "$base.status.callInfo yview"
    frame $base.chan -borderwidth 2 -relief groove
    frame $base.messages -borderwidth 2 -relief groove
    label $base.messages.header -text Messages
    listbox $base.messages.txt -xscrollcommand "$base.messages.xscrl set" -yscrollcommand "$base.messages.yscrl set" \
        -width 0 -height 1 -background White
    scrollbar $base.messages.yscrl -command "$base.messages.txt set"
    scrollbar $base.messages.xscrl -orient horizontal -command "$base.messages.txt set"
    button $base.messages.clear -tooltip "Clear message box" \
        -borderwidth 0 -highlightthickness 0 -image bmpClear -command "$base.messages.txt delete 0 end"

    channel:createChannelsInfo $base.chan 0

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.status -in $base -column 0 -row 0 -sticky nesw -pady 4 -padx 2
    grid columnconf $base.status 1 -weight 1
    grid columnconf $base.status 2 -weight 1
    grid $base.status.q931StatusLab -in $base.status -column 0 -row 0
    grid $base.status.q931Status -in $base.status -column 1 -row 0 -sticky ew -pady 2
    grid $base.status.h245StatusLab -in $base.status -column 0 -row 1
    grid $base.status.h245Status -in $base.status -column 1 -row 1 -sticky ew
    grid $base.status.bwLab -in $base.status -column 0 -row 2
    grid $base.status.bw -in $base.status -column 1 -row 2 -sticky ew -pady 2
    grid $base.status.callInfo -in $base.status -column 2 -row 0 -rowspan 3 -sticky nesw
    grid $base.status.scrl -in $base.status -column 3 -row 0 -rowspan 3 -sticky nesw
    grid $base.chan -in $base -sticky nesw -column 0 -row 1 -pady 4 -padx 2
    grid $base.messages -in $base -column 0 -row 2 -sticky nesw -pady 4 -padx 2
    grid columnconf $base.messages 0 -weight 1
    grid rowconf $base.messages 1 -weight 1
    grid $base.messages.txt -in $base.messages -column 0 -row 1 -sticky nesw -padx 5 -pady 5
    grid $base.messages.yscrl -in $base.messages -column 1 -row 1 -sticky nesw
    grid $base.messages.xscrl -in $base.messages -column 0 -row 2 -sticky nesw
    grid $base.messages.clear -in $base.messages -column 1 -row 2 -sticky nesw

    ###########
    # BINDINGS
    ###########
    bind $base.status.callInfo <<ListboxSelect>> "$base.status.callInfo selection clear 0 end"
    bind $base.messages.txt <<ListboxSelect>> "$base.messages.txt selection clear 0 end"

    ########
    # OTHER
    ########
    placeHeader $base.messages Messages
}



# call:doubleclickedCall
# This procedure is invoked when the user double-clicks on a call in the calls list box on the main
# window.
# It just opens the call's information window
# input  : none
# output : none
# return : none
proc {call:doubleclickedCall} {} {

    # Get the call's information window handle from the selection
    set callInfo [selectedItem .test.calls.list]
    set windowHandle [Call.GetWindowHandle $callInfo]

    if {$windowHandle != ""} {
        Window open .call ".$windowHandle" "Call: $windowHandle"
        Call.DisplayInfo $callInfo
    }
}


# Call.FindCallIndex
# This procedure finds out the index of a call inside the main calls window.
# It is used by C functions when a specific call needs to be located.
# input  : CallCounter - Counter information of the call
#                        This is the first number of the call information inside the main calls window.
# output : none
# return : The index of the call or -1 if call wasn't found
proc Call.FindCallIndex {CallCounter} {

    # Set search string for the call counter
    set SearchStr $CallCounter
    append SearchStr " *"

    # Get the list and find the call's index inside it
    set i [lsearch -glob [.test.calls.list get 0 end] $SearchStr]

    return $i
}



# Call:ChangeState
# This procedure changes the string representing a call inside the main window.
# It makes sure the current selection is not changed in the calls listbox.
# input  : counter      - Call counter for the changed call
#          stateString  - String to display instead of the last one
# output : none
# return : none
proc Call:ChangeState {counter stateString} {

    # Remember the last selection we had
    set selected [.test.calls.list curselection]

    # Find out the line we're looking for
    set i [Call.FindCallIndex $counter]

    if {$i != -1} then {
        # Remove the older element
        .test.calls.list delete $i
    } else {
        set i end
        set selected end
        .test.calls.conf insert end {}
        .test.calls.conn insert end {}
    }

    # Set the new state for the call
    .test.calls.list insert $i $stateString

    if { [.test.calls.list size] == 1 } then {
        # We mark a call if it's the only one we've got
        .test.calls.list selection set 0
    } elseif {$selected != ""} {
        # Make sure the selection is still fine
        .test.calls.list selection clear 0 end
        .test.calls.list selection set $selected
    }

    event generate .test.calls.list <<ListboxSelect>>
}

# Call:AddColor
# This procedure adds the confrence color to the color listbox
# input  : counter     - Call counter for the changed call
#          stateString - String to display instead of the last one
# output : none
# return : none
proc Call:AddColor {counter conf conn} {
    # Find out the line we're looking for
    set i [Call.FindCallIndex $counter]

    if {$i != -1} then {
        .test.calls.conf itemconfigure $i -background $conf
        .test.calls.conn itemconfigure $i -background $conn
    }
}

# Call:AddRemark
# This procedure adds a remark to the string representing a call inside the main window.
# It makes sure the current selection is not changed in the calls listbox.
# input  : counter   - Call counter for the changed call
#          remString - String to add
# output : none
# return : none
proc Call:AddRemark {counter remString} {

    # Remember the last selection we had
    set selected [.test.calls.list curselection]

    # Find out the line we're looking for
    set i [Call.FindCallIndex $counter]

    if {$i != -1} then {
        # Remove the older element
        set String [.test.calls.list get $i]
        .test.calls.list delete $i
    } else {
        return
    }

    # Add the remark to the string
    append String ", $remString"
    # Set the new state for the call
    .test.calls.list insert $i $String

    if { [.test.calls.list size] == 1 } then {
        # We mark a call if it's the only one we've got
        .test.calls.list selection set 0
    } elseif {$selected != ""} {
        # Make sure the selection is still fine
        .test.calls.list selection clear 0 end
        .test.calls.list selection set $selected
    }

    event generate .test.calls.list <<ListboxSelect>>
}

# Call:RemoveRemark
# This procedure removes any remarks from the string representing a call inside the main window.
# It makes sure the current selection is not changed in the calls listbox.
# input  : counter   - Call counter for the changed call
# output : none
# return : none
proc Call:RemoveRemark {counter} {

    # Remember the last selection we had
    set selected [.test.calls.list curselection]

    # Find out the line we're looking for
    set i [Call.FindCallIndex $counter]

    if {$i != -1} then {
        # Remove the older element
        set String [.test.calls.list get $i]
        .test.calls.list delete $i
    } else {
        return
    }

    # Remove the remark from the string
    set rangeEnd [string last , $String]
    if {$rangeEnd != -1} {
        incr rangeEnd -1
        set String [string range $String 0 $rangeEnd]
    }
    # Set the new state for the call
    .test.calls.list insert $i $String

    if { [.test.calls.list size] == 1 } then {
        # We mark a call if it's the only one we've got
        .test.calls.list selection set 0
    } elseif {$selected != ""} {
        # Make sure the selection is still fine
        .test.calls.list selection clear 0 end
        .test.calls.list selection set $selected
    }

    event generate .test.calls.list <<ListboxSelect>>
}

# Call:cleanup
# This procedure closes all windows that belong to a specific call
# input  : callInfo      - Call counter for the changed call
# output : none
# return : none
proc Call:cleanup {callInfo} {
    global tmp app

    if {[array get tmp $callInfo] != ""} {
        # close all windows that belong to specific call
        foreach win $tmp($callInfo) {
            after idle "Window close $win"
        }

        array unset app $callInfo
    }
}

# Call:addWindowToList
# This procedure inserts all a window that belong to a specific call to a global list
# input  : callInfo      - Call counter for the changed call
# output : none
# return : none
proc Call:addWindowToList {callInfo win} {
     global tmp app

    # Set list of windows for a call
    if {[array get tmp $callInfo] != ""} {
        # Already have windows for this call
        lappend tmp($callInfo) $win
    } else {
        # First window in call - make a list for it
        set tmp($callInfo) [list $win]

    }

    # Make sure this window knows its call
    set tmp($win) $callInfo
}

# call:SetButtonsState
# This procedure makes sure buttons are disabled in the main window when no call is selected.
# input  : none
# output : none
# return : none
proc call:SetButtonsState {base} {

    # Check if any call is selected from the list
    if {[.test.calls.list curselection] != ""} {
        set stateStr "normal"
    } else {
        set stateStr "disable"
    }

    $base.callBut.drop configure -state $stateStr
    #$base.callBut.dropAll configure -state $stateStr
    $base.callBut.callProceeding configure -state $stateStr
    $base.callBut.alerting configure -state $stateStr
    $base.callBut.connect configure -state $stateStr
}

# call:SetChanButtonsState
# This procedure makes sure buttons are disabled in the main window when no call or
# no channel is selected.
# input  : none
# output : none
# return : none
proc call:SetChanButtonsState {base} {
    # Check if any call is selected from the list
    if {[.test.calls.list curselection] != ""} {
        $base.chanBut.new configure -state "normal"
    } else {
        $base.chanBut.new configure -state "disable"
        $base.chanBut.replace configure -state "disable"
        $base.chanBut.same configure -state "disable"
        $base.chanBut.answer configure -state "disable"
        $base.chanBut.drop configure -state "disable"
        $base.chanBut.dropIn configure -state "disable"
        $base.chanBut.dropOut configure -state "disable"
        $base.chanBut.loop configure -state "disable"
        return
    }

    # Check if any channles are selected from the lists
    set inSel [.test.chan.inList curselection]
    set outSel [.test.chan.outList curselection]
    if { $inSel != "" } {set inSel [.test.chan.inList get $inSel]}
    if { $outSel != "" } {set outSel [.test.chan.outList get $outSel]}

    if { $inSel != ""} {
        set stateStr "normal"
    } else {
        set stateStr "disable"
    }
    if { $outSel != ""} {
        $base.chanBut.loop configure -state $stateStr
        set stateStr "normal"
        $base.chanBut.replace configure -state $stateStr
        $base.chanBut.same configure -state "disable"
        $base.chanBut.answer configure -state "disable"
    } else {
        # activate SameSession only if there is an incoming channle without an outgoing one
        $base.chanBut.same configure -state $stateStr
        $base.chanBut.answer configure -state $stateStr
        $base.chanBut.replace configure -state "disable"
        $base.chanBut.loop configure -state "disable"
    }
    # set drop to active if either is selected
    $base.chanBut.drop configure -state $stateStr
}

# call:Log
# This procedure logs a message about a call
# input  : callCounter  - Call's counter value (this is part of the window's handle
#          message      - The message to log
# output : none
# return : none
proc call:Log {callCounter message} {
    global tmp app

    # See if we want to display the message at all
    if {$app(options,suppressMessages) == 1} return

    set base ".call$callCounter"

    if {[winfo exists $base]} {
        $base.messages.txt insert end $message
    }

    test:Log "Call $callCounter: $message"
}



# Call:DropAll
# This procedure drops all the calls stated in the given list box
# input  : listboxWidget    - The listbox widget holding the calls
#          dropReason       - The drop reason string to use for disconnecting the calls
# output : none
# return : none
proc Call:DropAll {listboxWidget dropReason base} {
    # Drop each of the calls found in the listbox
    if { [$listboxWidget index end] } {
        Call:DropAllIter $listboxWidget [$listboxWidget get 0 end] $dropReason
    }
}

# Call:DropAllIter
# This procedure drops each of the call in the list, with Idle time between them.
# input  : callList         - List of the calls to drop
#          dropReason       - The drop reason string to use for disconnecting the calls
# output : none
# return : none
proc Call:DropAllIter {listboxWidget callList dropReason} {
    set callHandle [lindex $callList 0]

    # Drop the first call in the list if it still exists
    if { [lsearch -exact [$listboxWidget get 0 end] $callHandle] != -1 } {
        Call.Drop $callHandle $dropReason
    }

    if { [llength $callList] } {
        after idle "Call:DropAllIter $listboxWidget { [lrange $callList 1 end] } $dropReason"
    }
}





##############################################################################################
#
#   INCOMING CALL operations
#
##############################################################################################


# incall:create
# Creation procedure of an incoming call window.
# This window is opened whenever there's an incoming call to an application which is not set to
# receive the call in automatic mode
# input  : none
# output : none
# return : none
proc incall:create {} {
    global tmp app

    if {[winfo exists .incall]} {
        wm deiconify .incall; return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .incall -class Toplevel
    wm focusmodel .incall passive
    wm geometry .incall $app(incall,size)
    wm maxsize .incall 1028 753
    wm minsize .incall 150 150
    wm overrideredirect .incall 0
    wm resizable .incall 1 1
    wm title .incall "Incoming Call"
    wm transient .incall .dummy
    wm protocol .incall WM_DELETE_WINDOW {.incall.buttons.clos invoke}

    # Remote side params
    frame .incall.remote -borderwidth 2 -relief groove
    checkbutton .incall.remote.fs -state disabled -text "Fast Start"
    checkbutton .incall.remote.e245 -state disabled -text Early
    checkbutton .incall.remote.tun -state disabled -text Tunneling
    checkbutton .incall.remote.sendCmplt -state disabled -text "Sending Complete"
    checkbutton .incall.remote.canOvsp -state disabled -text "Can Overlap Send"

    # General call information
    frame .incall.info -borderwidth 2 -relief groove
    label .incall.info.uuiLab -text "User User Information" -width 18
    label .incall.info.uui -width 0 -relief sunken -anchor w
    label .incall.info.dispLab -borderwidth 0 -text Display -width 18
    label .incall.info.disp -width 0 -relief sunken -anchor w
    frame .incall.info.sourDest -height 75 -width 107
    scrollbar .incall.info.sourDest.scr -command {.incall.info.sourDest.lis yview}
    listbox .incall.info.sourDest.lis -height 12 -width 33 -yscrollcommand {.incall.info.sourDest.scr set} -background White

    # RTP/RTCP
#    if {[test.RtpSupported]} {
#        frame .incall.media -borderwidth 2 -relief groove
#        media:create .incall.media
#    }

    # Buttons frame
    frame .incall.buttons -height 75 -width 125
    label .incall.buttons.callInfo
    button .incall.buttons.clos -padx 0 -pady 0 -text Close -width 7 \
        -command {
                set app(incall,size) [winfo geometry .incall]
                Window close .incall
                focus .test
        }
    button .incall.buttons.wait -padx 0 -pady 0 -text Wait -width 7 \
        -command { H450.callWait [.incall.buttons.callInfo cget -text]
            .incall.buttons.clos invoke}
    button .incall.buttons.ica -padx 0 -pady 0 -text Incomplete -width 7 \
        -command {Call.IncompleteAddress [.incall.buttons.callInfo cget -text]
                .incall.buttons.clos invoke}
    button .incall.buttons.ac -padx 0 -pady 0 -text Complete -width 7 \
        -command {Call.AddressComplete [.incall.buttons.callInfo cget -text]
                .incall.buttons.clos invoke}
    button .incall.buttons.conn -padx 0 -pady 0 -text Connect -width 7 \
        -command {Call.SendConnect [.incall.buttons.callInfo cget -text]
                .incall.buttons.clos invoke}

    ###########
    # BINDINGS
    ###########
    bind .incall <Return> { .incall.buttons.clos invoke }
    bind .incall <Escape> { .incall.buttons.clos invoke }

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf .incall 0 -weight 1
    grid rowconf .incall 0 -minsize 7
    grid rowconf .incall 1 -weight 1
    grid rowconf .incall 4 -minsize 4

    grid .incall.remote -in .incall -column 1 -row 1 -sticky nesw -pady 2 -padx 2
    grid columnconf .incall.remote 0 -weight 1
    grid rowconf .incall.remote 0 -minsize 5
    foreach i {1 2 3 4 5} {grid rowconf .incall.remote $i -weight 1}
    grid .incall.remote.fs -in .incall.remote -column 0 -row 0 -sticky w
    grid .incall.remote.e245 -in .incall.remote -column 0 -row 2 -sticky w
    grid .incall.remote.tun -in .incall.remote -column 0 -row 3 -sticky w
    grid .incall.remote.sendCmplt -in .incall.remote -column 0 -row 4 -sticky w
    grid .incall.remote.canOvsp -in .incall.remote -column 0 -row 5 -sticky w

    grid .incall.info -in .incall -column 0 -row 1 -sticky news -pady 2 -padx 2
    grid columnconf .incall.info 1 -weight 1
    grid rowconf .incall.info 3 -weight 1
    grid .incall.info.uuiLab -in .incall.info -column 0 -row 1
    grid .incall.info.uui -in .incall.info -column 1 -row 1 -ipady 1 -pady 6 -sticky ew -padx 2
    grid .incall.info.dispLab -in .incall.info -column 0 -row 2
    grid .incall.info.disp -in .incall.info -column 1 -row 2 -ipady 1 -pady 0 -sticky ew -padx 2
    grid .incall.info.sourDest -in .incall.info -column 0 -row 3 -columnspan 2 -pady 3 -padx 1 -sticky nesw
    grid columnconf .incall.info.sourDest 0 -weight 1
    grid rowconf .incall.info.sourDest 1 -weight 1
    grid .incall.info.sourDest.scr -in .incall.info.sourDest -column 1 -row 1 -sticky nesw
    grid .incall.info.sourDest.lis -in .incall.info.sourDest -column 0 -row 1 -pady 1 -sticky nesw

    set culspan 2

#    if {[test.RtpSupported]} {
#        grid .incall.media -in .incall -column 2 -row 1 -sticky nesw -pady 2 -padx 2
#        incr culspan
#    }

    grid .incall.buttons -in .incall -column 0 -row 6 -columnspan $culspan -sticky new
    foreach i {0 1} {grid columnconf .incall.buttons $i -weight 1}
    grid rowconf .incall.buttons 0 -weight 1
    pack .incall.buttons.conn -in .incall.buttons -fill both -padx 2 -side right
    pack .incall.buttons.ac   -in .incall.buttons -fill both -padx 2 -side right
    pack .incall.buttons.ica  -in .incall.buttons -fill both -padx 2 -side right
    if { [test.H450Supported] } {
        pack .incall.buttons.wait -in .incall.buttons -fill both -padx 2 -side right
    }
    pack .incall.buttons.clos -in .incall.buttons -fill both -padx 2 -side right

    #########
    # OTHERS
    #########
    placeHeader .incall.info "Call Information"
    placeHeader .incall.remote "Remote"

#    if {[test.RtpSupported]} {
#        placeHeader .incall.media "Media channels"
#    }
}


# incall:clear
# Clear the incoming call window
# Called when window is closed
# input  : none
# output : none
# return : none
proc incall:clear {} {
    set base .incall

    $base.remote.fs deselect
    $base.remote.e245 deselect
    $base.remote.tun deselect
    $base.remote.sendCmplt deselect
    $base.remote.canOvsp deselect
    $base.info.uui configure -text ""
    $base.info.disp configure -text ""
    $base.info.sourDest.lis delete 0 end
}

# incall:terminate
# Terminates the incoming call window
# Called when a call is connected
# input  : callInfo string
# output : none
# return : none
proc incall:terminate { callInfo } {
    if {[winfo exists .incall]} {
        if { $callInfo == [.incall.buttons.callInfo cget -text] } {
            Window close .incall }
    }
}





##############################################################################################
#
#   NEW CALL operations
#
##############################################################################################



# newcall:multiRateValidate
# validates the band width combo box and the multi rate check box
# input  : rate - the band width that the user asked for
# output : none
# return : 0 or 1
proc newcall:multiRateValidate {rate base} {
    if {$rate > 8064001} {
        return 0
    }
    if {($rate > 38400) && ($rate <8064001)} {
        $base.callInfo.multiRate configure -state normal
        return 1
    }
    if {$rate <= 38400} {
        $base.callInfo.multiRate configure -state disable
        return 1
    }
    return 1
}






##############################################################################################
#
#   REDIAL operations
#
##############################################################################################


proc redial:send {handle} {
    Call.SendAdditionalAddr [.redial$handle.callInfo cget -text] 0 \
	    [.redial$handle.entry.alias get]
}

# redial:create
# Creation procedure of a redial window (Overlap sending)
# This window is opened when an "Incomplete address" message is sent by the other side of the
# call. It allows rejecting the call or supplying some more alias information for the call.
# input  : handle - handle of the call
# output : none
# return : none
proc redial:create {handle} {
    global tmp app

    set tmp(redial,base) .redial$handle

    if {[winfo exists $tmp(redial,base)]} {
        wm deiconify $tmp(redial,base)
        focus $tmp(redial,base).entry.alias
        return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel $tmp(redial,base) -class Toplevel
    wm focusmodel $tmp(redial,base) passive
    wm geometry $tmp(redial,base) +136+140
    wm overrideredirect $tmp(redial,base) 0
    wm resizable $tmp(redial,base) 0 0
    wm title $tmp(redial,base) "Overlapped Sending"
    wm transient $tmp(redial,base) .dummy
    wm protocol $tmp(redial,base) WM_DELETE_WINDOW \
            "Window close $tmp(redial,base); focus .test"
    frame $tmp(redial,base).entry

    # Allow adding only a string of digits
    label $tmp(redial,base).entry.aliasLab -borderwidth 1 -text "Digits"
    entry $tmp(redial,base).entry.alias -borderwidth 1 -validate key -invcmd bell \
            -vcmd { expr { [string length %P] < 131 } }
    checkbutton $tmp(redial,base).entry.sendComplete  -text "Sending Complete" \
            -variable tmp(redial,sendComplete) -tooltip "Signal that complete address was sent"

    button $tmp(redial,base).sendBut -highlightthickness 0 -padx 0 -pady 0 -text Send \
        -command "redial:send $handle"

    label $tmp(redial,base).callInfo

    ###########
    # BINDINGS
    ###########
    bind $tmp(redial,base) <Return> { $tmp(redial,base).sendBut invoke }

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $tmp(redial,base) 1 -weight 1
    grid rowconf $tmp(redial,base) 1 -weight 1
    grid $tmp(redial,base).entry -in $tmp(redial,base) -column 0 -row 0 -columnspan 4 \
            -sticky news

    # Allow adding only a string of digits
    grid $tmp(redial,base).entry.aliasLab -in $tmp(redial,base).entry -column 0 -row 0
    grid $tmp(redial,base).entry.alias -in $tmp(redial,base).entry -column 1 -row 0
    grid $tmp(redial,base).entry.sendComplete -in $tmp(redial,base).entry -column 2 -row 0 \
            -columnspan 2

    grid $tmp(redial,base).sendBut -in $tmp(redial,base) -column 3 -row 1 \
            -padx 1 -pady 1 -sticky e
    grid $tmp(redial,base).callInfo -in $tmp(redial,base) -column 2 -row 1

    #########
    # OTHERS
    #########
    focus $tmp(redial,base).entry.alias
}



##############################################################################################
#
#   FACILITY operations
#
##############################################################################################


# facility:create
# Creation procedure of a facility window
# This window is opened when the Facility button is pressed on the main window.
# It allows sending the 2 types of facility messages to the other side of a call.
# input  : callInfo - Selected call information
# output : none
# return : none
proc facility:create {callInfo} {
    global tmp app

    # Make sure we've got a call
    if {$callInfo == ""} {return}

    # Remove the additional information from the callInfo
    set callHandle [string range $callInfo 0 [expr [string first ":" $callInfo] - 1]]
    set call [string range $callHandle [expr [string first "x" $callHandle]-1] [string length $callHandle]]


    Call:addWindowToList $call ".facility"


    if {[winfo exists .facility]} {
        wm deiconify .facility
        .facility.sendBut configure -command "facility:send $callHandle"
        return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .facility -class Toplevel
    wm focusmodel .facility passive
    wm geometry .facility 360x270+221+182
    wm maxsize .facility 1028 753
    wm minsize .facility 325 217
    wm overrideredirect .facility 0
    wm resizable .facility 1 1
    wm deiconify .facility
    wm title .facility "Facility"
    wm protocol .facility WM_DELETE_WINDOW {.facility.cancel invoke}
    wm transient .facility .dummy

    radiobutton .facility.undefRad -text Undefined -value "undefined" -variable tmp(facility,type) \
        -command {
            set tmp(.facility,forwardRad) 0
            .facility.destAlias.name delete 0 end
            .facility.destAlias.name configure -state disable
            .facility.aliasList.txt delete 0 end
            .facility.userInput configure -state normal
            .facility.transAddress.histEnt.name configure -state disable
            focus .facility.userInput
        }
    radiobutton .facility.forwardRad -text {Forward Call} -value "forward" -variable tmp(facility,type) \
        -command {
            set tmp(.facility,undefRad) 0
            .facility.userInput delete 0 end
            .facility.userInput configure -state normal
            .facility.destAlias.name configure -state normal
             .facility.transAddress.histEnt.name configure -state normal
            focus .facility.destAlias.name
        }

    entry .facility.userInput -width 0 -textvar tmp(facility,userInput)
    frame .facility.destAlias -height 10 -relief groove
    frame .facility.aliasList -relief groove
    alias:createEntry .facility .facility.destAlias .facility.aliasList "" "Forwarding Address"
    button .facility.sendBut -highlightthickness 0 -padx 0 -pady 0 -text Send  -width 7 \
        -tooltip "Send Facility Message" \
        -command "facility:send $callHandle [alias:getAll .facility.aliasList.txt]"
    label .facility.facilityStrLabel -text {Facility Str}
    label .facility.callInfo -text $callHandle
    button .facility.cancel -highlightthickness 0 -padx 0 -pady 0 -text Cancel  -width 7 \
            -command {Window close .facility; focus .test}
    frame .facility.transAddress -relief flat
    ip:createEntry facility .facility.transAddress .facility.sendBut .facility.transAddress,address

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf .facility 0 -weight 1
    grid rowconf .facility 8 -weight 1
    grid .facility.undefRad -in .facility -column 0 -row 2 -columnspan 3 -sticky nw
    grid .facility.forwardRad -in .facility -column 0 -row 3 -columnspan 3 -sticky w
    grid .facility.userInput -in .facility -column 0 -row 1 -columnspan 3 -sticky ew
    grid .facility.destAlias -in .facility -column 0 -row 4 -columnspan 3 -sticky ew
    grid .facility.aliasList -in .facility -column 0 -row 5 -columnspan 3 -sticky nesw
    grid .facility.callInfo -in .facility -column 1 -row 8 -sticky w
    grid .facility.sendBut -in .facility -column 2 -row 8 -sticky e -padx 1
    grid .facility.cancel -in .facility -column 1 -row 8 -sticky e -padx 1
    grid .facility.facilityStrLabel -in .facility -column 0 -row 0 -columnspan 3 -sticky w
    grid .facility.transAddress -in .facility -column 0 -columnspan 3 -row 6 -sticky ew

    #########
    # OTHERS
    #########
    .facility.undefRad invoke
}


# facility:send
# Send facility message. Called internally when the facility button is pressed on the facility
# window.
# input  : counter  - Application counter of the call
#          handle   - Handle of the selected call
# output : none
# return : none
proc facility:send {counter handle} {
    Call.SendFacility "$counter $handle" [.facility.aliasList.txt get 0 end]
}







##############################################################################################
#
#   FAST START operations
#
##############################################################################################


# faststart:create
# Creation procedure of a fast start information window
# This window is opened whenever information about fast start call creation or acceptance is
# needed.
# input  : incoming - Indicates if operation is incoming or outgoing
#          types    - List of data types supported by the endpoint
#          followUp - the command to return to after setting the FS channels
# output : none
# return : none
proc faststart:create {callInfo incoming types followUp} {
    global tmp app

    if {[winfo exists .faststart]} {
        if {($tmp(faststart,incoming) == $incoming) && ([.faststart.callInfo cget -text] == $callInfo)} {
            # Just update and exit
            wm deiconify .faststart
            .faststart.chan.lb delete 0 end
            return
        }

        # We have to rebuild the faststart screen
        destroy .faststart
    }


    set tmp(faststart,incoming) $incoming
    set allTypes $types
    lappend allTypes ""

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .faststart -class Toplevel
    wm focusmodel .faststart passive
    wm geometry .faststart 450x300+78+159
    wm overrideredirect .faststart 0
    wm resizable .faststart 1 1
    wm deiconify .faststart
    wm title .faststart "Fast Start Information"
    wm transient .faststart .dummy
    wm protocol .faststart WM_DELETE_WINDOW {.faststart.cancel invoke ; focus .test}
    focus .dummy

    label .faststart.callInfo -text "$callInfo"

    menubutton .faststart.name -menu .faststart.name.m -text [lindex $types 0] -tooltip "Cahnnel name" \
        -indicatoron 1 -padx 5 -width 20 -pady 0 -relief sunken -textvariable tmp(faststart,name)
    menu .faststart.name.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach type $allTypes {
        if { $type != "" } {
            .faststart.name.m add radiobutton -indicatoron 0 -value $type -label $type \
                -variable tmp(faststart,name) -command {faststart:updateName $tmp(faststart,name)}
        }
    }
    menubutton .faststart.dir -menu .faststart.dir.m -text "In" -tooltip "Cahnnel direction" \
        -indicatoron 1 -padx 5 -width 7 -pady 0 -relief sunken -textvariable tmp(faststart,dir)
    menu .faststart.dir.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach diection {In Out} {
        .faststart.dir.m add radiobutton -indicatoron 0 -value $diection -label $diection \
            -variable tmp(faststart,dir) -command {faststart:updateDir $tmp(faststart,name)}
    }
    entry .faststart.rtcpAddr -width 20 -textvariable tmp(faststart,rtcpAddr) -tooltip "Cahnnel rtcp address"
    entry .faststart.rtpAddr  -width 20 -textvariable tmp(faststart,rtpAddr) -tooltip "Cahnnel rtp address"

    button .faststart.add -highlightthickness 0 -padx 0 -pady 0 -command "faststart:add" -image sunkDown -tooltip "Add channel to request list"
    button .faststart.rep -highlightthickness 0 -padx 0 -pady 0 -command "faststart:replace" -image sunkDown -tooltip "Replace channel in request list"
    button .faststart.del -highlightthickness 0 -padx 0 -pady 0 -command "faststart:del" -image sunkSlash -tooltip "Delete selected channel(s)"
    button .faststart.clr -highlightthickness 0 -padx 0 -pady 0 -command ".faststart.chan.lb delete 0 end" -image sunkX -tooltip "Clear all channels"

    frame .faststart.chan -borderwidth 0
    listbox .faststart.chan.lb -yscrollcommand ".faststart.chan.scrl set" -height 1 -width 0 -background White
    scrollbar .faststart.chan.scrl -command ".faststart.chan.lb yview"
    .faststart.chan.lb delete 0 end
    if {! $incoming } {
        set type [lindex $types 0]
        set IP [Options.GetLocalIP]
        if {![string compare -nocase -length 1 $type "g"]} {
            set rtp $tmp(faststart,audioPort)
            set rtcp [expr {$tmp(faststart,audioPort) + 1}]
        }
        if {![string compare -nocase -length 1 $type "h"]} {
            set rtp $tmp(faststart,videoPort)
            set rtcp [expr {$tmp(faststart,videoPort) + 1}]
        }
        if {![string compare -nocase -length 1 $type "t"]} {
            set rtp $tmp(faststart,dataPort)
            set rtcp [expr {$tmp(faststart,dataPort) + 1}]
        }
        .faststart.chan.lb insert end "$type : In ; $IP:$rtcp , $IP:$rtp"
        .faststart.chan.lb insert end "$type : Out ; $IP:$rtcp , 0.0.0.0:0"
    }

    button .faststart.cancel -highlightthickness 0 -padx 0 -pady 0 -width 8 \
        -text "Cancel"  -command "Window delete .faststart; $followUp" \
        -tooltip "Close faststart window"
    button .faststart.request  -highlightthickness 0 -padx 0 -pady 0 -width 8 \
        -text "Request" -command "Call.BuildOutgoingFaststart {$callInfo} {$followUp}" \
        -tooltip "Request faststart channels information from the other side"
    button .faststart.approve  -highlightthickness 0 -padx 0 -pady 0 -width 8 \
        -text "Approve" -command "Call.ApproveFaststart $callInfo" \
        -tooltip "Approve incoming faststart channels information"
    button .faststart.refuse  -highlightthickness 0 -padx 0 -pady 0 -width 8 \
        -text "Refuse" -command "Call.RefuseFaststart $callInfo" \
        -tooltip "Refuse incoming faststart"

    if {$incoming} {
        .faststart.rtcpAddr delete 0 end
        .faststart.rtpAddr delete 0 end
        set tmp(faststart,name) ""
        set tmp(faststart,dir) ""
        .faststart.chan.lb configure  -selectmode single
        .faststart.name configure     -state disabled
        .faststart.dir configure      -state disabled
        bind .faststart.chan.lb <<ListboxSelect>> {faststart:extract}
    } else {
        .faststart.chan.lb configure  -selectmode multiple
        .faststart.name configure     -state normal
        .faststart.dir configure      -state normal
        bind .faststart.chan.lb <<ListboxSelect>> ""
        set tmp(faststart,name) [lindex $types 0]
        set tmp(faststart,dir) "In"
        faststart:updateName $tmp(faststart,name)
    }

    ###################
    # SETTING GEOMETRY
    ###################

    grid columnconf .faststart 0 -weight 1
    grid columnconf .faststart 1 -weight 1
    grid columnconf .faststart 2 -weight 1
    grid columnconf .faststart 3 -weight 1
    grid rowconf    .faststart 1 -weight 1

    grid .faststart.name     -in .faststart -column 0 -row 0 -pady 2 -padx 2
    grid .faststart.dir      -in .faststart -column 1 -row 0 -pady 2 -padx 2
    grid .faststart.rtcpAddr -in .faststart -column 2 -row 0 -pady 2 -padx 2
    grid .faststart.rtpAddr  -in .faststart -column 3 -row 0 -pady 2 -padx 2
    if {$incoming} {
        grid .faststart.rep  -in .faststart -column 4 -row 0 -pady 2 -padx 2
    } else {
        grid .faststart.add  -in .faststart -column 4 -row 0 -pady 2 -padx 2
    }
    grid .faststart.del      -in .faststart -column 5 -row 0 -pady 2 -padx 2
    grid .faststart.clr      -in .faststart -column 6 -row 0 -pady 2 -padx 2

    grid .faststart.chan      -in .faststart -column 0 -columnspan 7 -row 1 -pady 2 -padx 2 -sticky news
    grid columnconf .faststart.chan 0 -weight 1
    grid rowconf    .faststart.chan 0 -weight 1
    grid .faststart.chan.lb   -in .faststart.chan -column 0 -row 0 -pady 0 -padx 0 -sticky news
    grid .faststart.chan.scrl -in .faststart.chan -column 1 -row 0 -pady 0 -padx 0 -sticky ns

    grid .faststart.cancel   -in .faststart -column 3 -row 2 -pady 2 -padx 2
    if {$incoming} {
        grid .faststart.approve -in .faststart -column 4 -columnspan 3 -row 2 -pady 2 -padx 2
        grid .faststart.refuse -in .faststart -column 2 -columnspan 1 -row 2 -pady 2 -padx 2
    } else {
        grid .faststart.request -in .faststart -column 4 -columnspan 3 -row 2 -pady 2 -padx 2
    }
}

# faststart:updateName
# Reset the information of a line inside the faststart window.
# input  : none
# output : none
# return : none
proc faststart:updateName { type } {
    global tmp app

    set localIP [Options.GetLocalIP]

    if {![string compare -nocase -length 1 $type "g"]} {
        set rtcp [expr {$tmp(faststart,audioPort) + 1}]
    }
    if {![string compare -nocase -length 1 $type "h"]} {
        set rtcp [expr {$tmp(faststart,videoPort) + 1}]
    }
    if {![string compare -nocase -length 1 $type "t"]} {
        set rtcp [expr {$tmp(faststart,dataPort) + 1}]
    }

    set tmp(faststart,rtcpAddr) "$localIP:$rtcp"
    faststart:updateDir $type
}

# faststart:updateDir
# Reset the information of a line inside the faststart window.
# input  : none
# output : none
# return : none
proc faststart:updateDir { type } {
    global tmp app

    set localIP [Options.GetLocalIP]

    if {$tmp(faststart,dir) == "Out"} {
        set tmp(faststart,rtpAddr) "0.0.0.0:0"
    } else {
        if {![string compare -nocase -length 1 $type "g"]} {
            set rtp $tmp(faststart,audioPort)
        }
        if {![string compare -nocase -length 1 $type "h"]} {
            set rtp $tmp(faststart,videoPort)
        }
        if {![string compare -nocase -length 1 $type "t"]} {
            set rtp $tmp(faststart,dataPort)
        }

        set tmp(faststart,rtpAddr) "$localIP:$rtp"
    }
}

# faststart:add
# Add a line to the FS channels list
# input  : none
# output : none
# return : none
proc faststart:add { } {
    global tmp
    #build the line
    set str "$tmp(faststart,name) : $tmp(faststart,dir) ; $tmp(faststart,rtcpAddr) , $tmp(faststart,rtpAddr)"
    # Insert it into the list
    .faststart.chan.lb insert end $str
    # Make sure it's visible inside the listbox
    .faststart.chan.lb see end
}

# faststart:add
# Delete a line from the FS channels list
# input  : none
# output : none
# return : none
proc faststart:del { } {
    set delAliases [.faststart.chan.lb curselection]
    set size [llength $delAliases]
    for {set index [incr size -1]} {$index >= 0} {incr index -1} {
        .faststart.chan.lb delete [lindex $delAliases $index]
    }
}

# faststart:extract
# Convert selected line from the FS channels list to the windows
# input  : none
# output : none
# return : none
proc faststart:extract { } {
    global tmp

    set str [.faststart.chan.lb get [.faststart.chan.lb curselection] ]
    set lst [split $str]
    set name [lindex $lst 1]
    set dir [lindex $lst 3]
    set rtcpAddr [lindex $lst 5]
    set rtpAddr [lindex $lst 7]

    set tmp(faststart,name) $name
    set tmp(faststart,dir) $dir
    set tmp(faststart,rtcpAddr) $rtcpAddr
    set tmp(faststart,rtpAddr) $rtpAddr
}

# faststart:replace
# Replace the selected line with data from the windows
# input  : none
# output : none
# return : none
proc faststart:replace { } {
    global tmp

    if { $tmp(faststart,name) == "" } return
    #find where it was taken from
    set spot [.faststart.chan.lb curselection]
    #build the line
    set orig [.faststart.chan.lb get $spot]
    set first [string first "." $orig]
    set index [string range $orig 0 $first]
    set str "$index $tmp(faststart,name) : $tmp(faststart,dir) ; $tmp(faststart,rtcpAddr) , $tmp(faststart,rtpAddr)"
    #replace with update
    .faststart.chan.lb delete $spot
    .faststart.chan.lb insert $spot $str
    .faststart.chan.lb selection set $spot
    #make sure it's visible inside the listbox
    .faststart.chan.lb see $spot
}

# faststart:terminate
# Remove the FS window
# input  : none
# output : none
# return : none
proc faststart:terminate { callInfo } {
    if {[winfo exists .faststart]} {
        if { $callInfo == [.faststart.callInfo cget -text] } {
            Window delete .faststart
        }
    }
}





##############################################################################################
#
#   MEDIA (RTP/RTCP) operations
#
##############################################################################################


# media:create
# Creation procedure for the media options available - record/replay etc.
# input  : base     - Base frame to work in
# output : none
# return : none
#proc media:create {base} {
#    global app
#
#    ###################
#    # CREATING WIDGETS
#    ###################
#    radiobutton $base.none -text "None" -value 0 -variable app(media,rtpMode)
#    radiobutton $base.play -text "Play" -value 1 -variable app(media,rtpMode) -state disabled
#    radiobutton $base.rec -text "Record" -value 2 -variable app(media,rtpMode) -state disabled
#    radiobutton $base.replay -text "Replay media" -value 3 -variable app(media,rtpMode)
#    radiobutton $base.recreplay -text "Record & Replay media" -value 4 -variable app(media,rtpMode) -state disabled
#    entry $base.filename -textvariable app(media,filename)
#    button $base.browse -padx 0 -pady 0 -text Browse -width 7 -command media:browse
#
#    ###################
#    # SETTING GEOMETRY
#    ###################
#    grid columnconf $base 1 -weight 1
#    foreach i {1 2 3 4 5} {grid rowconf $base $i -weight 1}
#    grid rowconf $base 0 -minsize 5
#    grid $base.none      -in $base -column 0 -row 1 -sticky w
#    grid $base.play      -in $base -column 0 -row 2 -sticky w
#    grid $base.rec       -in $base -column 0 -row 3 -sticky w -columnspan 2
#    grid $base.replay    -in $base -column 0 -row 4 -sticky w -columnspan 2
#    grid $base.recreplay -in $base -column 0 -row 5 -sticky w -columnspan 2
#    grid $base.filename  -in $base -column 1 -row 1 -sticky ew -padx 2
#    grid $base.browse    -in $base -column 1 -row 2 -sticky e  -padx 2
#
#    ###########
#    # BALLOONS
#    ###########
#    balloon:set $base.none "Don't do anything with the media on the channels"
#    balloon:set $base.rec "Record incoming media to a file"
#    balloon:set $base.play "Play a file on the outgoing media channels"
#    balloon:set $base.replay "Replay incoming media channels"
#    balloon:set $base.recreplay "Record incoming media to a file and replay them at once"
#
#    #########
#    # OTHERS
#    #########
#    $base.none select
#}
#
#
# media:browse
# Open file lise to select a file for read/write
# input  : none
# output : none
# return : none
#proc media:browse {} {
#    global tmp app
#
#    if {($app(media,rtpMode) == 0) || ($app(media,rtpMode) == 1)} {
#        # Open a file for playback
#        set app(media,filename) [tk_getOpenFile \
#            -initialfile $app(media,filename) \
#            -defaultextension "rec" \
#            -filetypes {{ "Recodrings" {*.rec} }} \
#            -title "Playback file"]
#    } else {
#        # Open a file for record
#        set app(media,filename) [tk_getSaveFile \
#            -initialfile $app(media,filename) \
#            -defaultextension "rec" \
#            -filetypes {{ "Recordings" {*.rec} }} \
#            -title "Record file"]
#    }
#}

##############################################################################################
#
#   Accept/Reject capability messages
#
##############################################################################################

# CapResponse
# sending response to capability message
# it is a function that only capability:create is using
# input  : none
# output : none
# return : none
proc CapResponse {callNum Response} {
    api:cm:CallCapResp [.capability$callNum.callInfo cget -text] $Response
}

# capability:create
# Creation procedure for accepting/rejecting capability window
# This window is opened whenever the destination endpoint sends H245 capability message
# input  : none
# output : none
# return : none
proc capability:create {callNum} {

    inchan:create  .capability$callNum "Capability Accept/Reject"
    # Change the commands of the buttons
    .capability$callNum.accept configure \
        -command "CapResponse $callNum ACK"
    .capability$callNum.reject configure \
        -command "CapResponse $callNum REJECT"
       # -highlightthickness 0 -text Reject \
}

