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
#                                 TAP_test.tcl
#
# This file contains the main window GUI along with some of its procedures, which are not
# related to any other specific window (such as calls, tools, etc).
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
#   MAIN WINDOW operations
#
##############################################################################################


# test:create
# This procedure creates the main window of the test application
# input  : none
# output : none
# return : none
proc test:create {} {
    global tmp app

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .test -class Toplevel -menu .test.main
    wm focusmodel .test passive
    wm geometry .test $app(test,size)
    wm overrideredirect .test 0
    wm resizable .test 1 1
    wm deiconify .test
    wm title .test $tmp(version)
    wm protocol .test WM_DELETE_WINDOW {
        set tmp(test,size) [winfo geometry .test]
        test.Quit
    }

    # Create the menu for this window
    test:createMenu

    # Tool buttons
    # todo : add edit script tool
    frame .test.line1 -relief sunken -border 1 -height 2
    frame .test.tools
    button .test.tools.log -relief flat -border 1 -command {Window toggle .log} -image LogBut -state disabled -tooltip "Stack Log"
    button .test.tools.config -relief flat -border 1 -command {config:open} -image ConfigBut -tooltip "Open Configuration File"
    button .test.tools.logFile -relief flat -border 1 -command {logfile:open} -image LogFileBut -tooltip "Open Log File"
    button .test.tools.status -relief flat -border 1 -command {Window toggle .status} -image StatusBut -tooltip "Resource Status (Ctrl-s)"
    button .test.tools.raise -relief flat -border 1 -command {focus .dummy} -image RaiseBut -tooltip "Raise Windows"
    label .test.tools.marker1 -relief sunken -border 1 -padx 0
    button .test.tools.execute -relief flat -border 1 -command {script:load} -image ExecuteBut -tooltip "Execute an Advanced Script"
    button .test.tools.stop -relief flat -border 1 -command {script:stop} -image StopBut -tooltip "Stop executing a running script"
    # top bar
    image create photo topbar -format gif -data $tmp(topbarFade)
    label .test.tools.topbar -image topbar -borderwidth 0 -anchor e
    frame .test.line2 -relief sunken -border 1 -height 2

    # Calls listbox
    proc callYviewScrl {args} {
        eval ".test.calls.list yview $args"
        eval ".test.calls.conf yview $args"
        eval ".test.calls.conn yview $args"
    }

    frame .test.calls -borderwidth 2 -relief groove
    listbox .test.calls.list -background White \
        -selectmode single -exportselection 0 -height 3 \
        -yscrollcommand {.test.calls.scrl set}
    listbox .test.calls.conf -background White \
        -selectmode single -exportselection 0 -height 3 -width 1\
        -yscrollcommand {.test.calls.scrl set} -tooltip "Conference"
    listbox .test.calls.conn -background White \
        -selectmode single -exportselection 0 -height 3 -width 1\
        -yscrollcommand {.test.calls.scrl set} -tooltip "Host"
    scrollbar .test.calls.scrl -command {callYviewScrl}

    # Channels listbox
    frame .test.chan -borderwidth 2 -relief groove
    channel:createChannelsInfo .test.chan 1

    # Tabs Section
    set tmp(test,otherTabs) { 1 2 3 4 5 6 7 }

    set tabs [
        notebook:create test .test {
            { "Basic"   test:basicTab  }
            { "Call"    test:callTab   }
            { "Channel" test:chanTab   }
            { "RAS"     test:rasTab    }
            { "Misc."   test:miscTab   }
            { "Options" test:optionTab }
            { "H.450"   test:h450Tab   }
        }
    ]

    # Messages
    frame .test.msg -borderwidth 2 -relief groove
    listbox .test.msg.list -selectmode single -exportselection 0 -height 1 -background White \
        -yscrollcommand {.test.msg.yscrl set} -xscrollcommand {.test.msg.xscrl set}
    scrollbar .test.msg.xscrl -orient horizontal -command {.test.msg.list xview}
    scrollbar .test.msg.yscrl -command {.test.msg.list yview}
    button .test.msg.clear -borderwidth 2 -command {.test.msg.list delete 0 end} \
        -highlightthickness 0 -padx 0 -pady 0 -text "Clear" -image bmpClear -tooltip "Clear message box (Ctrl-m)"

    # Status bar
    frame .test.status -borderwidth 0 -relief flat
    label .test.status.calls -borderwidth 1 -relief sunken -anchor w -tooltip "Number of current connected calls, Number of total calls connected"
    label .test.status.h245 -borderwidth 1 -relief sunken -width 12 -tooltip "H245 address passing Stage"
    label .test.status.gk -borderwidth 1 -relief sunken -width 12 -tooltip "Gatekeeper registration status"
    label .test.status.stack -borderwidth 1 -relief sunken -width 10 -tooltip "Stack's current execution status"
    label .test.status.mode -borderwidth 1 -relief sunken -width 7 -tooltip "Mode of the application: Normal mode, or Script mode"
    label .test.status.timer -borderwidth 1 -relief sunken -padx 3 -tooltip "Time passed since beginning of execution (in minutes)"

    # Manual frame
    frame .test.manual -relief groove -borderwidth 0

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf .test 0 -weight 1
    grid columnconf .test 1 -weight 0 -minsize 300
    grid rowconf .test 3 -weight 0 -minsize 6
    grid rowconf .test 4 -weight 0 -minsize 150
    grid rowconf .test 5 -weight 0 -minsize 7
    grid rowconf .test 6 -weight 0 -minsize 150
    grid rowconf .test 7 -weight 0 -minsize 5
    grid rowconf .test 8 -weight 1

    # Displaying the tool buttons
    grid .test.line1 -in .test -column 0 -row 0 -columnspan 3 -sticky new -pady 1
    grid .test.tools -in .test -column 0 -row 0 -columnspan 3 -sticky ew -pady 4
    pack .test.tools.log -in .test.tools -ipadx 2 -side left
    pack .test.tools.logFile -in .test.tools -ipadx 2 -side left
    pack .test.tools.config -in .test.tools -ipadx 2 -side left
    pack .test.tools.status -in .test.tools -ipadx 2 -side left
    pack .test.tools.raise -in .test.tools -ipadx 2 -side left
    pack .test.tools.marker1 -in .test.tools -pady 2 -padx 1 -side left -fill y
    pack .test.tools.execute -in .test.tools -ipadx 2 -side left
    pack .test.tools.stop -in .test.tools -ipadx 2 -side left
    # top bar
    pack .test.tools.topbar -in .test.tools -side right
    grid .test.line2 -in .test -column 0 -row 0 -columnspan 3 -sticky sew -pady 1

    # Calls
    grid .test.calls -in .test -column 0 -row 4 -sticky nesw -pad 2
    grid columnconf .test.calls 1 -weight 1
    grid rowconf .test.calls 0 -weight 0 -minsize 3
    grid rowconf .test.calls 1 -weight 1
    grid .test.calls.conf -in .test.calls -column 0 -row 1 -sticky nesw -pady 2
    grid .test.calls.list  -in .test.calls -column 1 -row 1 -sticky nesw -pady 2
    grid .test.calls.conn  -in .test.calls -column 2 -row 1 -sticky nesw -pady 2
    grid .test.calls.scrl  -in .test.calls -column 3 -row 1 -sticky ns -pady 2

    # Channels
    grid .test.chan -in .test -column 0 -row 6 -sticky nesw -padx 2

    # Tabs Section
    grid $tabs -in .test -column 1 -row 4 -rowspan 3 -sticky nesw -pad 2

    # Messages
    grid .test.msg -in .test -column 0 -row 8 -columnspan 3 -sticky nesw -pady 4 -padx 2
    grid columnconf .test.msg 0 -weight 1
    grid rowconf .test.msg 0 -weight 0 -minsize 5
    grid rowconf .test.msg 1 -weight 1
    grid .test.msg.list -in .test.msg -column 0 -row 1 -sticky nesw -padx 1 -pady 1
    grid .test.msg.xscrl -in .test.msg -column 0 -row 2 -sticky we
    grid .test.msg.yscrl -in .test.msg -column 1 -row 1 -sticky ns
    grid .test.msg.clear -in .test.msg -column 1 -row 2 -sticky news

    # Status bar
    grid .test.status -in .test -column 0 -row 9 -columnspan 3 -sticky ew -padx 1
    grid columnconf .test.status 0 -weight 1
    grid .test.status.calls -in .test.status -column 0 -row 0 -sticky ew -padx 1
    grid .test.status.h245 -in .test.status -column 1 -row 0 -sticky ew -padx 1
    grid .test.status.gk -in .test.status -column 2 -row 0 -sticky ew -padx 1
    grid .test.status.stack -in .test.status -column 3 -row 0 -sticky ew -padx 1
    grid .test.status.mode -in .test.status -column 4 -row 0 -sticky ew -padx 1
    grid .test.status.timer -in .test.status -column 5 -row 0 -sticky ew -padx 1

    ###########
    # BINDINGS
    ###########
    bind .test <Control-Key-m> {.test.msg.clear invoke}
    bind .test <Control-Key-s> {.test.tools.status invoke}
    bind .test <Control-Key-l> {.test.tools.logFile invoke}
    bind .test <Control-Key-f> { for {set i 0} {$i<10} {incr i} {Call.Make $tmp(quick,dest)} }
    #bind .test.calls.list <Double-Button-1> {call:doubleclickedCall}
    bind .test.msg.list <<ListboxSelect>> {.test.msg.list selection clear 0 end}
    bind .test.calls.conf <<ListboxSelect>> {.test.calls.conf selection clear 0 end}
    bind .test.calls.conn <<ListboxSelect>> {.test.calls.conn selection clear 0 end}

    foreach tool {log config status raise logFile execute stop} {
        bindtags .test.tools.$tool "[bindtags .test.tools.$tool] toolbar"
    }

    ########
    # OTHER
    ########
    bind .test.calls.list <<ListboxSelect>> {+
        set item [selectedItem .test.calls.list]
        if {$item != ""} {Channel.DisplayChannelList $item}
    }
    test:SetCalls 0 0
    test:SetH245Status $app(h245,stage)
    test:updateTimer
    trace variable tmp(script,running) w test:refreshMenu
    set tmp(script,running) 0

    placeHeader .test.calls "Calls"
    placeHeader .test.msg "Messages"
}

