##############################################################################################
#
#   TAP_gui.tcl : Prepared gui widgets and gui services
#
##############################################################################################

# Window
# This procedure is responsible for managing specific windows
# input  : cmd  - Command to perform on window:
#                 open      - Create and open the window. This will cause the window to be displayed
#                             on the screen and take the focus
#                 close     - Hide the window from view. This command will not destroy the window,
#                             only remove it from view
#                 toggle    - Change window status between closed and open
#                 delete    - Remove and destroy a window.
#          win  - The window we're dealing with
#          args - Arguments to use for the window's create function
# output : none
# return : none
proc Window {cmd win args} {
    global tmp app

    set procName [string range $win 1 end]
    set exists [winfo exists $win]
    if {$exists == "1"} {
        set mapped [winfo ismapped $win]
    } else {
        set mapped 0
    }

    if {$cmd == "toggle"} {
        if {$mapped == "1"} {
            set cmd "close"
        } else {
            set cmd "open"
        }
    }

    switch $cmd {
        open {
            eval "$procName:create $args"
            if {[winfo exists $win]} {
                focus $win
                catch {setIcon [winfo id $win]}
            }
        }

        close {
            # Remove binding for window when it is closed
            if {$exists == "1"} { bind $win <Expose> }

            # Remove window if it is displayed
            if {$mapped == "1"} {wm withdraw $win}

            # Call window's clearing function
            if {$exists == "1" && [info procs "$procName:clear"] != ""} {
                eval "$procName:clear"
            }

            # Remove this window from call's window list if it is a call-related window
            catch {
                set i [lsearch -glob $tmp($tmp($win)) $win]
                if {$i != -1} then {lreplace $tmp($tmp($win)) $i $i}
            }
        }

        delete {

            # Delete the window
            if {$exists == "1"} {

                # See if the window is one of the windows we're interested in their position and size
                if {[lsearch -exact $tmp(tops) $procName] != -1} {
                    set app($procName,size) [winfo geometry $win]
                }

                # Remove binding for window when it is deleted
                bind $win <Expose>

                destroy $win
            }
            # Remove this window from call's window list if it is a call-related window
            catch {
                set i [lsearch -glob $tmp($tmp($win)) $win]
                if {$i != -1} then {lreplace $tmp($tmp($win)) $i $i}
            }
        }
    }
}


# gui:SetDefaults
# This procedure makes sure we have somewhat the same appearance of widgets in
# Windows and Unix systems, allowing smaller code when actually creating the widgets.
# input  : none
# output : none
# return : none
proc gui:SetDefaults {} {
    set bgColor [option get . background {}]
    if {$bgColor != ""} {
        option add *highlightBackground $bgColor
        option add *activeBackground $bgColor
    }
    option add *exportSelection false
}


# gui:ToolTipWidgets
# This procedure creates new widget creation functions instead of the regular ones.
# The new functions have an additional tooltip option in them
# input  : none
# output : none
# return : none
proc gui:ToolTipWidgets {} {

    # Rename all of the original procedure names and delcare new procedure names
    foreach {p c} {
        button Button
        label Label
        frame Frame
        scrollbar Scrollbar
        text Text
        entry Entry
        listbox Listbox
        checkbutton Checkbutton
        menubutton Menubutton
        radiobutton Radiobutton
        menu Menu
    } {
        rename $p orig$c
        set body "gui:ballooned $c "
        append body {$w $args}
        proc $p {w args} "eval $body"
    }

    # Actual procedure that does the work:
    proc gui:ballooned {class w args} {
        set newArgs {}
        set tip ""

        # Look for the tooltip option and strip it from the arguments
        set max [llength $args]
        set i 0
        while {$i < $max} {
            set item [lindex $args $i]
            set value [lindex $args [expr $i+1]]
            if {$item == "-tooltip"} {
                set tip $value
            } else {
                lappend newArgs $item $value
            }
            incr i 2
        }

        # Call the original procedure
        set res [eval "orig$class $w $newArgs"]

        # Add the tooltip
        if {$tip != ""} {
            balloon:set $w $tip
        }

        return $res
    }
}



