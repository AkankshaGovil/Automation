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
#                                 TAP_wrapper.tcl
#
#   Advanced scripting wrapper.
#   This file holds all the procedures that allows loading and canceling of advanced scripts
#   that use the CM wrapper commands.
#
#
#       Written by                        Version & Date                        Change
#      ------------                       ---------------                      --------
#  Oren Libis & Tsahi Levent-Levi           28-Jul-2000
#
##############################################################################################



##############################################################################################
#
#   APPLICATION API operations
#
##############################################################################################


# api:app:SetScriptMode
# Set the application's script mode. This mode determines if new channels and calls are
# handled by the test application's GUI or scripts.
# input  : on   - When set to 1, calls and channels are handled by the script.
#                 When set to 0, they are handled by the GUI
# output : none
# return : none
proc api:app:SetScriptMode {on} {
    global tmp

    set tmp(options,scriptMode) $on
}


# api:app:StopScript
# Stop the script from running and get back to regular execution through GUI. This will
# also remove the script from memory.
# input  : none
# output : none
# return : none
proc api:app:StopScript {} {
    script:stop
}


# api:app:DescribeScript
# Display the script's description
# input  : none
# output : none
# return : none
proc api:app:DescribeScript {} {
    script:describe
}


# api:app:Log
# Log a message to the log window of the test application
# input  : str  - String to log
# output : none
# return : none
proc api:app:Log {str} {
    test:Log $str
}


# api:app:GetSelectedCall
# Return the handle of the selected call in the main window
# input  : none
# output : none
# return : The handle of the selected call
proc api:app:GetSelectedCall {} {
    set str [selectedItem .test.calls.list]
    if {$str == ""} {
        return "0x0"
    } else {
        return [string range [lindex $str 1] 0 end-1]
    }
}







##############################################################################################
#
#   SCRIPT operations
#
##############################################################################################


# script:load
# This procedure opens dialog box for opening script file, and invokes the selected script.
# It makes sure we can stop the script in the future and unload it.
# input  : none
# output : none
# return : none
proc script:load {} {
    global tmp app

    # Choose a script file to execute
    set scriptFile [tk_getOpenFile -filetypes {{ "TCL Scripts" {script*.tcl} }} -title "Open Script"]

    script:run $scriptFile
}