proc test:basicTab {base tabButton} {
    global tmp app
    set tmp(basic,base) $base

    # Quick Call
    frame $base.quick -borderwidth 2 -relief groove
    button $base.quick.make -text "Make" -width 6 -highlightthickness 0 -padx 0 -pady 0 \
        -command {global tmp; Call.Make $tmp(quick,dest)} -tooltip "Make a call"
    histEnt:create $base.quick $base.quick.make quick,dest "Address or Alias to call (Return to Make)"

    # conference and connection
    frame $base.cnc -borderwidth 2 -relief groove
    checkbutton $base.cnc.useAdHoc -text "Add to conference" -variable tmp(adHoc,use) -tooltip "Add new call to conference"\
        -command {
            global tmp
            $tmp(basic,base).cnc.callAdHoc.01 delete 0 end
            if { $tmp(adHoc,use) } {
                set tmp(adHoc,call) "Select Call"
                foreach call [.test.calls.list get 0 end] {
                    $tmp(basic,base).cnc.callAdHoc.01 add radiobutton \
                        -indicatoron 0 -value $call -variable tmp(adHoc,call) -label $call
                }
            } else { set tmp(adHoc,call) "" }
        }
    menubutton $base.cnc.callAdHoc -borderwidth 1 -height 1 -width 10 -anchor w -indicatoron 1 \
        -menu $base.cnc.callAdHoc.01 -relief raised -text "" -textvariable tmp(adHoc,call) -tooltip "Select call to add to"
    menu $base.cnc.callAdHoc.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0

    checkbutton $base.cnc.useMultiplex -text "Use same connection" \
        -variable tmp(multiplex,use) -tooltip "Use same connection as existing call" \
        -command {
            global tmp
            $tmp(basic,base).cnc.callMultiplex.01 delete 0 end
            if { $tmp(multiplex,use) } {
                set tmp(multiplex,call) "Select Call"
                foreach call [.test.calls.list get 0 end] {
                    $tmp(basic,base).cnc.callMultiplex.01 add radiobutton \
                        -indicatoron 0 -value $call -variable tmp(multiplex,call) -label $call
                }
            } else { set tmp(multiplex,call) "" }
        }
    menubutton $base.cnc.callMultiplex -borderwidth 1 -height 1 -width 10 -anchor w -indicatoron 1 \
        -menu $base.cnc.callMultiplex.01 -relief raised -text "" -textvariable tmp(multiplex,call) -tooltip "Select connection to use"
    menu $base.cnc.callMultiplex.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0

    # Calls buttons
    frame $base.callBut -borderwidth 2 -relief groove
    frame $base.callBut.bw -borderwidth 1 -height 34 -relief sunken -width 58
    label $base.callBut.bw.lab -borderwidth 0 -pady 0 -text "Bandwidth" -tooltip "Set/Change the call bandwidth"
    menubutton $base.callBut.bw.bwValues \
        -borderwidth 1 -height 1 -indicatoron 1 \
        -menu $base.callBut.bw.bwValues.m -padx 0 -pady 2 \
        -relief raised -text "128000" -textvariable app(test,bw) -width 2
    menu $base.callBut.bw.bwValues.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach lab { "64000" "128000" "192000" "256000" "320000" "384000" "Multirate" } {
        $base.callBut.bw.bwValues.m add radiobutton -indicatoron 0 -value $lab \
            -variable app(test,bw) -label "$lab" -command {Call.SetRate [selectedItem .test.calls.list] $app(test,bw)}
    }
    button $base.callBut.drop \
         -highlightthickness 0 -padx 0 -pady 0 -text "Drop" -width 6 -tooltip "Terminate the selected call (Ctrl-d)" \
         -command {Call.Drop [selectedItem .test.calls.list] $app(test,callDropReason)
                after idle {call:SetButtonsState $tmp(basic,base)}
         }

    button $base.callBut.dropAll \
        -command {Call:DropAll .test.calls.list $app(test,callDropReason) $tmp(basic,base)} \
        -highlightthickness 0 -padx 0 -pady 0 -text "Drop All" -width 6 -tooltip "Terminate all the calls (Ctrl-a)"
    frame $base.callBut.reason -borderwidth 1 -relief sunken
    label $base.callBut.reason.lab -borderwidth 0 -pady 0 -text "Drop Reason" -tooltip "Set call drop/reject reason"
    menubutton $base.callBut.reason.menu \
        -borderwidth 1 -height 1 -indicatoron 1 \
        -menu $base.callBut.reason.menu.m -padx 0 -pady 2 -relief raised -text "11" \
        -textvariable app(test,callDropReason)
    menu $base.callBut.reason.menu.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach reason {
        " 0 - No bandwidth" " 1 - Gatekeeper resources" " 2 - Unreachable destination"
        " 3 - Destination rejection" " 4 - Invalid revision" " 5 - No permission"
        " 6 - Unreachable gatekeeper" " 7 - Gateway resources" " 8 - Bad format address"
        " 9 - Adaptive busy" "10 - In conf" "11 - Undefined reason" "15 - Facility call deflection"
        "16 - Security denied" "17 - Called party not registered" "18 - Caller not registered"
        "21 - New connection needed" } value { 0 1 2 3 4 5 6 7 8 9 10 11 15 16 17 18 21 } {
        $base.callBut.reason.menu.m add radiobutton -indicatoron 0 \
            -variable app(test,callDropReason) -label "$reason" -value $value
    }
    button $base.callBut.callProceeding -tooltip "Send CallProceeding Message" \
        -command {Call.SendCallProceeding [selectedItem .test.calls.list]}  \
        -highlightthickness 0 -text "Proceeding" -width 6
    button $base.callBut.alerting -tooltip "Send Alerting Message" \
        -command {Call.SendAlerting [selectedItem .test.calls.list]} \
        -highlightthickness 0 -text "Alerting" -width 6
    button $base.callBut.connect -tooltip "Send Connect message" \
        -command {Call.SendConnect [selectedItem .test.calls.list]} \
        -highlightthickness 0 -text "Connect" -width 6

    # Check buttons
    frame $base.inType -borderwidth 2 -relief groove
    checkbutton $base.inType.isFastStart   -text "Fast Start"   -variable app(newcall,fastStart)    -tooltip "Accept FastStart offer"
    checkbutton $base.inType.isTunneling   -text "Tunneling"    -variable app(newcall,tunneling)    -tooltip "Accept Tunneling offer"
    checkbutton $base.inType.parallel      -text "Parallel"     -variable app(newcall,parallel)     -tooltip "Allow both FastStart and Tunneling"
    checkbutton $base.inType.isOverlapping -text "Overlap Send" -variable app(newcall,canOvsp)      -tooltip "Use overlap receiving procedures"
    checkbutton $base.inType.sendComplete  -text "Complete"     -variable app(newcall,sendComplete) -tooltip "Signal that complete address was sent"
    checkbutton $base.inType.autoAns       -text "Auto Answer"  -variable app(newcall,autoAns)      -tooltip "Answer incoming call automatically"

    frame $base.multiplex -borderwidth 2 -relief groove
    checkbutton $base.multiplex.multiplexed   -text "Multiplexed"  -variable app(newcall,multiplexed)  -tooltip "Multiplex connections"
    checkbutton $base.multiplex.maintain      -text "Maintain"     -variable app(newcall,maintain)     -tooltip "Maintain dropped connections"
    button $base.multiplex.updt -highlightthickness 0 -padx 0 -pady 0 -text "Update" -width 7 -tooltip "Update for selected call" \
        -command {Multiplex.Update [selectedItem .test.calls.list] $app(newcall,multiplexed) $app(newcall,maintain)}

    ########
    # GRID #
    ########

    grid rowconf $base 0 -weight 0 -minsize 7
    grid rowconf $base 1 -weight 0 -minsize 22
    grid rowconf $base 2 -weight 0 -minsize 7
    grid rowconf $base 3 -weight 1
    grid rowconf $base 4 -weight 0 -minsize 7
    grid rowconf $base 5 -weight 1
    grid rowconf $base 6 -weight 0 -minsize 7
    grid rowconf $base 7 -weight 0 -minsize 22
    grid columnconf $base 0 -weight 1

    # Quick Call
    grid $base.quick -in $base -column 0 -row 1 -sticky nesw -padx 2
    grid rowconf $base.quick 0 -weight 0 -minsize 7
    grid columnconf $base.quick 1 -weight 1
    grid $base.quick.make -in $base.quick -column 0 -row 1 -padx 4
    grid $base.quick.histEnt -in $base.quick -column 1 -row 1 -sticky ew -padx 3 -ipady 1 -pady 3

    # conference and connection
    grid $base.cnc -in $base -column 0 -row 3 -sticky nesw -padx 2
    grid rowconf $base.cnc 0 -weight 0 -minsize 2
    grid columnconf $base.cnc 1 -weight 1
    grid $base.cnc.useAdHoc -in $base.cnc  -column 0 -row 1 -sticky w -padx 4
    grid $base.cnc.callAdHoc -in $base.cnc -column 1 -row 1 -sticky ew -padx 2
    grid $base.cnc.useMultiplex -in $base.cnc  -column 0 -row 2 -sticky w -padx 4
    grid $base.cnc.callMultiplex -in $base.cnc -column 1 -row 2 -sticky ew -padx 2

    # Call buttons
    grid $base.callBut -in $base -column 0 -row 5 -sticky nesw -padx 2
    grid rowconf $base.callBut 0 -weight 0 -minsize 5
    grid rowconf $base.callBut 1 -weight 1
    grid rowconf $base.callBut 2 -weight 1
    grid rowconf $base.callBut 3 -weight 1
    grid columnconf $base.callBut 0 -weight 1
    grid columnconf $base.callBut 1 -weight 1
    grid columnconf $base.callBut 2 -weight 1
    grid $base.callBut.bw             -in $base.callBut        -column 0 -row 1 -columnspan 2 -sticky we -padx 6
    grid columnconf $base.callBut.bw 1 -weight 1
    grid $base.callBut.bw.lab           -in $base.callBut.bw     -column 0 -row 0 -padx 2
    grid $base.callBut.bw.bwValues      -in $base.callBut.bw     -column 1 -row 0 -sticky we
    grid $base.callBut.reason         -in $base.callBut        -column 0 -row 2 -columnspan 2  -padx 6 -sticky we
    grid $base.callBut.drop           -in $base.callBut        -column 0 -row 3 -sticky we  -padx 6
    grid $base.callBut.dropAll        -in $base.callBut        -column 1 -row 3 -sticky we  -padx 6
    grid columnconf $base.callBut.reason 1 -weight 1
    grid $base.callBut.reason.lab       -in $base.callBut.reason -column 0 -row 0 -padx 2
    grid $base.callBut.reason.menu      -in $base.callBut.reason -column 1 -row 0 -sticky we
    grid $base.callBut.callProceeding -in $base.callBut        -column 2 -row 1 -sticky we -padx 6
    grid $base.callBut.alerting       -in $base.callBut        -column 2 -row 2 -sticky we -padx 6
    grid $base.callBut.connect        -in $base.callBut        -column 2 -row 3 -sticky we -padx 6

    # Check buttons
    grid $base.inType -in $base -column 1 -row 1 -rowspan 7 -sticky nesw -padx 2 -ipady 0
    grid columnconf $base.inType 0 -weight 1
    grid rowconf $base.inType 0 -minsize 5
    foreach i {4 6} {grid rowconf $base.inType $i -weight 1}
    grid $base.inType.isFastStart   -in $base.inType -column 0 -row 1 -padx 2 -sticky sw
    grid $base.inType.isTunneling   -in $base.inType -column 0 -row 2 -padx 2 -sticky sw
    grid $base.inType.parallel      -in $base.inType -column 0 -row 3 -padx 2 -sticky sw
    grid $base.inType.isOverlapping -in $base.inType -column 0 -row 4 -padx 2 -sticky sw
    grid $base.inType.sendComplete  -in $base.inType -column 0 -row 5 -padx 2 -sticky sw
    grid $base.inType.autoAns       -in $base.inType -column 0 -row 6 -padx 2 -sticky sw

    # Multiplex
    grid $base.multiplex -in $base -column 0 -row 7 -sticky nesw -padx 2
    grid rowconf $base.multiplex 0 -minsize 5
    grid rowconf $base.multiplex 1 -weight 1
    grid columnconf $base.multiplex 0 -weight 1
    grid columnconf $base.multiplex 1 -weight 1
    grid columnconf $base.multiplex 2 -weight 1
    grid $base.multiplex.multiplexed -in $base.multiplex -column 0 -row 1
    grid $base.multiplex.maintain -in $base.multiplex -column 1 -row 1
    grid $base.multiplex.updt -in $base.multiplex -column 2 -row 1

    ########
    # BIND #
    ########

    bind $base.quick.histEnt.name <Return> "$base.quick.make invoke"
    bind .test <Control-Key-d> "$base.callBut.drop invoke"
    bind .test <Control-Key-a> "$base.callBut.dropAll invoke"

    placeHeader $base.quick "Quick Call"
    placeHeader $base.cnc "Conference & Connection"
    placeHeader $base.callBut "Call Handling"
    placeHeader $base.inType "Call Type"
    placeHeader $base.multiplex "Multiplex"

    bind .test.calls.list <<ListboxSelect>> "+
            call:SetButtonsState $base"

    call:SetButtonsState $base
}