##############################################################################################
#
#   Non Standard Data
#
##############################################################################################

# nsd:create
# Create the frame of the non standard data parameters
# input  : path             - path to the place that we want to insert the frame
# output : none
# return : the frame that was created
proc nsd:create {path {var "app(nsd"} } {
    global tmp app

    set finalPath "$path.nsd"
    frame $finalPath -borderwidth 2 -relief groove
    # the radiobuttons are for choosing which kind of non standard data
    radiobutton $finalPath.h221 -text "H.221" -value "H221" -variable "$var,choiceVar)"\
                -command "
                            $finalPath.manufacturerCodeEnt configure -state normal
                            $finalPath.countryCodeEnt configure -state normal
                            $finalPath.extensionEnt configure -state normal
                            $finalPath.objectEnt configure -state disable
                         "

    radiobutton $finalPath.object -text "Object" -value "Object" -variable "$var,choiceVar)"\
                -command "
                            $finalPath.manufacturerCodeEnt configure -state disable
                            $finalPath.countryCodeEnt configure -state disable
                            $finalPath.extensionEnt configure -state disable
                            $finalPath.objectEnt configure -state normal
                         "

    label $finalPath.manufacturerCodeLab -text "Manufacturer Code"
    entry $finalPath.manufacturerCodeEnt -textvar "$var,manVar)" -width 5
    label $finalPath.countryCodeLab -text "Country Code"
    entry $finalPath.countryCodeEnt -textvar "$var,counVar)" -width 5
    label $finalPath.extensionLab -text "Extension"
    entry $finalPath.extensionEnt -textvar "$var,extenVar)" -width 5
    label $finalPath.dataLab -text "Data"
    entry $finalPath.dataEnt -textvar "$var,dataVar)"
    label $finalPath.objectLab -text "Object"
    entry $finalPath.objectEnt -textvar "$var,objectVar)"

    grid rowconf $finalPath 0 -minsize 5
    grid rowconf $finalPath 8 -weight 1
    grid columnconf $finalPath 0 -minsize 4
    grid columnconf $finalPath 1 -minsize 8
    grid columnconf $finalPath 5 -weight 1 -minsize 4
    grid $finalPath.h221 -in $finalPath -column 1 -row 1 -columnspan 4 -padx 2 -sticky w
    grid $finalPath.object -in $finalPath -column 1 -row 5 -columnspan 4 -padx 2 -sticky w
    grid $finalPath.manufacturerCodeLab -in $finalPath -column 2 -row 2 -columnspan 2 -sticky w
    grid $finalPath.manufacturerCodeEnt -in $finalPath -column 4 -row 2 -sticky w -pady 1
    grid $finalPath.countryCodeLab -in $finalPath -column 2 -row 3 -columnspan 2 -sticky w
    grid $finalPath.countryCodeEnt -in $finalPath -column 4 -row 3 -sticky w -pady 1
    grid $finalPath.extensionLab -in $finalPath -column 2 -row 4 -columnspan 2 -sticky w
    grid $finalPath.extensionEnt -in $finalPath -column 4 -row 4 -sticky w -pady 1
    grid $finalPath.objectLab -in $finalPath -column 2 -row 6 -columnspan 2 -sticky w
    grid $finalPath.objectEnt -in $finalPath -column 4 -row 6 -sticky ew -pady 1
    grid $finalPath.dataLab -in $finalPath -column 1 -row 7 -columnspan 2 -sticky w
    grid $finalPath.dataEnt -in $finalPath -column 3 -row 7 -columnspan 3 -sticky ew -pady 1 -padx 5

    return $finalPath
 }


 # nsd:getParam
 # get a string from the non standard parameters
 # input  : widget - path of the nsd widget
 # output : none
 # return : the string that was built from the parameters
 proc nsd:getParam { {widget "nsd" } } {
    global tmp app

    if {![string compare $app($widget,choiceVar) "H221"]} {
        set tmp($widget,str) "H221 {$app($widget,counVar) $app($widget,extenVar) $app($widget,manVar)} $app($widget,dataVar)"
    } else {
        set tmp($widget,str) "Obj {$app($widget,objectVar)} $app($widget,dataVar)"
    }

    return $tmp($widget,str)
}