# script:run
# This procedure executes a script file
# input  : filename - The filename of the script to execute
# output : none
# return : none
proc script:run {filename} {
    global tmp app

    if {$filename == ""} return
    if {![file exists $filename]} {
        msgbox "Error" picExclamation "Couldn't find script: $filename" {{Ok -1 <Key-Return>}}
        return
    }

    if {$tmp(script,running) == 1} { script:stop }

    # Make sure we're in script mode
    set tmp(options,scriptMode) 1

    # Make sure we check procedures before declaring them
    rename proc script:proc

    # This function is used automatically while reading the given script. It is called automatically
    # on each declared procedure and it checks to see that the procedures don't run over any procedure
    # names used by the test application itself.
    script:proc proc {name args body} {
        global tmp app

        # Check that the declared procedure doesn't have a known name
        if {[info procs $name] != ""} {
            error "Cannot use the given procedure name $name"
        }

        # Declare the procedure - it's a good one
        script:proc $name $args $body
    }

    # Make sure we remember the variables and procedures we're currently dealing with
    set intVars [uplevel #0 info vars]
    set intProcs [uplevel #0 info procs]

    # Load the script
    if [catch {uplevel #0 [list source $filename]} err] {
        tk_dialog .error "Error" "Encountered an error while reading the script: $err" "" 0 "Ok"
    }

    # Generate a list of the global variables used by the script
    set tmp(script,vars) ""
    foreach varname [uplevel #0 info vars] {
        if {[lsearch -exact $intVars $varname] == -1} {lappend tmp(script,vars) $varname}
    }

    # Generate a list of the procedures used by the script
    set tmp(script,procs) ""
    foreach procname [uplevel #0 info procs] {
        if {[lsearch -exact $intProcs $procname] == -1} {lappend tmp(script,procs) $procname}
    }

    # Back to normal procedures...
    rename proc ""
    rename script:proc proc

    set tmp(script,running) 1
    set tmp(script,filename) $filename

    # Update recent scripts list
    if {[array names app script,recent] == ""} {
        set app(script,recent) [list $filename]
    } else {
        set index [lsearch -exact $app(script,recent) $filename]
        if {$index != -1} {
            # Remove earlier occurances - we put it as first...
            set app(script,recent) [lreplace $app(script,recent) $index $index]
        }

        # Add this one to recent scripts list
        set app(script,recent) [linsert $app(script,recent) 0 $filename]

        # Make sure list is not too long
        if {[llength $app(script,recent)] > 5} {
            set app(script,recent) [lreplace $app(script,recent) 5 end]
        }
    }

    # Update menu
    test:updateScriptMenu

    test:Log "SCRIPT: $filename"

    # Make sure we've got a description of the script
    global Script
    if {(![info exists Script]) || ([array names Script Description] == "")} {
        msgbox "Warning" picExclamation "Script doesn't contain any description - Script(Description)" { { "Ok" -1 <Key-Return> } }
    } else {
        foreach line $Script(Description) {
            test:Log "$line"
        }
    }
}


# script:stop
# This procedure stops the execution of a script
# input  : none
# output : none
# return : none
proc script:stop {} {
    global tmp app

    if {$tmp(script,running) == 0} return

    # Remove all of the script's global variables
    foreach varname $tmp(script,vars) {
        catch {
            global $varname
            unset $varname
        }
    }
    set tmp(script,vars) ""

    # Remove all of the script's procedures
    foreach procname $tmp(script,procs) {
        catch {
            rename $procname ""
        }
    }
    set tmp(script,procs) ""

    # Always disable script support when script is stopped
    set tmp(options,scriptMode) 0
    set tmp(script,running) 0

    # Change the script flag of each call to be NORMAL.
    set callLines [.test.calls.list get 0 end]
    foreach line $callLines {
        set callHandle [string range $line 0 [expr [string first ":" $line] - 1]]
        set call [string range $callHandle [expr [string first "x" $callHandle]-1] [string length $callHandle]]
        api:app:SetCallMode $call "Normal"
    }
}


# script:edit
# This procedure edits a script file
# input  : none
# output : none
# return : none
proc script:edit {} {
    global tmp app

    if {$tmp(script,running) == 0} {
        # Choose a file name
        set scriptFile [tk_getOpenFile -filetypes {{ "TCL Scripts" {script*.tcl} }} -title "Edit Script"]

        if {($scriptFile != "") && [file exists $scriptFile]} {
            set tmp(script,filename) $scriptFile
        } else {
            return
        }
    }

    exec $app(editor,script) $tmp(script,filename) & > nul
}


# script:describe
# This procedure describes a running script file
# input  : none
# output : none
# return : none
proc script:describe {} {
    global tmp app
    global Script

    if {(![info exists Script]) || ([array names Script Description] == "")} return

    foreach line $Script(Description) {
        append desc "${line}\n"
    }

    msgbox "Script Info" picInformation [string range $desc 0 end-1] { { "Ok" -1 <Key-Return> } }
}






##############################################################################################
#
#   CONSOLE operations
#
##############################################################################################

# console:create
# Creation procedure of the Console window
# This window is opened when "Advanced|Console input" is selected from the main menu
# input  : none
# output : none
# return : none
proc console:create {} {
    global tmp app

    if {[winfo exists .console]} {
        wm deiconify .console
        return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .console -class Toplevel
    wm focusmodel .console passive
    wm geometry .console 500x400+300+100
    wm minsize .console 150 50
    wm overrideredirect .console 0
    wm resizable .console 1 1
    wm deiconify .console
    wm title .console "Console input"
    wm transient .console .dummy

    text .console.output -state disabled -yscrollcommand {.console.scrl set} -wrap word
    scrollbar .console.scrl -command {.console.output yview} -highlightthickness 0
    label .console.label -text "command:" -relief groove -borderwidth 2
    entry .console.input

    .console.output tag configure col:black -foreground black
    .console.output tag configure col:blue -foreground blue
    .console.output tag configure col:red -foreground red

    ###################
    # SETTING GEOMETRY
    ###################
    grid rowconf .console 0 -weight 1
    grid columnconf .console 1 -weight 1
    grid .console.output -in .console -row 0 -column 0 -columnspan 2 -sticky nesw
    grid .console.scrl -in .console -row 0 -column 2 -sticky ns
    grid .console.label -in .console -row 1 -column 0 -padx 1 -pady 2 -sticky nesw
    grid .console.input -in .console -row 1 -column 1 -columnspan 2 -padx 1 -pady 2 -sticky ew

    ###########
    # BINDINGS
    ###########
    bind .console.input <Key-Return> "console:execute"

    #########
    # OTHERS
    #########
    focus .console.input
}


# console:write
# Write a string into the output window of the console
# input  : line     - Line to write
#          color    - Color to use
# output : none
# return : none
proc console:write {line color} {
    .console.output configure -state normal
    .console.output insert end $line col:$color
    .console.output configure -state disabled
    .console.output yview end
}


# console:execute
# Execute a command inside the console
# input  : none
# output : none
# return : none
proc console:execute {} {

    set cmd [.console.input get]

    # Insert the new command into the textbox
    console:write "% $cmd\n" black

    # Execute the command
    if { [catch {set result [uplevel #0 eval "$cmd"]} errMsg] } {
        console:write "$errMsg\n" red
    } else {
        console:write "$result\n" blue
    }
    .console.input delete 0 end
}