proc test:callTab {base tabButton} {
    global tmp app

    set tmp(callTab) $base

    ###################
    # CREATING WIDGETS
    ###################
    set tmp(newcall,IsMultiRate) 0

    # Action buttons
    frame $base.actions -borderwidth 2 -relief groove
    button $base.actions.okBut -highlightthickness 0 -padx 0 -pady 0 -text Dial -width 6 \
        -command "Call.Dial $base.aliasList.txt" -tooltip "Dial a call"
    # Dest IP
    frame $base.actions.destIP -borderwidth 0 -relief flat

    # Aliases
    frame $base.destAlias -borderwidth 2 -relief groove
    frame $base.aliasList -borderwidth 2 -relief groove

    # Call information
    frame $base.callInfo -borderwidth 2 -relief groove
    label $base.callInfo.dispLab -borderwidth 1 -text "Display"
    entry $base.callInfo.dispInfo -borderwidth 1 -textvariable app(options,display) \
        -validate key -vcmd { expr { [string length %P] < 131 } } -invcmd bell
    label $base.callInfo.uuLab -borderwidth 1 -text "UserUser"
    entry $base.callInfo.uuInfo -borderwidth 1 -textvariable app(options,userUser) \
        -validate key -vcmd { expr { [string length %P] < 131 } } -invcmd bell

    # Check buttons
    frame $base.outType -borderwidth 2 -relief groove
    checkbutton $base.outType.isFastStart   -text "Fast Start"   -variable app(newcall,fastStart)    -tooltip "Open channels with 'setup' message, no H245 session"
    checkbutton $base.outType.isTunneling   -text "Tunneling"    -variable app(newcall,tunneling)    -tooltip "Pass H245 messages tunneled in Q.931 messages"
    checkbutton $base.outType.parallel      -text "Parallel"     -variable app(newcall,parallel)     -tooltip "Allow both FastStart and Tunneling"
    checkbutton $base.outType.isOverlapping -text "Overlap Send" -variable app(newcall,canOvsp)      -tooltip "Use overlap sending procedures"
    checkbutton $base.outType.sendComplete  -text "Complete"     -variable app(newcall,sendComplete) -tooltip "Signal that complete address was sent"
    checkbutton $base.outType.autoAns       -text "Auto Answer"  -variable app(newcall,autoAns)      -tooltip "Answer incoming call automatically"

    # Multiplex
    frame $base.multiplex -borderwidth 2 -relief groove
    checkbutton $base.multiplex.multiplexed -text "Multiplexed"  -variable app(newcall,multiplexed)  -tooltip "Multiplex connections"
    checkbutton $base.multiplex.maintain -text "Maintain"  -variable app(newcall,maintain)           -tooltip "Maintain dropped connections"
    button $base.multiplex.updt -highlightthickness 0 -padx 0 -pady 0 -text "Update" -width 7 \
        -command "Multiplex.Update $app(newcall,multiplexed) $app(newcall,maintain) " -tooltip "Update for selected call"

#    frame $base.rtp -borderwidth 2 -relief groove

    # RTP/RTCP record and playback
#    media:create $base.rtp

    alias:createEntry $base $base.destAlias $base.aliasList "Destination Aliases" "Destination"
    ip:createEntry "newcall" $base.actions.destIP $base.actions.okBut newcall,address

    ###########
    # BINDINGS
    ###########
    bind $base.actions.destIP.histEnt.name <Return> "$base.actions.okBut invoke"
    bind $base.destAlias.name <Double-Return> "$base.actions.okBut invoke"

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 1 -weight 1 -minsize 100
    grid rowconf $base 0 -weight 0 -minsize 7
    grid rowconf $base 1 -weight 0 -minsize 22
    grid rowconf $base 2 -weight 0 -minsize 7
    grid rowconf $base 3 -weight 1 -minsize 0
    grid rowconf $base 4 -weight 0 -minsize 7
    grid rowconf $base 5 -weight 1 -minsize 0
    grid rowconf $base 6 -weight 0 -minsize 7
    grid rowconf $base 7 -weight 0 -minsize 22

    grid $base.actions -in $base -column 0 -columnspan 2 -row 1 -sticky nesw -padx 2
    grid rowconf $base.actions 0 -minsize 7
    grid rowconf $base.actions 1 -weight 1
    grid columnconf $base.actions 1 -weight 1
    grid $base.actions.okBut    -in $base.actions -column 0 -row 1 -padx 4
    grid $base.actions.destIP -in $base.actions -column 1 -row 1 -sticky news -ipady 2 -padx 2 -pady 1

    grid $base.destAlias -in $base -column 0 -row 3 -columnspan 2 -sticky news -ipady 1 -padx 2
    grid $base.aliasList -in $base -column 1 -row 5 -sticky nesw -padx 2


    # Call information
#    grid $base.rtp -in $base -column 0 -row 5 -sticky news -padx 2
    grid $base.callInfo -in $base -column 0 -row 5 -sticky news -padx 2

    grid columnconf $base.callInfo 0 -weight 1
    grid rowconf $base.callInfo 0 -minsize 5
    grid rowconf $base.callInfo 3 -minsize 7
    grid $base.callInfo.dispLab    -in $base.callInfo -column 0 -row 1 -padx 3 -sticky w
    grid $base.callInfo.dispInfo   -in $base.callInfo -column 0 -row 2 -ipady 2 -padx 6 -pady 2 -sticky ew -columnspan 2
    grid $base.callInfo.uuLab      -in $base.callInfo -column 0 -row 4 -padx 3 -sticky w
    grid $base.callInfo.uuInfo     -in $base.callInfo -column 0 -row 5 -ipady 2 -padx 6 -pady 2 -sticky ew -columnspan 2

    # Check buttons
    grid $base.outType -in $base -column 2 -row 1 -rowspan 7 -sticky nesw -padx 2
    grid columnconf $base.outType 0 -weight 1
    grid rowconf $base.outType 0 -minsize 5
    foreach i {4 6} {grid rowconf $base.outType $i -weight 1}
    grid $base.outType.isFastStart   -in $base.outType -column 0 -row 1 -padx 2  -sticky sw
    grid $base.outType.isTunneling   -in $base.outType -column 0 -row 2 -padx 2  -sticky sw
    grid $base.outType.parallel      -in $base.outType -column 0 -row 3 -padx 2  -sticky sw
    grid $base.outType.isOverlapping -in $base.outType -column 0 -row 4 -padx 2  -sticky sw
    grid $base.outType.sendComplete  -in $base.outType -column 0 -row 5 -padx 2  -sticky sw
    grid $base.outType.autoAns       -in $base.outType -column 0 -row 6 -padx 2  -sticky sw

    # Multiplex
    grid $base.multiplex -in $base -column 0 -columnspan 2 -row 7 -sticky nesw -padx 2
    grid rowconf $base.multiplex 0 -minsize 5
    grid rowconf $base.multiplex 1 -weight 1
    grid columnconf $base.multiplex 0 -weight 1
    grid columnconf $base.multiplex 1 -weight 1
    grid columnconf $base.multiplex 2 -weight 1
    grid $base.multiplex.multiplexed -in $base.multiplex -column 0 -row 1
    grid $base.multiplex.maintain -in $base.multiplex -column 1 -row 1
    grid $base.multiplex.updt -in $base.multiplex -column 2 -row 1

    ###########
    # BALLOONS
    ###########
    balloon:set $base.destAlias.menBox "Set alias type"
    balloon:set $base.actions.destIP.histEnt.name "Address to call (Return to Dial)"

    ########
    # OTHER
    ########
    placeHeader $base.actions "Destination IP"
    placeHeader $base.destAlias "Destination Alias"
    placeHeader $base.outType "Call Type"
    placeHeader $base.multiplex "Multiplex"
#    placeHeader $base.rtp "RTP/RTCP"
    placeHeader $base.callInfo "Call Information"
}