# nsd:getData
 # get the non standard data
 # input  : widget - path of the nsd widget
 # output : none
 # return : the data in a string format
 proc nsd:getData {widget} {
    global tmp app

       return $tmp($widget,dataVar)
}

##############################################################################################
#
#   Alias
#
##############################################################################################

proc alias:add {listWidget entryBase} {

    # Get the type and string of the alias from the GUI
    set aliasType [$entryBase.menBox.type cget -text]
    set aliasString [$entryBase.name get]

    # Make sure parameters are correct
    if {$aliasType == ""} {return}
    if {$aliasString == ""} {return}

    # Create the full alias string
    if [string equal -length 2 $aliasType "PN" ] {
        set alias [format "PN:%s$%s" $aliasString [string range $aliasType 2 end ] ]
    } else {
        set alias "$aliasType:$aliasString"
    }

    # See if we already have such an alias in the list
    set i [lsearch -exact [$listWidget get 0 end] $alias]

    if {$i == -1} {
        # Insert it into the list - we don't have it there yet...
        $listWidget insert end $alias

        # Make sure it's visible inside the listbox
        $listWidget see $i
    }
}

proc alias:del {listWidget} {

    set delAliases [$listWidget curselection]
    set size [llength $delAliases]
    for {set index [incr size -1]} {$index >= 0} {incr index -1} {
        $listWidget delete [lindex $delAliases $index]
    }
}

proc alias:getAll {listWidget} {
    set aliasString ""
    for {set ind 0} {$ind < [$listWidget index end] } {incr ind} {
        set curAlias [$listWidget get $ind]
        append aliasString "$curAlias,"
    }
    return [string trimright $aliasString ","]
}

