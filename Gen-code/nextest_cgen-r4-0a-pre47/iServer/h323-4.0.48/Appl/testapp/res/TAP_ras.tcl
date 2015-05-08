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
#                                 TAP_ras.tcl
#
#   RAS handling.
#   This file holds all the GUI procedures for ep-gk communication: RRQ, GRQ, URQ, etc.
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
#   REGISTRATION operations
#
##############################################################################################


# rgrq:GetRegInfo
# Get the registration information from the configuration and
# put it inside the RAS tab of the test application
# input  : none
# output : none
# return : none
proc rgrq:GetRegInfo {} {
    global tmp app

    set base $tmp(rasWidget)

    # Clear before we begin
    $base.rglist.txt delete 0 end

    # Fetch some node ids
    set valTree [api:cm:GetValTree]
    set rasConfNode [api:cm:GetRASConfigurationHandle]

    catch {
        set aliasesNode [api:pvt:GetByPath $valTree $rasConfNode "registrationInfo.terminalAlias"]

        # Get the aliases while we can
        set index 1
        set aliasNode [api:pvt:GetByPath $valTree $aliasesNode $index]
        while {$aliasNode >= 0} {
            $base.rglist.txt insert end [api:cm:Vt2Alias $valTree $aliasNode]

            # Get the next one
            set aliasNode -1
            catch {
                incr index
                set aliasNode [api:pvt:GetByPath $valTree $aliasesNode $index]
            }
        }
    }

    catch {
        set gkNode [api:pvt:GetByPath $valTree $rasConfNode "registrationInfo.gatekeeperIdentifier"]
        set tmp(rgrq,gkAlias) [api:pvt:GetString $valTree $gkNode]
    }

    set tmp(rgrq,address) ""
    catch {
        set gkNode [api:pvt:GetByPath $valTree $rasConfNode "manualDiscovery.defaultGatekeeper"]
        set tmp(rgrq,address) [api:app:Vt2Address $valTree $gkNode]
    }
}


# rgrq:IsAutoRas
# Checks if the configuration uses automatic RAS or not
# input  : none
# output : none
# return : 1 if automatic RAS is used, 0 otherwise
proc rgrq:IsAutoRas {} {
    # Fetch some node ids
    set valTree [api:cm:GetValTree]
    set rasConfNode [api:cm:GetRASConfigurationHandle]

    if { [catch {api:pvt:GetByPath $valTree $rasConfNode "manualRAS"}] } {
        # No ManualRAS flag - we're automatic...
        return 1
    }

    # ManualRAS used
    return 0
}


# rgrq:register
# Register the endpoint
# This procedure is called when the "OK" button is pressed inside the registration window
# input  : none
# output : none
# return : none
proc rgrq:register {} {
    global tmp app

    set base $tmp(rasWidget)

    set valTree [api:cm:GetValTree]
    set rasConfNode [api:cm:GetRASConfigurationHandle]

    # Remove current aliases in configuration
    catch {
        set aliasesNode [api:pvt:GetByPath $valTree $rasConfNode "registrationInfo.terminalAlias"]
        api:pvt:Delete $valTree $aliasesNode
    }

    # Put the ones in the listbox
    set index 0
    set aliasesList [$base.rglist.txt get 0 end]
    if {[llength $aliasesList] > 0} {
        set aliasesNode [api:pvt:BuildByPath $valTree $rasConfNode "registrationInfo.terminalAlias" 0]
        foreach alias  $aliasesList {
            incr index
            set nodeId [api:pvt:BuildByPath $valTree $aliasesNode $index 0]
            api:cm:Alias2Vt $valTree $alias $nodeId
        }
    }

    # Remove current gatekeeper's IP address
    catch {
        set nodeId [api:pvt:GetByPath $valTree $rasConfNode "manualDiscovery"]
        api:pvt:Delete $valTree $nodeId
    }

    # Set the gk's IP address if we have one
    if {$tmp(rgrq,address) != ""} {
        set nodeId [api:pvt:BuildByPath $valTree $rasConfNode "manualDiscovery.defaultGatekeeper" 0]
        if {[catch {api:app:Address2Vt $valTree $tmp(rgrq,address) $nodeId}]} {
            set nodeId [api:pvt:GetByPath $valTree $rasConfNode "manualDiscovery"]
            api:pvt:Delete $valTree $nodeId
        }
    }

    # Remove current gatekeeper Alias
    catch {
        set nodeId [api:pvt:GetByPath $valTree $rasConfNode "registrationInfo.gatekeeperIdentifier"]
        api:pvt:Delete $valTree $nodeId
    }

    # Set the gk Alias if we have one
    if {$tmp(rgrq,gkAlias) != ""} {
        set nodeId [api:pvt:BuildByPath $valTree $rasConfNode "registrationInfo.gatekeeperIdentifier" $tmp(rgrq,gkAlias)]
    }

    # Register the endpoint
    api:cm:Register
}