proc test:chanTab {base tabButton} {
    global tmp app

    set tmp(chanTab) $base

    # Channels buttons
    frame $base.chanBut -borderwidth 2 -relief groove
    button $base.chanBut.new -tooltip "Create a new channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Open New" -width 11 \
        -command {Channel.ConnectOutgoing [selectedItem .test.calls.list] "" }
    button $base.chanBut.same -tooltip "Create a channel in the same session as selected incoming one" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Same Session" -width 11 \
        -command {Channel.ConnectOutgoing [selectedItem .test.calls.list] "SameAs" [selectedItem .test.chan.inList] }
    button $base.chanBut.replace -tooltip "Replace selected outgoing channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Replace" -width 11 \
        -command {Channel.ConnectOutgoing [selectedItem .test.calls.list] "Replace" [selectedItem .test.chan.outList] }
    button $base.chanBut.answer -tooltip "Answer selected incoming channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Answer" -width 11 \
        -command {Channel.Answer [selectedItem .test.chan.inList]}
    button $base.chanBut.drop -tooltip "Terminate selected incoming and outgoing channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Drop" -width 3 \
        -command {Channel.Drop [selectedItem .test.chan.inList] [selectedItem .test.chan.outList] $tmp(test,chanDropReason)}
    button $base.chanBut.dropIn -tooltip "Terminate selected incoming channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "In" -width 2 \
        -command {Channel.Drop [selectedItem .test.chan.inList] "" $tmp(test,chanDropReason)}
    button $base.chanBut.dropOut -tooltip "Terminate selected outgoing channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Out" -width 2 \
        -command {Channel.Drop "" [selectedItem .test.chan.outList] $tmp(test,chanDropReason)}
    frame $base.chanBut.reason -borderwidth 1 -relief sunken
    label $base.chanBut.reason.lab -borderwidth 0 -pady 0 -text "Drop Reason" -tooltip "Set channel drop/reject reason"
    menubutton $base.chanBut.reason.menu \
        -borderwidth 1 -height 1 -indicatoron 1 -width 3 \
        -menu $base.chanBut.reason.menu.m -padx 0 -pady 2 -relief raised -text "0" \
        -textvariable tmp(test,chanDropReason)
    menu $base.chanBut.reason.menu.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach reason { "0 - UNKNOWN" "1 - REOPEN" "2 - RESERVATION FAILURE" "3 - NORMAL" } value {0 1 2 3} {
        $base.chanBut.reason.menu.m add radiobutton -indicatoron 0 \
            -variable tmp(test,chanDropReason) -label "$reason" -value $value
    }
    button $base.chanBut.loop -tooltip "Activate media loop for selected channel"\
        -highlightthickness 0 -padx 0 -pady 0 -text "Media Loop" -width 11 \
        -command {Channel.MediaLoop [selectedItem .test.chan.outList]}
    button $base.chanBut.setRate \
        -highlightthickness 0 -padx 0 -pady 0 -text "Rate" -width 4 \
        -command {Channel.Rate $app(chan,rate) [selectedItem .test.chan.outList] [selectedItem .test.chan.inList]}
    entry $base.chanBut.rate -width 6 -textvariable app(chan,rate) \
            -validate key -invcmd bell -vcmd { string is digit %P }
    # Channel Type
    frame $base.chanBut.chanprm -borderwidth 1 -relief sunken
    label $base.chanBut.chanprm.dataTypelab -borderwidth 0 -padx 0 -pady 0 -text "Data Type"
    menubutton $base.chanBut.chanprm.dataTypemen -tooltip "Select data type" \
        -borderwidth 1 -height 1 -indicatoron 1 -width 7 \
        -menu $base.chanBut.chanprm.dataTypemen.01 -padx 0 -pady 2 -relief raised \
        -textvariable tmp(newchan,dataType)
    menu $base.chanBut.chanprm.dataTypemen.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0

    # Channels checkboxes
    frame $base.chanBut.chekBut -borderwidth 0
    checkbutton $base.chanBut.chekBut.autoAccept -tooltip "Automatically accept every incoming channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Auto accept channels" -variable app(options,autoAccept)
    checkbutton $base.chanBut.chekBut.autoDrop -tooltip "Automatically drop every channel on request from remote side" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Auto drop channels" -variable app(options,autoDrop)
    checkbutton $base.chanBut.chekBut.autoAcceptFs -tooltip "Automatically accept every faststart channel proposal" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Auto faststart" -variable app(options,autoAcceptFs)
    checkbutton $base.chanBut.chekBut.autoMimic -tooltip "Automatically open the same channel from the local side on bidirectional channels" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Auto mimic channels" -variable app(options,autoMimic)
    checkbutton $base.chanBut.chekBut.replay -tooltip "Replay the incoming media on the outgoing channel" \
        -highlightthickness 0 -padx 0 -pady 0 -text "Replay Media" -variable app(options,replayMedia)

    # Manual: H245 buttons
    frame $base.h245 -borderwidth 2 -relief groove
    button $base.h245.create -tooltip "Create an H245 session for a call (fill address)" \
        -highlightthickness 1 -text "Create" -width 6 -borderwidth 1 \
        -command { Call.CreateH245 [selectedItem .test.calls.list] $tmp(h245,address) }
    entry $base.h245.address -width 10 -textvariable tmp(h245,address) -tooltip "Remote H245 address to connect to (fill for Create / filled by listen)"
    set tmp(h245,address) "0.0.0.0:0"
    button $base.h245.connect -tooltip "Connect H245 control session for selected call" \
        -highlightthickness 1 -text "Connect" -width 6 -borderwidth 1 \
        -command {Call.ConnectControl [selectedItem .test.calls.list]}
    button $base.h245.seperate -tooltip "Separate H245 control session for selected call" \
        -highlightthickness 1 -text "Separate" -width 6 -borderwidth 1 \
        -command {Call.SeperateControl [selectedItem .test.calls.list]}
    button $base.h245.close -tooltip "Close H245 control session for selected call" \
        -highlightthickness 1 -text "Close" -width 6 -borderwidth 1 \
        -command {Call.CloseControl [selectedItem .test.calls.list]}
    button $base.h245.caps -tooltip "Send capability message on selected call" \
        -highlightthickness 1  -text "TCS" -width 6 -borderwidth 1 \
        -command {Call.SendCaps [selectedItem .test.calls.list]}
    button $base.h245.emptyCaps -tooltip "Send empty capability message on selected call" \
        -highlightthickness 1  -text "ECS" -width 6 -borderwidth 1 \
        -command {Call.SendEmptyCaps [selectedItem .test.calls.list]}
    button $base.h245.capsAck -tooltip "Send capability acknowledgement on selected call" \
        -highlightthickness 1  -text "TCSAck" -width 6 -borderwidth 1 \
        -command {Call.SendCapsAck [selectedItem .test.calls.list]}
    button $base.h245.ms -tooltip "Initiate Master/Slave transaction" \
        -highlightthickness 1  -text "M / S" -width 6 -borderwidth 1 \
        -command {Call.SendMSD [selectedItem .test.calls.list]}
    button $base.h245.loopOff -tooltip "Cancel media loop for all channels in selected call" \
        -command {Call.LoopOff [selectedItem .test.calls.list]} \
        -highlightthickness 1 -text "Loop Off" -borderwidth 1 -width 6
#    button $base.h245.uii -tooltip "Send User Input Indication" \
#        -command {Call.UII [selectedItem .test.calls.list] $tmp(h245,ui) } \
#        -highlightthickness 1 -text "UII" -borderwidth 1 -width 6
    entry $base.h245.userInput -tooltip "Press <Enter> to send User Input on selected call" \
        -width 6 -textvariable tmp(h245,ui)
    frame $base.h245.stage -borderwidth 1 -relief sunken -tooltip "Select stage to pass H245 address in"
    label $base.h245.stage.lab -borderwidth 0 -pady 5 -padx 4 -text "Stage"
    menubutton $base.h245.stage.men -borderwidth 1 -height 1 -indicatoron 1 -width 8 \
        -menu $base.h245.stage.men.m -padx 0 -pady 2 -relief raised -text "setup" \
        -textvariable app(h245,stage)
    menu $base.h245.stage.men.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach stage { "setup" "call proceeding" "alerting" "connect" "early" "facility" "no H245" } {
        $base.h245.stage.men.m add radiobutton -indicatoron 0 \
            -variable app(h245,stage) -label "$stage" -value $stage -command { test:SetH245Status $app(h245,stage) }
    }

    ########
    # GRID #
    ########

    grid rowconf $base 0 -weight 0 -minsize 7
    grid rowconf $base 1 -weight 4
    grid rowconf $base 2 -weight 0 -minsize 7
    grid rowconf $base 3 -weight 1
    grid columnconf $base 0 -weight 1

    # Channel buttons
    grid $base.chanBut -in $base -column 0 -row 1 -sticky nesw -padx 2
    grid rowconf $base.chanBut 0 -weight 0 -minsize 5
    grid rowconf $base.chanBut 1 -weight 1
    grid rowconf $base.chanBut 2 -weight 1
    grid rowconf $base.chanBut 3 -weight 1
    grid rowconf $base.chanBut 4 -weight 1
    grid columnconf $base.chanBut 4 -weight 1

    grid $base.chanBut.new         -in $base.chanBut -column 0 -row 1 -padx 4
    grid $base.chanBut.same        -in $base.chanBut -column 1 -row 1 -padx 4
    grid $base.chanBut.replace     -in $base.chanBut -column 2 -row 1 -padx 4
    grid $base.chanBut.answer      -in $base.chanBut -column 0 -row 3 -padx 4
    grid $base.chanBut.loop        -in $base.chanBut -column 1 -row 3 -padx 4
    grid $base.chanBut.setRate     -in $base.chanBut -column 2 -row 3 -padx 4 -sticky w
    grid $base.chanBut.rate        -in $base.chanBut -column 2 -row 3 -padx 4 -sticky e -ipady 2
    grid $base.chanBut.reason      -in $base.chanBut -column 0 -columnspan 2 -row 4 -sticky ew -padx 4
    grid $base.chanBut.dropIn      -in $base.chanBut -column 2 -row 4 -padx 4 -sticky w
    grid $base.chanBut.dropOut     -in $base.chanBut -column 2 -row 4 -padx 4 -sticky e
    grid $base.chanBut.drop        -in $base.chanBut -column 2 -row 4 -padx 4
    grid columnconf $base.chanBut.reason 1 -weight 1
    grid $base.chanBut.reason.lab  -in $base.chanBut.reason -column 0 -row 0 -padx 2
    grid $base.chanBut.reason.menu -in $base.chanBut.reason -column 1 -row 0 -sticky ew
    # Channel Type
    grid $base.chanBut.chanprm -in $base.chanBut -column 0 -row 2 -columnspan 3 -sticky ew -padx 4
    grid columnconf $base.chanBut.chanprm 1 -weight 1
    grid $base.chanBut.chanprm.dataTypelab -in $base.chanBut.chanprm -column 0 -row 0 -padx 2
    grid $base.chanBut.chanprm.dataTypemen -in $base.chanBut.chanprm -column 1 -row 0 -sticky ew
    # Channels checkboxes
    grid $base.chanBut.chekBut     -in $base.chanBut -column 4 -row 0 -rowspan 5 -sticky nes
    grid rowconf $base.chanBut.chekBut 0 -weight 1
    grid rowconf $base.chanBut.chekBut 1 -weight 1
    grid rowconf $base.chanBut.chekBut 2 -weight 1
    grid rowconf $base.chanBut.chekBut 3 -weight 1
    grid rowconf $base.chanBut.chekBut 4 -weight 1
    grid columnconf $base.chanBut.chekBut 0 -weight 1
    grid $base.chanBut.chekBut.autoAccept   -in $base.chanBut.chekBut -column 0 -row 0 -sticky w
    grid $base.chanBut.chekBut.autoDrop     -in $base.chanBut.chekBut -column 0 -row 1 -sticky w
    grid $base.chanBut.chekBut.autoAcceptFs -in $base.chanBut.chekBut -column 0 -row 2 -sticky w
    grid $base.chanBut.chekBut.autoMimic    -in $base.chanBut.chekBut -column 0 -row 3 -sticky w
    grid $base.chanBut.chekBut.replay       -in $base.chanBut.chekBut -column 0 -row 4 -sticky w

    # Manual: H245 buttons
    grid $base.h245 -in $base -column 0 -row 3 -sticky nesw -padx 2
    grid rowconf $base.h245 0 -weight 0 -minsize 6
    grid $base.h245.connect   -in $base.h245 -column 0 -row 1 -padx 6 -pady 4
    grid $base.h245.seperate  -in $base.h245 -column 1 -row 1 -padx 6 -pady 4
    grid $base.h245.close     -in $base.h245 -column 2 -row 1 -padx 6 -pady 4
    grid $base.h245.create    -in $base.h245 -column 0 -row 2 -padx 6 -pady 4
    grid $base.h245.address   -in $base.h245 -column 1 -row 2 -padx 6 -pady 4 -sticky ew -columnspan 2 -ipady 3
    grid $base.h245.stage     -in $base.h245 -column 3 -row 1 -padx 6 -pady 4 -sticky ew -columnspan 2
#    grid $base.h245.uii       -in $base.h245 -column 3 -row 2 -padx 6 -pady 4
    grid $base.h245.userInput -in $base.h245 -column 3 -row 2 -padx 6 -pady 4 -ipady 3
    grid $base.h245.loopOff   -in $base.h245 -column 4 -row 2 -padx 6 -pady 4
    grid $base.h245.caps      -in $base.h245 -column 5 -row 1 -padx 3 -pady 4
    grid $base.h245.emptyCaps -in $base.h245 -column 6 -row 1 -padx 3 -pady 4
    grid $base.h245.capsAck   -in $base.h245 -column 5 -row 2 -padx 3 -pady 4
    grid $base.h245.ms        -in $base.h245 -column 6 -row 2 -padx 6 -pady 4
    grid columnconf $base.h245.stage 1 -weight 1
    grid $base.h245.stage.lab -in $base.h245.stage -column 0 -row 0
    grid $base.h245.stage.men -in $base.h245.stage -column 1 -row 0 -sticky nsew

    ########
    # BIND #
    ########
    placeHeader $base.chanBut "Channel Handling"
    placeHeader $base.h245 "H.245"
    bind .test.calls.list <<ListboxSelect>> "+
            call:SetChanButtonsState $base"
    bind .test.chan.inList <<ListboxSelect>> "+
            channel:SetButtonsState $base"
    bind .test.chan.outList <<ListboxSelect>> "+
            channel:SetButtonsState $base"
    channel:SetButtonsState $base
    call:SetChanButtonsState $base

    bind $base.h245.userInput <Key-Return> {
        Call.UII [selectedItem .test.calls.list] $tmp(h245,ui)
    }
}