proc alias:createEntry {bindBase entryBase listBase header aliasLabel} {
    global tmp app

    ###################
    # CREATING WIDGETS
    ###################
    frame $entryBase.menBox -borderwidth 1 -relief sunken
    menubutton $entryBase.menBox.type -borderwidth 1 -indicatoron 1  -width 7\
            -menu $entryBase.menBox.type.01 -padx 0 -pady 0 -relief raised -text TEL -textvariable tmp($entryBase,aliasType)
    menu $entryBase.menBox.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach value {TEL NAME URL TNAME EMAIL} {
        $entryBase.menBox.type.01 add radiobutton -indicatoron 0 -label $value -value $value \
            -variable tmp($entryBase,aliasType) -command "$entryBase.name delete 0 end"
    }
    menu $entryBase.menBox.type.01.02 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach name { "PublicUnknown" "PublicInternationalNumber" "PublicNationalNumber" "PublicNetworkSpecificNumber" \
        "PublicSubscriberNumber" "PublicAbbreviatedNumber" "DataPartyNumber" "TelexPartyNumber" "PrivateUnknown" \
        "PrivateLevel2RegionalNumber" "PrivateLevel1RegionalNumber" "PrivatePISNSpecificNumber" "PrivateLocalNumber" \
        "PrivateAbbreviatedNumber" "NationalStandardPartyNumber" } \
        value { "PUU" "PUI" "PUN0" "PUNS" "PUS" "PUA" "D" "T" "PRU" "PRL2" "PRL1" "PRP" "PRL0" "PRA" "N" } {
            $entryBase.menBox.type.01.02 add radiobutton -indicatoron 0 -label $name -value "PN$value" \
                -variable tmp($entryBase,aliasType) -command "$entryBase.name delete 0 end"
    }
    $entryBase.menBox.type.01 add cascade -label PN -menu $entryBase.menBox.type.01.02
    entry $entryBase.name -borderwidth 1 -width 1 -validate key -invcmd bell \
            -vcmd "alias:validate %P $entryBase"
    button $entryBase.add -highlightthickness 0 -padx 0 -pady 0 -text "Add Alias" \
            -command "alias:add $listBase.txt $entryBase" -image sunkDown
    button $entryBase.del -highlightthickness 0 -padx 0 -pady 0 -text "Del Alias" \
            -command "alias:del $listBase.txt" -image sunkSlash
    button $entryBase.clr -highlightthickness 0 -padx 0 -pady 0 -text "Clear Aliases" \
            -command "$listBase.txt delete 0 end" -image sunkX
    scrollbar $listBase.scrl -command "$listBase.txt yview"
    listbox $listBase.txt -borderwidth 1 -background White -yscrollcommand "$listBase.scrl set" \
        -selectmode multiple -height 1 -width 1

    ###########
    # BINDINGS
    ###########
    bind $entryBase.name <Return> "$entryBase.add invoke"
    bind $listBase <Delete> "$entryBase.del invoke"

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $entryBase 1 -weight 1
    grid rowconf $entryBase 0 -minsize 4
    grid $entryBase.menBox -in $entryBase -column 0 -row 1 -pady 2 -sticky we -padx 1
    grid columnconf $entryBase.menBox 0 -weight 1
    grid $entryBase.menBox.type -in $entryBase.menBox -column 0 -row 0 -ipady 2 -sticky we
    grid $entryBase.name -in $entryBase -column 1 -row 1 -ipadx 1 -ipady 2 -sticky we -padx 1
    grid $entryBase.add -in $entryBase -column 2 -row 1 -padx 1 -pady 2
    grid $entryBase.del -in $entryBase -column 3 -row 1 -padx 1 -pady 2
    grid $entryBase.clr -in $entryBase -column 4 -row 1 -padx 1 -pady 2
    grid columnconf $listBase 0 -weight 1
    grid rowconf $listBase 0 -weight 1
    grid $listBase.scrl -in $listBase -column 1 -row 0 -sticky ns
    grid $listBase.txt -in $listBase -column 0 -row 0 -sticky nesw

    ###########
    # BALLOONS
    ###########
    set typeStr [string tolower $aliasLabel]
    balloon:set $entryBase.menBox "Select type of $typeStr alias to add"
    balloon:set $entryBase.name "Enter $typeStr alias"
    balloon:set $entryBase.add "Add alias to $typeStr parameters (Return)"
    balloon:set $entryBase.del "Delete selected aliases from $typeStr parameters (Delete)"
    balloon:set $entryBase.clr "Delete all aliases from $typeStr parameters"
    balloon:set $listBase.txt "Select/deselect with mouse button 1, delete with Del"

    ########
    # OTHER
    ########
    # Place cursor in input window
    focus $entryBase.name
}