proc test:rasTab {base tabButton} {
    global tmp app

    set tmp(rasTab) $base

    # Manual: RAS buttons
    frame $base.ras -borderwidth 2 -relief groove
    button $base.ras.rrq -tooltip "Send RegistrationRequest/GatekeeperRequest message (ctrl-r)" \
        -command rgrq:register -highlightthickness 0 -pady 0 -text "Register" -width 11 -borderwidth 1
    button $base.ras.urq -tooltip "Send UnregistrationRequest message (Ctrl-u)" \
        -highlightthickness 0 -pady 0 -text "Unregister" -width 11 -borderwidth 1 -highlightthickness 0 \
        -command {api:cm:Unregister}
    button $base.ras.clear -borderwidth 1 -highlightthickness 0 -pady 0 -text Clear -width 11 \
        -tooltip "Clear all (Ctrl-c)" \
        -command "$base.rglist.txt delete 0 end
                  $base.rgsource.name delete 0 end
                  $base.rgGK.histEnt.name delete 0 end
                  $base.rgGK.entAlias delete 0 end"
    button $base.ras.reset -borderwidth 1 -highlightthickness 0 -pady 0 -text Reset -width 11 \
            -command rgrq:GetRegInfo -tooltip "Reset registration information by current configuration settings"
    button $base.ras.bw    -borderwidth 1 -highlightthickness 0 -pady 0 -text BandWidth -width 11 \
            -command {RAS.SendBRQ [selectedItem .test.calls.list] $app(ras,bw)} -tooltip "Send Call Rate Request for the selected bandwidth"
    menubutton $base.ras.bwValues -borderwidth 1 -height 1 -indicatoron 1 -menu $base.ras.bwValues.m \
            -padx 0 -pady 2 -relief raised -text "1280" -textvariable app(ras,bw) -width 2 -tooltip "Select Bandwidth"
    menu $base.ras.bwValues.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach lab { "64000" "128000" "192000" "256000" "320000" "384000" "Multirate" } {
        $base.ras.bwValues.m add radiobutton -indicatoron 0 -value $lab -variable app(ras,bw) -label $lab
    }
    button $base.ras.nsm -borderwidth 1 -highlightthickness 0 -pady 0 -text NSM -width 11 \
            -command {RAS.SendNSM [nsd:getParam]} -tooltip "Send RAS Non Standard Message"
    checkbutton $base.ras.rai -borderwidth 1 -highlightthickness 0 -pady 0 -text AOOR \
            -variable tmp(ras,aoor) -command {RAS.SendRAI $tmp(ras,aoor)} -tooltip "Signal Out Of Resources on or off"
    checkbutton $base.ras.nrr -borderwidth 1 -highlightthickness 0 -pady 0 -text "NoReReg" \
            -variable app(ras,rereg) -tooltip "If checked, the app will stop the stack from re-registaring on URQ"

    # RRQ/GRQ buttons
    frame $base.rgsource -borderwidth 2 -relief groove
    frame $base.rglist -borderwidth 1
    frame $base.rgGK -borderwidth 2 -relief groove
    label $base.rgGK.labAlias -borderwidth 1 -text "Gatekeeper Alias" -width 15
    entry $base.rgGK.entAlias -textvar tmp(rgrq,gkAlias) -width 1

    alias:createEntry $base $base.rgsource $base.rglist "Source Type" "Source"
    ip:createEntry rgrq $base.rgGK $base.ras.rrq "rgrq,address"
    grid rowconf $base.rgGK 0 -minsize 7

    set tmp(rasWidget) $base

    ########
    # GRID #
    ########

    grid rowconf $base 0 -weight 0 -minsize 7
    grid rowconf $base 1 -weight 0
    grid rowconf $base 2 -weight 0 -minsize 7
    grid rowconf $base 3 -weight 1
    grid rowconf $base 4 -weight 0 -minsize 7
    grid rowconf $base 5 -weight 0
    grid columnconf $base 0 -weight 1

    # Manual: RAS buttons
    grid $base.ras -in $base -column 1 -rowspan 5 -row 1 -sticky nesw -padx 2
    grid rowconf $base.ras 0 -weight 0 -minsize 5
    foreach rowNum { 1 2 3 4 5 6 7 8 9 10 11 12 } rowWeight {1 1 3 1 1 3 1 1 3 1 0 0} {
        grid rowconf $base.ras $rowNum -weight $rowWeight
    }
    grid $base.ras.rrq    -in $base.ras -column 0 -row 1 -padx 4
    grid $base.ras.urq    -in $base.ras -column 0 -row 2 -padx 4
    grid $base.ras.clear  -in $base.ras -column 0 -row 4 -padx 4
    grid $base.ras.reset  -in $base.ras -column 0 -row 5 -padx 4
    grid $base.ras.bw     -in $base.ras -column 0 -row 7 -padx 4 -sticky s
    grid $base.ras.bwValues -in $base.ras -column 0 -row 8 -padx 4 -sticky nwe -ipady 1
    grid $base.ras.nsm    -in $base.ras -column 0 -row 10 -padx 4
    grid $base.ras.rai    -in $base.ras -column 0 -row 11 -padx 4 -sticky w
    grid $base.ras.nrr    -in $base.ras -column 0 -row 12 -padx 4 -sticky w

    # RRQ/GRQ buttons
    grid $base.rgsource   -in $base -column 0 -row 1 -sticky nsew -ipady 1 -padx 2
    grid $base.rglist     -in $base -column 0 -row 3 -sticky nesw -padx 2
    grid $base.rgGK       -in $base -column 0 -row 5 -sticky nsew -ipady 2 -padx 2
    grid $base.rgGK.labAlias -in $base.rgGK -column 0 -row 3 -sticky w
    grid $base.rgGK.entAlias -in $base.rgGK -column 1 -row 3 -ipady 2 -pady 2 -sticky ew -columnspan 3

    ########
    # BIND #
    ########
    bind .test <Control-Key-r> {$base.ras.rrq invoke}
    bind .test <Control-Key-u> {$base.ras.urq invoke}
    bind $base <Control-Key-c> {$base.clearList invoke}
    bind $base <Double-Return> {$base.ras.rrq invoke}

    placeHeader $base.ras "RAS"
    placeHeader $base.rgGK "Gatekeeper Address"
    placeHeader $base.rgsource "Source Address"
}

proc test:miscTab {base tabButton} {
    # Call buttons
    frame $base.call -borderwidth 2 -relief groove
    button $base.call.status -tooltip "Send Status message" \
        -command {Call.SendStatusInquiry [selectedItem .test.calls.list]} \
        -highlightthickness 0  -text "Status" -borderwidth 1 -width 7
    button $base.call.progress -tooltip "Send Progress message" \
        -command {Call.SendProgress [selectedItem .test.calls.list]} \
        -highlightthickness 0 -text "Progress" -borderwidth 1 -width 7
    button $base.call.notify -tooltip "Send Notify message" \
        -command {Call.SendNotify [selectedItem .test.calls.list]} \
        -highlightthickness 0 -text "Notify" -borderwidth 1 -width 7
    button $base.call.userInf -tooltip "Send UserInformation message" \
        -command {Call.SendUserInformation [selectedItem .test.calls.list] $app(options,display) } \
        -highlightthickness 0 -text "User Info" -borderwidth 1 -width 7
    button $base.call.facility -tooltip "Send Facility message" \
        -command {Window open .facility [selectedItem .test.calls.list]} \
        -highlightthickness 0 -text "Facility" -borderwidth 1 -width 7

    # Non Standard Data
    frame $base.nsdFrame -borderwidth 2 -relief groove
    checkbutton $base.nsdFrame.use -tooltip "Put NonStandardData structure in all messages where possible" \
        -padx 0 -pady 0 -text "Use non standard data" -variable app(options,nonStandardData)
    set nsd [nsd:create $base.nsdFrame]
    $nsd configure -borderwidth 0

    frame $base.sec -borderwidth 2 -relief groove
    label $base.sec.modeLab -borderwidth 0 -text "Outgoing mode:"
    menubutton $base.sec.mode -borderwidth 1 -height 1 -indicatoron 1 \
        -menu $base.sec.mode.01 -padx 2 -pady 2 -relief raised \
        -text None -textvariable app(options,secMode) -width 0 -tooltip "Select security mode"
    menu $base.sec.mode.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach secMode { None Procedure1 } {
        $base.sec.mode.01 add radiobutton \
            -variable app(options,secMode) -label $secMode -value $secMode -indicatoron 0
    }
    checkbutton $base.sec.checkIn -text "Check Incoming" -variable app(options,checkIncoming)
    label $base.sec.senderLab   -borderwidth 0 -text "Sender ID"
    label $base.sec.generalLab  -borderwidth 0 -text "General ID"
    label $base.sec.passwordLab -borderwidth 0 -text "Password"
    entry $base.sec.senderEnt -textvariable app(options,senderID) -width 12 \
            -validate key -invcmd bell -vcmd { expr { [string length %P] < 64 } }
    entry $base.sec.generalEnt -textvariable app(options,generalID) -width 12 \
            -validate key -invcmd bell -vcmd { expr { [string length %P] < 64 } }
    entry $base.sec.passwordEnt -textvariable app(options,password) -width 12 \
            -validate key -invcmd bell -vcmd { expr { [string length %P] < 64 } }
    button $base.sec.setParam -tooltip "Set the sender, general and password parameters" \
        -command { SEC.notifyModeChange } \
        -highlightthickness 1 -text "Set" -borderwidth 2 -width 6

    ########
    # GRID #
    ########

    grid rowconf $base 0 -weight 0 -minsize 7
    grid rowconf $base 1 -weight 1
    grid rowconf $base 2 -weight 0 -minsize 7
    grid rowconf $base 3 -weight 1
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1

    # Manual: Call buttons
    grid $base.call -in $base -column 0 -columnspan 2 -row 1 -padx 2 -sticky nesw
    grid columnconf $base.call 1 -weight 1
    grid columnconf $base.call 2 -weight 1
    grid columnconf $base.call 3 -weight 1
    grid columnconf $base.call 4 -weight 1
    grid columnconf $base.call 5 -weight 1
    grid columnconf $base.call 6 -weight 1
    grid rowconf $base.call 0 -minsize 6
    grid rowconf $base.call 1 -weight 1
    grid $base.call.status   -in $base.call -column 1 -row 1 -padx 1 -pady 1
    grid $base.call.progress -in $base.call -column 2 -row 1 -padx 1 -pady 1
    grid $base.call.notify   -in $base.call -column 3 -row 1 -padx 1 -pady 1
    grid $base.call.userInf  -in $base.call -column 4 -row 1 -padx 1 -pady 1
    grid $base.call.facility -in $base.call -column 5 -row 1 -padx 1 -pady 1

    # Non Standard Data
    grid $base.nsdFrame -in $base -column 0 -row 3 -padx 2 -sticky snew
    grid columnconf $base.nsdFrame 0 -weight 1
    grid rowconf $base.nsdFrame 0 -minsize 5
    grid rowconf $base.nsdFrame 3 -minsize 3
    grid $base.nsdFrame.use -in $base.nsdFrame -column 0 -row 1 -sticky w
    grid $nsd -in $base.nsdFrame -column 0 -row 2 -sticky nesw

    grid $base.sec -in $base -column 1 -row 3 -padx 2 -sticky snew
    grid columnconf $base.sec 1 -weight 1
    grid rowconf $base.sec 0 -minsize 5
    foreach i {1 2 3 4 5 6} {grid rowconf $base.sec $i -weight 1}
    grid rowconf $base.sec 7 -weight 3

    grid $base.sec.modeLab -in $base.sec -column 0 -columnspan 2 -row 1 -padx 3 -sticky w
    grid $base.sec.mode -in $base.sec -column 0 -columnspan 2 -row 2 -padx 3 -sticky ew
    grid $base.sec.checkIn -in $base.sec -column 0 -columnspan 2 -row 3 -padx 3 -sticky w
    grid $base.sec.senderLab -in $base.sec -column 0 -row 4 -padx 2 -sticky w
    grid $base.sec.generalLab -in $base.sec -column 0 -row 5 -padx 2 -sticky w
    grid $base.sec.passwordLab -in $base.sec -column 0 -row 6 -padx 2 -sticky w
    grid $base.sec.senderEnt -in $base.sec -column 1 -row 4 -padx 2 -sticky ew
    grid $base.sec.generalEnt -in $base.sec -column 1 -row 5 -padx 2 -sticky ew
    grid $base.sec.passwordEnt -in $base.sec -column 1 -row 6 -padx 2 -sticky ew
    grid $base.sec.setParam -in $base.sec -column 0 -columnspan 2 -row 7 -padx 2

    ########
    # BIND #
    ########
    placeHeader $base.call "Call"
    placeHeader $base.nsdFrame "Non-Standard Data"
    placeHeader $base.sec "Security"
}