# alias:validate
# A validator (used with the validate option), which will validate the alias according to it's type.
# input  : alias - the alias to validate
#          entryBase - used to get the alias type, the alias entrybase.
# output : none
# return : true is valid alias, false o.w.
proc {alias:validate} {alias entryBase} {
    global tmp app
    switch [string range $tmp($entryBase,aliasType) 0 1] {
        TE {
            return [ expr { [expr { [eval {string length $alias}] < 129 } ]  &&
                            [string is digit [string map {# 0 * 0 , 0} $alias] ]    } ]
        }
        NA {
            return [ expr { [eval {string length $alias}] < 257 } ]
        }
        UR {
            return [ expr { [expr { [eval {string length $alias}] < 513 } ]         &&
                            [string is alnum [string map {: 0 . 0 / 0} $alias] ]   } ]
        }
        TN {
            return [ expr { [expr { [eval {string length $alias}] < 65 } ]   &&
                            [string is digit [string map {: 0 . 0} $alias] ]    } ]
        }
        EM {
            return [ expr { [expr { [string length $alias] < 513 } ]             &&
                            [string is alnum [string map {@ 0 . 0 _ 0} $alias] ]    } ]
        }
        PN {
            return [ expr { [expr { [eval {string length $alias}] < 257 } ] &&
                            [string is alnum [string map {: 0} $alias] ]   } ]
        }
    }
    return 1
}

##############################################################################################
#
#   Frame Headers
#
##############################################################################################

proc placeHeader { forFrame headerText } {
    set win [winfo toplevel $forFrame]

    # Create the label and make sure we'll display it when the time comes
    label $forFrame:header -text $headerText -borderwidth 0 -padx 2
    bind $win <Expose> "+replaceHeader $forFrame"
}

proc replaceHeader { forFrame } {
    set x [winfo x $forFrame]
    set y [winfo y $forFrame]

    if {($x != 0) || ($y != 0)} {
        # We only place the header when we're sure the frame is mapped and gridded
        place $forFrame:header -x [expr { $x + 9 } ] -y [expr { $y - 7 } ] \
            -anchor nw -bordermode ignore
    }
}

##############################################################################################
#
#   DataType
#
##############################################################################################

# cfgDataType:create
# create generic menu widget for the Data Type parameter
# input  : widget - path of the dataType widget
#        : dataTypeVar - the variable of the combo box of data types
# output : none
# return : the string that was built from the parameters
proc cfgDataType:create {widget} {
    global tmp app

    set dataTypes [api:app:GetDataTypes]
    frame $widget.cfg -borderwidth 2 -relief groove -width 30

    label $widget.cfg.cfgLab -text "CFG Data types"
    menubutton $widget.cfg.cfgMenu \
        -borderwidth 1 -height 1 -indicatoron 1 -menu $widget.cfg.cfgMenu.m \
        -padx 0 -pady 2 -relief raised -textvariable tmp($widget,dataTypeVar)
    menu $widget.cfg.cfgMenu.m -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach dataType $dataTypes {
        $widget.cfg.cfgMenu.m add radiobutton -indicatoron 0 -value $dataType \
            -variable tmp($widget,dataTypeVar) -label [lindex $dataType 0]
    }

    grid columnconf $widget.cfg 1 -weight 1
    grid $widget.cfg.cfgLab -in $widget.cfg -column 0 -row 0 -sticky w
    grid $widget.cfg.cfgMenu -in $widget.cfg -column 1 -row 0 -sticky ew -padx 3 -pady 1

    return "$widget.cfg"
}

##############################################################################################
#
#   History Entry
#
##############################################################################################

proc histEnt:create { entryBase bindButton varName {tip ""}} {
    global app tmp

    frame $entryBase.histEnt -borderwidth 2 -relief sunk
    grid columnconfig $entryBase.histEnt 0 -weight 1
    grid columnconfig $entryBase.histEnt 1 -minsize 28
    grid rowconfig $entryBase.histEnt 0 -weight 1

    menubutton $entryBase.histEnt.lastNames -borderwidth 0 -indicatoron 1 \
        -menu $entryBase.histEnt.lastNames.01 -padx 0 -pady 0 -relief raised -text "" \
        -pady 1 -padx 7
    menu $entryBase.histEnt.lastNames.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    entry $entryBase.histEnt.name -borderwidth 0 -textvariable tmp($varName) -width 1 \
        -validate key -vcmd { expr { [string length %P] < 250 } } -invcmd bell

    grid $entryBase.histEnt.lastNames -in $entryBase.histEnt -column 0 -row 0 -columnspan 2 -sticky nswe
    grid $entryBase.histEnt.name -in $entryBase.histEnt -column 0 -row 0 -sticky nswe -ipady 0

    set command "+
            histEnt:add $entryBase $varName"
    bind $bindButton <1> $command
    set tmp($varName) ""

    #inserting the ip addresses that were saved in the file to the combo box
    for {set i 0} {$i <= 5} {incr i} {
        if {[array get app "$entryBase,ip,$i"]!=""} {
            $entryBase.histEnt.lastNames.01 insert end radiobutton -indicatoron 0 \
                -label $app($entryBase,ip,$i) -value $app($entryBase,ip,$i) \
                -variable tmp($varName)
        }
    }

    if {$tip != ""} {
        balloon:set $entryBase.histEnt $tip
    }
}

proc histEnt:add {entryBase varName} {
    global tmp app

    if ![winfo viewable $entryBase] {return}
    if [expr { [$entryBase.histEnt.name cget -state] == "disabled" } ] {return}
    if { ![string length $tmp($varName)] } {return}

    foreach index {0 1 2 3 4} {
        if ![string compare [$entryBase.histEnt.lastNames.01 entrycget $index -value] $tmp($varName) ] {
            $entryBase.histEnt.lastNames.01 delete $index
        }
    }
    $entryBase.histEnt.lastNames.01 insert 0 radiobutton -indicatoron 0 \
        -label $tmp($varName) -value $tmp($varName) -variable tmp($varName)
    if [ expr [$entryBase.histEnt.lastNames.01 index end] == 5 ] {$entryBase.histEnt.lastNames.01 delete end}

    #saving the menu items into an array
    if { ![string is digit [$entryBase.histEnt.lastNames.01 index end] ] } {
        set app($entryBase,ip,0) [$entryBase.histEnt.lastNames.01 entrycget 0 -value]
        return
    }
    for {set i 0} {$i <= [$entryBase.histEnt.lastNames.01 index end]} {incr i} {
        set app($entryBase,ip,$i) [$entryBase.histEnt.lastNames.01 entrycget $i -value]
    }
}

##############################################################################################
#
#   Transport Address
#
##############################################################################################

# ip:createEntry
# create generic menu widget for the Data Type parameter
# input  : windowName - window name
#        : entryBase - the path to the widget
#        : okBut - the path to the ok button
# output : none
# return : the string that was built from the parameters
proc ip:createEntry {windowName entryBase okBut varName} {
    global tmp app

    ###################
    # CREATING WIDGETS
    ###################
    frame $entryBase.menBox -borderwidth 1 -relief sunken
    menubutton $entryBase.menBox.type -borderwidth 1 -indicatoron 1 \
        -menu $entryBase.menBox.type.01 -padx 0 -pady 0 -relief raised -text IP \
        -textvariable tmp($windowName,addressType) -width 3
    menu $entryBase.menBox.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach value {IP} {
        $entryBase.menBox.type.01 add radiobutton -indicatoron 0 -label $value -value $value \
        -variable tmp($windowName,addressType) -command "$entryBase.histEnt.name delete 0 end"
    }
    histEnt:create $entryBase $okBut $varName

    ###################
    # SETTING GEOMETRY
    ###################

    grid $entryBase.menBox -in $entryBase -column 0 -row 1 -pady 0 -sticky we -padx 1
    grid columnconf $entryBase.menBox 0 -weight 1
    grid $entryBase.menBox.type -in $entryBase.menBox -column 0 -row 0 -ipady 2 -sticky we
    grid $entryBase.histEnt -in $entryBase -column 1 -row 1 -ipadx 1 -ipady 1 -sticky we -padx 1
    grid columnconf $entryBase 0 -weight 0
    grid columnconf $entryBase 1 -weight 1 -minsize 0
    grid rowconf $entryBase 0 -minsize 0
}

##############################################################################################
#
#   General gui
#
##############################################################################################

# selectedItem
# Select an item from a listbox, returning its string
# input  : lbox - Listbox widget being used
# output : none
# return : String value of the selected item
proc selectedItem {lbox} {

    # Get the selection
    set i [catch {$lbox get [$lbox curselection]} item]

    # Handle error values
    if {$i != 0} then { set item "" }

    # Return the result
    return $item
}

# gui:WheelEvent
# Make sure wheel events are handled properly.
# This causes listboxes, canvases and text widgets to move using the mouse wheel.
# input  : x        - X position of the mouse
#          y        - Y position of the mouse
#          delta    - Delta of mouse wheel movement
# output : none
# return : none
proc gui:WheelEvent { x y delta } {

    # Find out what's the widget we're on
    set act 0
    set widget [winfo containing $x $y]

    if {$widget != ""} {
        # Make sure we've got a vertical scrollbar for this widget
        if {[catch "$widget cget -yscrollcommand" cmd]} return

        if {$cmd != ""} {
            # Find out the scrollbar widget we're using
            set scroller [lindex $cmd 0]

            # Make sure we act
            set act 1
        }
    }

    if {$act == 1} {
        # Now we know we have to process the wheel mouse event
        set xy [$widget yview]
        set factor [expr [lindex $xy 1]-[lindex $xy 0]]

        # Make sure we activate the scrollbar's command
        set cmd "[$scroller cget -command] scroll [expr -int($delta/(120*$factor))] units"
        eval $cmd
    }
}

# gui:SetReadOnly
# Set a text widget as readonly or writable
# input  : widget       - Text widget to update
#          isReadonly   - 0 if read/write, 1 if readonly
# output : none
# return : none
proc gui:SetReadOnly { widget isReadonly } {
    global readonly

    if {$isReadonly == 1} {
        # Rename the widget's function to make sure we can catch insert and delete commands
        catch {rename $widget _$widget}

        # Declare the new procedure we'll use
        proc $widget {cmd args} [format {
            switch $cmd {
                insert {}
                delete {}
                default {
                    set cmd "_%s $cmd $args"
                    return [eval $cmd]
                }
            }
        } $widget]

        set readonly($widget) 1

    } else {
        if {$readonly($widget)} {
            rename $widget ""
            rename _$widget $widget
            array unset readonly $widget
        }
    }
}

# gui:askQuestion
# open a window containing a yes/no question, and activates a command in accordance with the reply
# input  : Qid    - name for the window (should be unique)
#          Qtitle - window title
#          Qtext  - text of the question
#          yesOp  - operation if yes is pressed
#          noOp   - operation if no is pressed
# output : none
# return : none
proc gui:askQuestion { Qid Qtitle Qtext yesOp noOp } {
    global tmp app

    set base .q$Qid

    if {[winfo exists $base]} {
        wm deiconify $base; return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel
    wm focusmodel $base passive
    wm geometry $base 250x80+300+200
    wm overrideredirect $base 0
    wm resizable $base 0 0
    wm title $base $Qtitle
    wm transient $base .test

    label $base.text -text $Qtext -wraplength 230
    button $base.no -text "No" -width 6 -command "$noOp
            Window delete $base"
    button $base.yes -text "Yes" -width 6 -command "$yesOp
            Window delete $base"

    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid rowconf $base 0 -weight 1
    grid rowconf $base 1 -minsize 30

    grid $base.text -in $base -row 0 -column 0 -columnspan 2 -sticky news
    grid $base.no -in $base -row 1 -column 0
    grid $base.yes -in $base -row 1 -column 1
}

proc gui:centerWindow { winToCenter {winInside main} } {
    update
    set w $winToCenter

    if {$winInside == "main"} {
        set xmax [winfo screenwidth $w]
        set ymax [winfo screenheight $w]
        set x0 0
        set y0 0
    } else {
        set xmax [winfo reqwidth $winInside]
        set ymax [winfo reqheight $winInside]
        set x0 [winfo x $winInside]
        set y0 [winfo y $winInside]
    }
    set x [expr {$x0+(($xmax - [winfo reqwidth $w])/2)}]
    set y [expr {$y0+(($ymax - [winfo reqheight $w])/2)}]
    wm minsize $w [winfo reqwidth $w] [winfo reqheight $w]
    wm geometry $w "+$x+$y"
}

proc gui:selectedActive { menuWdg } {
    set index [$menuWdg index end]
    if {$index == "none"} {set index 0}
    incr index
    return "$menuWdg activate $index"
}