proc test:optionTab {base tabButton} {
    global app tmp

    frame $base.internal -borderwidth 2 -relief groove
    checkbutton $base.internal.suppress -text "Message suppression" -variable app(options,suppressMessages) \
        -highlightthickness 0 -tooltip "Suppress messages from application's log windows"
    checkbutton $base.internal.wrapperLogs -text "Wrapper logging" -variable app(options,scriptLogs) \
        -highlightthickness 0 -tooltip "Add log messages when scripts access any wrapper functions"
    checkbutton $base.internal.popUp -text "Enable pop-ups" -variable app(options,popUp) \
        -highlightthickness 0 -tooltip "Allow sub window pop-up for info and immediate controls"
    button $base.internal.raslab -borderwidth 0 -text "RAS Address" -tooltip "Click here to copy to clipboard" \
        -command {global tmp; clipboard clear; clipboard append $tmp(options,ras)}
    label $base.internal.ras -textvariable tmp(options,ras) -width 1 -relief sunken -borderwidth 1
    button $base.internal.q931lab -borderwidth 0 -text "Q.931 Address" -tooltip "Click here to copy to clipboard" \
        -command {global tmp; clipboard clear; clipboard append $tmp(options,q931)}
    label $base.internal.q931 -textvariable tmp(options,q931) -width 1 -relief sunken -borderwidth 1
    button $base.internal.annexElab -borderwidth 0 -text "Annex E Address" -tooltip "Click here to copy to clipboard" \
        -command {global tmp; clipboard clear; clipboard append "ETA:"; clipboard append $tmp(options,annexE)}
    label $base.internal.annexE -textvariable tmp(options,annexE) -width 1 -relief sunken -borderwidth 1

    frame $base.external -borderwidth 2 -relief groove
    foreach type {log config script} \
            txt { "Log viewer" "Configuration editor" "Script editor" } \
            i {1 2 3} {
        entry $base.external.${type}Ed -textvariable app(editor,$type) -width 0
        button $base.external.${type}But -text "..." -pady 0 -padx 0 -borderwidth 1 \
            -command "options:chooseEditor $type"
        label $base.external.${type}Lab -borderwidth 0 -text $txt
        grid $base.external.${type}Lab -in $base.external -column 0 -row $i -sticky w
        grid $base.external.${type}Ed -in $base.external -column 1 -row $i -sticky ew
        grid $base.external.${type}But -in $base.external -column 2 -row $i -pady 1
    }

    ########
    # GRID #
    ########
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -minsize 7
    grid rowconf $base 1 -weight 1
    grid rowconf $base 2 -minsize 7
    grid rowconf $base 3 -weight 1

    grid $base.internal -in $base -column 0 -row 1 -padx 2 -sticky nsew
    grid columnconf $base.internal 2 -weight 1
    grid rowconf $base.internal 0 -minsize 7
    grid $base.internal.suppress -in $base.internal    -column 0 -row 0 -padx 6 -pady 1 -sticky w
    grid $base.internal.wrapperLogs -in $base.internal -column 0 -row 1 -padx 6 -pady 1 -sticky w
    grid $base.internal.popUp -in $base.internal       -column 0 -row 2 -padx 6 -pady 1 -sticky w
    grid $base.internal.raslab    -in $base.internal -column 1 -row 0 -sticky w -padx 3
    grid $base.internal.ras       -in $base.internal -column 2 -row 0 -padx 3 -pady 1 -sticky ew
    grid $base.internal.q931lab   -in $base.internal -column 1 -row 1 -sticky w -padx 3
    grid $base.internal.q931      -in $base.internal -column 2 -row 1 -padx 3 -pady 1 -sticky ew
    grid $base.internal.annexElab -in $base.internal -column 1 -row 2 -sticky w -padx 3
    grid $base.internal.annexE    -in $base.internal -column 2 -row 2 -padx 3 -pady 1 -sticky ew

    grid $base.external -in $base -column 0 -row 3 -padx 2 -sticky nsew
    grid columnconf $base.external 2 -weight 1
    grid rowconf $base.external 0 -minsize 7

    ########
    # BIND #
    ########
    placeHeader $base.internal "Internal"
    placeHeader $base.external "External"
}


# test:createMenu
# Creates the menu for the main window
# input  : none
# output : none
# return : none
proc test:createMenu {} {
    global tmp app

    # Make sure we've got all the menus here
    set m(tools) {
        {Config             0 {}          config:open}
        {Status             0 Ctrl+S      "Window open .status"}
        { "Log File"        4 {}          logfile:open}
        {separator}
        { "Start stack"     3 {}          "api:cm:Start"}
        { "Stop stack"      3 {}          "api:cm:Stop"}
        {Restart            0 {}          "test.Restart"}
        {Exit               0 {}          "test.Quit"}
    }
    set m(advanced) {
        { "Console input"   0 {}          "Window open .console"}
        {separator}
        { "Execute script"  0 {}          script:load}
        { "Stop script"     0 {}          script:stop}
        { "Describe script" 0 {}          script:describe}
        { "Edit script"     0 {}          script:edit}
    }
    set m(help) {
        { "About"           0 {}          "Window open .about"}
    }

    # Create the main menu and all of the sub menus from the array variable m
    menu .test.main -tearoff 0
    foreach submenu {tools advanced help} {
        .test.main add cascade -label [string toupper $submenu 0 0] \
            -menu .test.main.$submenu -underline 0
        menu .test.main.$submenu -tearoff 0

        foreach item $m($submenu) {
            set txt [lindex $item 0]

            if {$txt == "separator"} {
                # Put a separator
                .test.main.$submenu add separator
            } else {
                # Put a menu item
                set under [lindex $item 1]
                set key [lindex $item 2]
                set cmd [lindex $item 3]
                .test.main.$submenu add command -label $txt -accel $key -command $cmd -underline $under
            }
        }
    }

    test:updateScriptMenu
}


# test:updateGui
# Update the GUI display after the stack was initialized
# input  : none
# output : none
# return : none
proc test:updateGui {} {
    global tmp app

    # Are we working with automatic RAS ?
    if {[rgrq:IsAutoRas] == 0} {
        test:SetGatekeeperStatus "No GK"

        foreach widget {rrq urq rai nsm bw} {
            $tmp(rasTab).ras.$widget configure -state disabled
        }
    } else {
        foreach widget {rrq urq rai nsm bw} {
            $tmp(rasTab).ras.$widget configure -state normal
        }
    }

    # Are we supporting RTP ?
    if {[test.RtpSupported] == 0} {
#    grid remove $tmp(callTab).rtp
        grid $tmp(callTab).callInfo -in $tmp(callTab)
        $tmp(chanTab).chanBut.chekBut.replay configure -state disabled
    } else {
#        grid remove $tmp(callTab).callInfo
#        grid $tmp(callTab).rtp -in $tmp(callTab)
        $tmp(chanTab).chanBut.chekBut.replay configure -state normal
    }

    # Are we supporting H450 ?
    if {[test.H450Supported] == 0} {
        $tmp(h450Button) configure -state disabled
        if { $app(test,currTab) == 7 } {
            notebook:changeSelection test 1
        }
    } else {
        $tmp(h450Button) configure -state normal
    }

    # Refresh the list of data types used for the channels
    set dataTypes [api:app:GetDataTypes]
    set tmp(newchan,dataType) [lindex $dataTypes 0]
    $tmp(chanTab).chanBut.chanprm.dataTypemen.01 delete 0 end
    foreach dataType $dataTypes {
        $tmp(chanTab).chanBut.chanprm.dataTypemen.01 add radiobutton \
            -indicatoron 0 -value $dataType -variable tmp(newchan,dataType) -label $dataType
    }

    # Make sure we display the registration information and the status of the stack as Running
    rgrq:GetRegInfo
    test:SetStackStatus "Running"

    # Notify application about the used ports
    set ras ""; set rPort 0
    set annexE ""; set ePort 0
    set q931 [api:cm:GetLocalCallSignalAddress]
    catch {
        set ras [api:cm:GetLocalRASAddress]
        set rPort "[lindex [string map {":" " "} $ras] 1]"
    }
    catch {
        set annexE [api:cm:GetLocalAnnexEAddress]
        set ePort "[lindex [string map {":" " "} $annexE] 1]"
    }
    if {$rPort != 0} {
        test:Log "RAS port is $rPort"
        set tmp(options,ras) $ras
    } else {
        test:Log "No RAS support"
        set tmp(options,ras) "- none -"
    }
    test:Log "Q.931 port is [lindex [string map {":" " "} $q931] 1]"
    if {$ePort != 0} {
        test:Log "Annex E port is $ePort"
        set tmp(options,annexE) $annexE
    } else {
        test:Log "No Annex E support"
        set tmp(options,annexE) "- none -"
    }
    set tmp(quick,dest) $q931
    set tmp(newcall,address) $q931
    set tmp(options,q931) $q931

    .test.calls.list delete 0 end
    .test.calls.conf delete 0 end
    .test.calls.conn delete 0 end
    .test.chan.inList delete 0 end
    .test.chan.outList delete 0 end
}


# test:updateScriptMenu
# Update the menu holding dynamically executed scripts
# input  : none
# output : none
# return : none
proc test:updateScriptMenu {} {
    global tmp app

    if {[.test.main.advanced index end] > 5} {
        .test.main.advanced delete 6 end
    }

    if {[array names app "script,recent"] != ""} {
        set i 1

        # Add dynamic script list menu items
        .test.main.advanced add separator
        foreach item $app(script,recent) {
            .test.main.advanced add command -label "$i $item" -underline 0 \
                -command "script:run {$item}"
            incr i
        }
    }
}


# test:refreshMenu
# This procedure makes sure menu items are enabled/disabled when specific variables status change
# input  : name1    - Array name (app)
#          name2    - Element name in array
#          op       - Operation on variable
# output : none
# return : none
proc test:refreshMenu {name1 name2 op} {
    global tmp app

    # Determine the menu item's state
    if {$tmp($name2) == 0} {
        set state "disabled"
    } else {
        set state "normal"
    }

    # Make sure to update the right menu item
    switch $name2 {
        script,running {
            foreach i {3 4 5} {
                .test.main.advanced entryconfigure $i -state $state
            }
            if {$tmp($name2) == 0} {
                .test.status.mode configure -text "Normal"
            } else {
                .test.status.mode configure -text "Script"
            }
        }
    }
}


# test:updateTimer
# Update the application's timer
# input  : none
# output : none
# return : none
proc test:updateTimer {} {
    global tmp app

    set diff [expr {[clock seconds] - $tmp(testapp,start)} ]

    set d [expr $diff / 86400]
    set h [expr ($diff / 3600) % 24]
    set m [expr ($diff / 60) % 60]

    .test.status.timer configure -text [format "%d - %02d:%02d" $d $h $m]

    # Make sure we redisplay the time after a minute
    after 60000 test:updateTimer
}


# test:Log
# This procedure logs a message to the main window
# input  : message      - The message to log
# output : none
# return : none
proc test:Log {message} {
    global tmp app

    # See if we want to display the message at all
    if {$app(options,suppressMessages) == 1} return

    set timer [.test.status.timer cget -text]

    .test.msg.list insert end "$message"
    .test.msg.list see end

    set lines [expr {[.test.msg.list size] - $tmp(maxLines)}]
    if {$lines > 0} {
        .test.msg.list delete 0 $lines
    }
}


# test:SetCalls
# This procedure sets the status bar call information
# input  : cur      - Current number of calls
#          total    - Total number of calls connected so far
# output : none
# return : none
proc test:SetCalls {cur total} {
    .test.status.calls configure -text "Calls: $cur, $total"
}


# test:SetGatekeeperStatus
# This procedure sets the status bar gatekeeper status information
# input  : status   - String representing the current registration status
# output : none
# return : none
proc test:SetGatekeeperStatus {status} {
    .test.status.gk configure -text $status
}


# test:SetH245Status
# This procedure sets the status bar h245 status information
# input  : status   - String representing the current registration status
# output : none
# return : none
proc test:SetH245Status {status} {
    .test.status.h245 configure -text $status
}


# test:SetStackStatus
# This procedure sets the status bar stack status information
# input  : status   - String representing the current stack status
# output : none
# return : none
proc test:SetStackStatus {status} {
    .test.status.stack configure -text $status
}


# test:toggleManualButtons
# Toggle the manual mode buttons on the main window on and off.
# This procedure is called by the manual mode checkbox on the main window.
# input  : manualButtonsOn  - "1" indicates that the manual buttons should be displayed on the main
#                             window.
# output : none
# return : none
proc test:toggleManualButtons {manualButtonsOn} {

    if {$manualButtonsOn == 1} {
        # Set manual mode for calls on
        grid .test.manual -in .test -column 3 -row 2 -sticky nesw
        #after 50 {
        #    place .test.manualHeader -x [expr { [winfo x .test.manual] + 9 } ] \
        #        -y [expr { [winfo y .test.manual] - 1 } ] -anchor nw -bordermode ignore }
    } else {
        # Remove manual mode
        grid remove .test.manual
        #place forget .test.manualHeader
    }
}


# test:activateTool
# Activate a tool on the tools bar
# input  : toolname - Name of the tool to activate
# output : none
# return : none
proc test:activateTool {toolname} {

    .test.tools.$toolname configure -relief sunken
}


# test:deactivateTool
# Deactivate a tool on the tools bar
# input  : toolname - Name of the tool to dactivate
# output : none
# return : none
proc test:deactivateTool {toolname} {

    .test.tools.$toolname configure -relief flat
}

bind toolbar <Enter> {
    if [string compare [%W cget -relief] "sunken"] {
        if [string compare [%W cget -state] "disabled"] {
            %W configure -relief raised
        }
    }
}

bind toolbar <Leave> {
    if [string compare [%W cget -relief] "sunken"] {%W configure -relief flat}
}


