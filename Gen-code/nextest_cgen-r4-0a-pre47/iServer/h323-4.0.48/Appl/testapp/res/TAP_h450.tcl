
proc test:h450Tab {base tabButton} {
    global tmp app

    set tmp(h450Button) $tabButton
    set tmp(h450Tab) $base

    #default reply
    frame $base.reply -borderwidth 2 -relief groove
    checkbutton $base.reply.ask -tooltip "Display message asking for user replay" \
        -text "Ask Me" -variable tmp(h450,ask) -command "h450replyButtons $base"
    radiobutton $base.reply.conf -tooltip "Confirm any services requested" \
        -text "Default Confirm" -variable tmp(h450,defAns) -value 1
    radiobutton $base.reply.rejc -tooltip "Reject any services requested" \
        -text "Default Reject" -variable tmp(h450,defAns) -value 0
    set tmp(h450,defAns) 1
    set tmp(h450,ask) 0


    # Services
    ################
    frame $base.service -borderwidth 2 -relief groove

    #transfer - 450.2
    frame $base.service.trans -borderwidth 0
    button $base.service.trans.act -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callTransfer [selectedItem .test.calls.list] $app(h450,trnAddr) $app(h450,trnCall)} \
        -tooltip "Transfer a call to a secondary call"
    entry $base.service.trans.entAddr -textvariable app(h450,trnAddr) -tooltip "Fill address to transfer to..."
    menubutton $base.service.trans.call -borderwidth 1 -height 1 -width 10 -anchor w -indicatoron 1 \
        -tooltip "...or Choose a secondary call" \
        -menu $base.service.trans.call.01 -relief raised -text "Select Call" -textvariable app(h450,trnCall)
    menu $base.service.trans.call.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    bind $base.service.trans.call <Enter> "callSelect:updt $base.service.trans.call.01 app(h450,trnCall) {$base.service.trans.entAddr delete 0 end} "

    #forward and reroute - 450.3
    frame $base.service.fwdrrt -borderwidth 0
    radiobutton $base.service.fwdrrt.selectFwd -text "Forward" -variable app(h450,fwdrrt) -value 0 \
        -command { global app
            place forget $tmp(h450Tab).service.fwdrrt.rrt
            place forget $tmp(h450Tab).service.fwdrrt.res
            place forget $tmp(h450Tab).service.fwdrrt.intr
            place $tmp(h450Tab).service.fwdrrt.entAddr -in $tmp(h450Tab).service.fwdrrt -anchor n -relx 0.5 -y 40
            place $tmp(h450Tab).service.fwdrrt.cond -in $tmp(h450Tab).service.fwdrrt -anchor n -relx 0.5 -y 90
            place $tmp(h450Tab).service.fwdrrt.act -in $tmp(h450Tab).service.fwdrrt -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.fwdrrt.dea -in $tmp(h450Tab).service.fwdrrt -anchor c -x 396 -y 150
        }
    radiobutton $base.service.fwdrrt.selectRrt -text "Reroute" -variable app(h450,fwdrrt) -value 1 \
        -command { global app
            place forget $tmp(h450Tab).service.fwdrrt.act
            place forget $tmp(h450Tab).service.fwdrrt.dea
            place forget $tmp(h450Tab).service.fwdrrt.cond
            place forget $tmp(h450Tab).service.fwdrrt.intr
            place $tmp(h450Tab).service.fwdrrt.entAddr -in $tmp(h450Tab).service.fwdrrt -anchor n -relx 0.5 -y 40
            place $tmp(h450Tab).service.fwdrrt.res -in $tmp(h450Tab).service.fwdrrt -anchor n -relx 0.5 -y 90
            place $tmp(h450Tab).service.fwdrrt.rrt -in $tmp(h450Tab).service.fwdrrt -anchor c -x 330 -y 150
        }
    radiobutton $base.service.fwdrrt.selectInt -text "Interrogate" -variable app(h450,fwdrrt) -value 2 \
        -command { global app
            place forget $tmp(h450Tab).service.fwdrrt.act
            place forget $tmp(h450Tab).service.fwdrrt.dea
            place forget $tmp(h450Tab).service.fwdrrt.rrt
            place forget $tmp(h450Tab).service.fwdrrt.res
            place forget $tmp(h450Tab).service.fwdrrt.entAddr
            place $tmp(h450Tab).service.fwdrrt.cond -in $tmp(h450Tab).service.fwdrrt -anchor n -relx 0.5 -y 90
            place $tmp(h450Tab).service.fwdrrt.intr -in $tmp(h450Tab).service.fwdrrt -anchor c -x 330 -y 150
        }
    button $base.service.fwdrrt.act -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.forwardActivate $app(h450,fwdRrtAddr) $app(h450,fwdCond) } \
        -tooltip "Activate the Call Forwarding service"
    button $base.service.fwdrrt.dea -text "Deactivate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.forwardDeactivate $app(h450,fwdRrtAddr) $app(h450,fwdCond) } \
        -tooltip "Cancel the Call Forwarding service"
    button $base.service.fwdrrt.rrt -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callReroute [selectedItem .test.calls.list] $app(h450,fwdRrtAddr) $app(h450,rrtRes) } \
        -tooltip "Reroute an incoming call"
    button $base.service.fwdrrt.intr -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.forwardInterrogate $app(h450,fwdCond) } -tooltip "Quary destination for forward list"
    entry $base.service.fwdrrt.entAddr -textvariable app(h450,fwdRrtAddr) -tooltip "Fill the address to which to Forward/Reroute"
    frame $base.service.fwdrrt.tul -borderwidth 0 -relief raised
    menubutton $base.service.fwdrrt.cond -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -tooltip "Choose the conditions in which to Forward" \
        -menu $base.service.fwdrrt.cond.01 -relief raised -text "Unconditional" -textvariable app(h450,fwdCond)
    menu $base.service.fwdrrt.cond.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach type { Unconditional Busy NoResponse } {
        $base.service.fwdrrt.cond.01 add radiobutton \
            -indicatoron 0 -value $type -variable app(h450,fwdCond) -label $type
    }
    menubutton $base.service.fwdrrt.res -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -tooltip "Choose Reroute reason" \
        -menu $base.service.fwdrrt.res.01 -relief raised -text "Unknown" -textvariable app(h450,rrtRes)
    menu $base.service.fwdrrt.res.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach type { Unknown Unconditional Busy NoResponse } {
        $base.service.fwdrrt.res.01 add radiobutton \
            -indicatoron 0 -value $type -variable app(h450,rrtRes) -label $type
    }

    #hold - 450.4
    frame $base.service.hold -borderwidth 0
    button $base.service.hold.act -text "Activate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callHold [selectedItem .test.calls.list] $app(h450,nearHold) } \
        -tooltip "Puts a call on Hold"
    button $base.service.hold.ret -text "Deactivate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callHoldRtrv [selectedItem .test.calls.list] } \
        -tooltip "Retrieves a call from Hold"
    checkbutton $base.service.hold.near -text "Near" -variable app(h450,nearHold) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Chooses between Near Hold and Remote Hold"

    #park pickup - 450.5
    frame $base.service.prkpku -borderwidth 0
    radiobutton $base.service.prkpku.selectPrk -text "Park" -variable app(h450,prkpku) -value 0 \
        -command { global app
            place forget $tmp(h450Tab).service.prkpku.actPku
            place forget $tmp(h450Tab).service.prkpku.calls
            place forget $tmp(h450Tab).service.prkpku.localPk
            place $tmp(h450Tab).service.prkpku.actPrk      -in $tmp(h450Tab).service.prkpku -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.prkpku.enablePrk   -in $tmp(h450Tab).service.prkpku -anchor w -x 310 -y 70
            place $tmp(h450Tab).service.prkpku.alertingPrk -in $tmp(h450Tab).service.prkpku -anchor w -x 310 -y 90
            place $tmp(h450Tab).service.prkpku.insEP       -in $tmp(h450Tab).service.prkpku -anchor n -relx 0.475 -y 65
            place $tmp(h450Tab).service.prkpku.delEP       -in $tmp(h450Tab).service.prkpku -anchor n -relx 0.525 -y 65
            place $tmp(h450Tab).service.prkpku.eps         -in $tmp(h450Tab).service.prkpku -anchor n -relx 0.5 -y 90
            place $tmp(h450Tab).service.prkpku.parkingLotAddr      -in $tmp(h450Tab).service.prkpku -anchor n -relx 0.5 -y 40
        }
    radiobutton $base.service.prkpku.selectPku -text "Pickup" -variable app(h450,prkpku) -value 1 \
        -command { global app
            place forget $tmp(h450Tab).service.prkpku.actPrk
            place forget $tmp(h450Tab).service.prkpku.enablePrk
            place forget $tmp(h450Tab).service.prkpku.alertingPrk
            place forget $tmp(h450Tab).service.prkpku.eps
            place forget $tmp(h450Tab).service.prkpku.delEP
            place forget $tmp(h450Tab).service.prkpku.insEP
            place forget $tmp(h450Tab).service.prkpku.parkingLotAddr
            place $tmp(h450Tab).service.prkpku.actPku -in $tmp(h450Tab).service.prkpku -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.prkpku.calls -in $tmp(h450Tab).service.prkpku -anchor n -relx 0.5 -y 90
            place $tmp(h450Tab).service.prkpku.localPk -in $tmp(h450Tab).service.prkpku -anchor w -x 310 -y 70
        }
    button $base.service.prkpku.actPrk -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callPark [selectedItem .test.calls.list] $app(h450,parkingLotAddr) } \
        -tooltip "Activate the Park service"
    button $base.service.prkpku.actPku -text "Activate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { eval "H450.callPick \"[selectedItem .test.calls.list]\" $tmp(h450,callPku)" } \
        -tooltip "Activate the Pickup service"
    entry $base.service.prkpku.parkingLotAddr -textvariable app(h450,parkingLotAddr) \
        -tooltip "Fill the address of the parking lot"
    menubutton $base.service.prkpku.calls -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.prkpku.calls.01 -relief raised -text "Pick Call" -textvariable tmp(h450,callPku) \
        -tooltip "Choose a call to pick up"
    menu $base.service.prkpku.calls.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    button $base.service.prkpku.insEP -borderwidth 1 -highlightthickness 0 -image sunkDown \
        -command { $tmp(h450Tab).service.prkpku.eps.01 add radiobutton -indicatoron 0 \
                -label $app(h450,parkingLotAddr) -command [ gui:selectedActive $tmp(h450Tab).service.prkpku.eps.01 ]
            }
    button $base.service.prkpku.delEP -borderwidth 1 -highlightthickness 0 -image sunkSlash \
        -command { $tmp(h450Tab).service.prkpku.eps.01 delete active
                $tmp(h450Tab).service.prkpku.eps.01 activate none
        }
    menubutton $base.service.prkpku.eps -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.prkpku.eps.01 -relief raised -text "EndPoints" -textvariable tmp(h450,notifyEP) \
        -tooltip "List of notified EndPoints"
    menu $base.service.prkpku.eps.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    checkbutton $base.service.prkpku.enablePrk -text "Enable" -variable app(h450,enablePrk) \
        -highlightthickness 0 -width 20 -anchor w -tooltip "Enables being parked in"
    checkbutton $base.service.prkpku.alertingPrk -text "Park on Alerting" -variable app(h450,alertingPrk) \
        -highlightthickness 0 -width 20 -anchor w -tooltip "Activates the Park service when sending Alerting"
    checkbutton $base.service.prkpku.localPk -text "Local Pickup" -variable app(h450,localPickup) \
        -highlightthickness 0 -width 20 -anchor w -tooltip "Pickup from the local (this) endpoint"

    #wait - 450.6
    frame $base.service.wait -borderwidth 0
    button $base.service.wait.act -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callWait [selectedItem .test.calls.list] } \
        -tooltip "Activate call waiting service for selected call"
    checkbutton $base.service.wait.disable -text "Disable" -variable app(h450,disableWait) \
        -highlightthickness 0 -width 20 -anchor w \
        -tooltip "Disables the Call Waiting service for future calls"

    #message waiting - 450.7
    frame $base.service.mwi -borderwidth 0
    radiobutton $base.service.mwi.selectMes -text "Message" -variable app(h450,indication) -value 0 \
        -command { global app
            place forget $tmp(h450Tab).service.mwi.callBack
            place forget $tmp(h450Tab).service.mwi.deactivateCB
            place forget $tmp(h450Tab).service.mwi.interogate
            place $tmp(h450Tab).service.mwi.message -in $tmp(h450Tab).service.mwi -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.mwi.deactivateMes -in $tmp(h450Tab).service.mwi -anchor c -x 396 -y 150
        }
    radiobutton $base.service.mwi.selectCB -text "Callback" -variable app(h450,indication) -value 1 \
        -command { global app
            place forget $tmp(h450Tab).service.mwi.message
            place forget $tmp(h450Tab).service.mwi.deactivateMes
            place forget $tmp(h450Tab).service.mwi.interogate
            place $tmp(h450Tab).service.mwi.callBack -in $tmp(h450Tab).service.mwi -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.mwi.deactivateCB -in $tmp(h450Tab).service.mwi -anchor c -x 396 -y 150
        }
    radiobutton $base.service.mwi.selectIntr -text "Interrogate" -variable app(h450,indication) -value 2 \
        -command { global app
            place forget $tmp(h450Tab).service.mwi.callBack
            place forget $tmp(h450Tab).service.mwi.deactivateCB
            place forget $tmp(h450Tab).service.mwi.message
            place forget $tmp(h450Tab).service.mwi.deactivateMes
            place $tmp(h450Tab).service.mwi.interogate -in $tmp(h450Tab).service.mwi -anchor c -x 330 -y 150
        }
    button $base.service.mwi.message -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.MC.ActivateMessage $MWIsendTo } \
        -tooltip "Send a Message Indication to client"
    button $base.service.mwi.callBack -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.MC.ActivateCallBack $MWIsendTo } \
        -tooltip "Send a Callback Indication to client"
    button $base.service.mwi.deactivateMes -text "Deactivate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.MC.Deactivate $MWIsendTo 0 } \
        -tooltip "Send a Message Deactivation to client"
    button $base.service.mwi.deactivateCB -text "Deactivate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.MC.Deactivate $MWIsendTo 1 } \
        -tooltip "Send a CallBack Deactivation to client"
    button $base.service.mwi.interogate -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.SU.Interogate $MWIsendTo } \
        -tooltip "Send an Interogation message to server"
    entry  $base.service.mwi.sendTo -textvariable MWIsendTo -tooltip "Fill here the address of the client/server"

    #name ID - 450.8
    frame $base.service.nameID -borderwidth 0
    menubutton $base.service.nameID.type -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.nameID.type.01 -relief raised -text None -textvariable app(h450,nameType) \
        -tooltip "Choose type of name ID"
    menu $base.service.nameID.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach type {Allowed Restricted NA None} state { normal normal disabled disabled } {
        $base.service.nameID.type.01 add radiobutton \
            -indicatoron 0 -value $type -variable app(h450,nameType) -label $type \
            -command "$base.service.nameID.entName delete 0 end
                    $base.service.nameID.entName config -state $state"
    }
    entry $base.service.nameID.entName -textvariable app(h450,nameID) \
        -tooltip "Fill name ID for Allowed and Restricted types"

    #completion -450.9
    frame $base.service.comp -borderwidth 0
    radiobutton $base.service.comp.a1 -text "User A - Busy" -variable app(h450,completion) -value 1 \
        -command {h4509SetSide 1} -tooltip "Handle User A's actions when it's on Busy"
    radiobutton $base.service.comp.a2 -text "User A - NoResponse" -variable app(h450,completion) -value 0 \
        -command {h4509SetSide 0} -tooltip "Handle User A's actions when it's on NoResponse"
    radiobutton $base.service.comp.b -text "User B" -variable app(h450,completion) -value 2 \
        -command {h4509SetSide 2} -tooltip "Handle User B's actions"
    button $base.service.comp.activate -text "Activate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command {H450.callCompletion $app(h450,completion) $app(h450,compAddr) [selectedItem .test.calls.list]} \
        -tooltip "Send call Setup with completion"
    button $base.service.comp.cancel -text "Cancel" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command {H450.callCompletionAction Cancel $tmp(h450,compService)} \
        -tooltip "Cancel selected completion service"
    button $base.service.comp.resume -text "Resume" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command {H450.callCompletionAction Resume $tmp(h450,compService)} \
        -tooltip "Indicate User B that User A isn't busy after Suspend"
    button $base.service.comp.ringout -text "Ringout" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command {H450.callCompletionAction Ringout $tmp(h450,compService)} \
        -tooltip "Complete the call from User A to User B"
    button $base.service.comp.execPossible -text "ExecPossible" -width 12 -borderwidth 1 -highlightthickness 0 \
        -state disable -command {H450.callCompletionAction ExecPossible $tmp(h450,compService)} \
        -tooltip "Indicate User A that User B isn't busy anymore"
    entry $base.service.comp.entAddr -textvariable app(h450,compAddr) -tooltip "Fill address to send Setup to"
    label $base.service.comp.callLab -text "Send with"
    menubutton $base.service.comp.call -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.comp.call.01 -relief raised -text <None> -textvariable tmp(h450,compService) \
        -tooltip "Open service to work with"
    menu $base.service.comp.call.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    label $base.service.comp.servLab -text "Receive to"
    menubutton $base.service.comp.serv -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.comp.serv.01 -relief raised -text <None> -textvariable tmp(h450,compFoundService) \
        -tooltip "Route incoming messages to the selected service"
    menu $base.service.comp.serv.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0

    checkbutton $base.service.comp.canRetainServ -text "Can Retain Service" -variable app(h450,canRetainServ) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Indication if User A can retain the service"
    checkbutton $base.service.comp.retainServ -text "Retain Service" -variable app(h450,retainServ) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "User B chooses to retain service or not"
    checkbutton $base.service.comp.retainConn -text "Retain Connection" -variable app(h450,retainConn) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Check to indicate signalling connection retention throughout the service lifetime"
    checkbutton $base.service.comp.busy -text "Busy" -variable app(h450,compBusy) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Indicate that the endpoint is busy or to use BSRequest and not NRRequest"

    #offering - 450.10
    frame $base.service.offer -borderwidth 0
    radiobutton $base.service.offer.selectSend -text "Send" -variable app(h450,offer) -value 0 \
        -command { global app
            place forget $tmp(h450Tab).service.offer.in
            place forget $tmp(h450Tab).service.offer.rua
            place $tmp(h450Tab).service.offer.out -in $tmp(h450Tab).service.offer -anchor n -relx 0.5 -y 90
            place $tmp(h450Tab).service.offer.send -in $tmp(h450Tab).service.offer -anchor c -x 330 -y 150
        }
    radiobutton $base.service.offer.selectRet -text "Accept" -variable app(h450,offer) -value 1 \
        -command { global app
            place forget $tmp(h450Tab).service.offer.send
            place forget $tmp(h450Tab).service.offer.out
            place $tmp(h450Tab).service.offer.rua -in $tmp(h450Tab).service.offer -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.offer.in -in $tmp(h450Tab).service.offer -anchor n -relx 0.5 -y 90
        }
    button $base.service.offer.send -text "Activate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callOffer $app(h450,offerAddr) $app(h450,overideCFB) } \
        -tooltip "Send call Setup with offering"
    button $base.service.offer.rua -text "Activate" -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.remoteUserAlerting [selectedItem .test.calls.list] } -tooltip "Send RUA"
    entry $base.service.offer.entAddr -textvariable app(h450,offerAddr) \
        -tooltip "Fill address to send Setup to"
    menubutton $base.service.offer.in -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.offer.in.01 -relief raised -text "Select Call" -textvariable app(h450,offerIn) \
        -tooltip "Incomming calls offered to"
    menu $base.service.offer.in.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    menubutton $base.service.offer.out -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.offer.out.01 -relief raised -text "Select Call" -textvariable app(h450,offerOut) \
        -tooltip "Outgoing calls offered"
    menu $base.service.offer.out.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    checkbutton $base.service.offer.overide -text "Overide CFB" -variable app(h450,overideCFB) \
        -highlightthickness 0 -width 0 -anchor w \
        -tooltip "Check to overide the destination's Call Forwarding on Busy"
    checkbutton $base.service.offer.full -text "Waiting List Full" -variable app(h450,waitListFull) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Check to indicate that waiting list is full"

    #intrusion - 450.11
    frame $base.service.intr -borderwidth 0
    radiobutton $base.service.intr.selectFR -text "Forced Release" -variable app(h450,intrusion) -value 0 \
        -command { global tmp
            place forget $tmp(h450Tab).service.intr.sm
            place forget $tmp(h450Tab).service.intr.ir
            place forget $tmp(h450Tab).service.intr.post
            place forget $tmp(h450Tab).service.intr.callID
            place forget $tmp(h450Tab).service.intr.getCallID
            place $tmp(h450Tab).service.intr.fr -in $tmp(h450Tab).service.intr -anchor c -x 330 -y 150
        }
    radiobutton $base.service.intr.selectIR -text "Intrusion Request" -variable app(h450,intrusion) -value 1 \
        -command { global tmp
            place forget $tmp(h450Tab).service.intr.sm
            place forget $tmp(h450Tab).service.intr.fr
            place forget $tmp(h450Tab).service.intr.callID
            place forget $tmp(h450Tab).service.intr.getCallID
            place $tmp(h450Tab).service.intr.ir -in $tmp(h450Tab).service.intr -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.intr.post -in $tmp(h450Tab).service.intr -anchor n -relx 0.5 -y 90
        }
    radiobutton $base.service.intr.selectSM -text "Silent Monitor" -variable app(h450,intrusion) -value 2 \
        -command { global tmp
            place forget $tmp(h450Tab).service.intr.ir
            place forget $tmp(h450Tab).service.intr.fr
            place forget $tmp(h450Tab).service.intr.post
            place $tmp(h450Tab).service.intr.sm -in $tmp(h450Tab).service.intr -anchor c -x 330 -y 150
            place $tmp(h450Tab).service.intr.callID -in $tmp(h450Tab).service.intr -anchor n -relx 0.55 -y 90
            place $tmp(h450Tab).service.intr.getCallID -in $tmp(h450Tab).service.intr -anchor n -relx 0.45 -y 90
        }
    button $base.service.intr.fr -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callIntrusion $app(h450,intrDestAddr) fr "" $app(h450,intrUseWaiting) [selectedItem .test.calls.list] } \
        -tooltip "Send Forced Release to destination"
    button $base.service.intr.ir -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callIntrusion $app(h450,intrDestAddr) ir $app(h450,intrPostIntr) $app(h450,intrUseWaiting) [selectedItem .test.calls.list] } \
        -tooltip "Send intrusion to destination or post intrusion request on call"
    button $base.service.intr.sm -text "Activate"  -width 8 -borderwidth 1 -highlightthickness 0 \
        -command { H450.callIntrusion $app(h450,intrDestAddr) sm "" $app(h450,intrUseWaiting) [selectedItem .test.calls.list] } \
        -tooltip "Activate silent monitoring for destination's call"
    entry  $base.service.intr.sendTo -textvariable app(h450,intrDestAddr) \
        -tooltip "Fill here the address of the destination"
    menubutton $base.service.intr.post -borderwidth 1 -height 1 -width 10 -indicatoron 1 \
        -menu $base.service.intr.post.01 -relief raised -text "" -textvariable app(h450,intrPostIntr) \
        -tooltip "Choose post intrusion request"
    menu $base.service.intr.post.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach type { "" "forced release" "wait on busy" "isolation" } {
        $base.service.intr.post.01 add radiobutton \
            -indicatoron 0 -value $type -variable app(h450,intrPostIntr) -label $type
    }
    entry  $base.service.intr.callID -textvariable app(h450,intrCallID) -width 6 \
        -tooltip "Fill here the call ID of the monitored call"
    button  $base.service.intr.getCallID -text "call ID" -highlightthickness 0 -borderwidth 0 \
        -command { set app(h450,intrCallID) [H450.getCallID [selectedItem .test.calls.list]] } \
        -tooltip "Click to copy the call ID to clipboad"
    checkbutton $base.service.intr.pos -text "possible" -variable app(h450,intrPossible) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Check to indicate intrusion possible"
    checkbutton $base.service.intr.iso -text "Is Isolated" -variable app(h450,intrIsIsolated) \
        -highlightthickness 0 -width 0 -anchor w -tooltip "Check for isolation, uncheck for conference"
    checkbutton $base.service.intr.use -text "Use Waiting" -variable app(h450,intrUseWaiting) \
        -highlightthickness 0 -width 0 -anchor w \
        -tooltip "Check to use a call that is in \"wait on busy\" mode, and select the call"
    label $base.service.intr.callHandle -text ""

    #selector
    set tmp(h450,base) "$base"
    frame $base.service.action -borderwidth 0
    radiobutton $base.service.action.trans -text "Transfer" -variable app(h450,service) -value 2 \
        -tooltip "Transfer a call to another endpoint (H450.2)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.trans"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news
                }
    radiobutton $base.service.action.fwdrrt -text "Fwd/Rrt" -variable app(h450,service) -value 3 \
        -tooltip "Forward (by server) or Reroute incomming calls to another endpoint (H450.3)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.fwdrrt"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.hold -text "Hold" -variable app(h450,service) -value 4 \
        -tooltip "Puts a call on hold (H450.4)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.hold"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.prkpku -text "Prk/Pku" -variable app(h450,service) -value 5 \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.prkpku"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.wait -text "Wait" -variable app(h450,service) -value 6 \
        -tooltip "Place an incomming call in a call-waiting mode (H450.6)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.wait"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.mwi -text "MWI" -variable app(h450,service) -value 7 \
        -tooltip "Send a Message Waiting or Call Back Indication to client, and Interogate server (H450.7)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.mwi"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.nameID -text "NameID" -variable app(h450,service) -value 8 \
        -tooltip "Send name ID with outgoing Setup, Alerting, Connect and RelComp Busy (H450.8)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.nameID"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.comp -text "Completion" -variable app(h450,service) -value 9 \
        -tooltip "Wait on an endpoint until it is free to answer (H450.9)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.comp"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.offer -text "Offer" -variable app(h450,service) -value 10 \
        -tooltip "Have an endpoint callback when it is ready to answer (H450.10)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.offer"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}
    radiobutton $base.service.action.intr -text "Intrusion" -variable app(h450,service) -value 11 \
        -tooltip "Get in the middle of a call between two other endpoints (H450.11)" \
        -command { global tmp app
                if {$app(h450,action) != ""} { grid remove $app(h450,action) }
                set app(h450,action) "$tmp(h450,base).service.intr"
                grid $app(h450,action) -in $tmp(h450,base).service -row 1 -column 0 -sticky news}

    ########
    # GRID #
    ########

    grid rowconfig $base 0 -minsize 7
    grid rowconfig $base 1 -minsize 24
    grid rowconfig $base 2 -minsize 7
    grid rowconfig $base 3 -weight 1
    grid columnconfig $base 0 -weight 1

    grid $base.reply -in $base -row 1 -column 0 -padx 2 -sticky news
    grid $base.reply.ask  -in $base.reply -row 0 -column 0 -sticky nw
    grid $base.reply.conf -in $base.reply -row 0 -column 1 -sticky nw
    grid $base.reply.rejc -in $base.reply -row 0 -column 2 -sticky nw

    grid $base.service -in $base -row 3 -column 0 -padx 2 -sticky news
    grid rowconfig $base.service 1 -weight 1
    grid columnconfig $base.service 0 -weight 1
    grid $base.service.action -in $base.service -row 0 -column 0 -padx 0 -sticky news
    grid columnconfig $base.service.action 0 -weight 1
    grid columnconfig $base.service.action 1 -weight 1
    grid columnconfig $base.service.action 2 -weight 1
    grid columnconfig $base.service.action 3 -weight 1
    grid columnconfig $base.service.action 4 -weight 1
    grid $base.service.action.trans  -in $base.service.action -row 0 -column 0 -padx 2 -pady 0 -sticky w
    grid $base.service.action.fwdrrt -in $base.service.action -row 0 -column 1 -padx 2 -pady 0 -sticky w
    grid $base.service.action.hold   -in $base.service.action -row 0 -column 2 -padx 2 -pady 0 -sticky w
    grid $base.service.action.prkpku -in $base.service.action -row 0 -column 3 -padx 2 -pady 0 -sticky w
    grid $base.service.action.wait   -in $base.service.action -row 0 -column 4 -padx 2 -pady 0 -sticky w
    grid $base.service.action.mwi    -in $base.service.action -row 1 -column 0 -padx 2 -pady 0 -sticky w
    grid $base.service.action.nameID -in $base.service.action -row 1 -column 1 -padx 2 -pady 0 -sticky w
    grid $base.service.action.comp   -in $base.service.action -row 1 -column 2 -padx 2 -pady 0 -sticky w
    grid $base.service.action.offer  -in $base.service.action -row 1 -column 3 -padx 2 -pady 0 -sticky w
    grid $base.service.action.intr   -in $base.service.action -row 1 -column 4 -padx 2 -pady 0 -sticky w

    place $base.service.trans.entAddr -in $base.service.trans -anchor n -relx 0.5 -y 40
    place $base.service.trans.call -in $base.service.trans -anchor n -relx 0.5 -y 90
    place $base.service.trans.act -in $base.service.trans -anchor c -x 330 -y 150

    place $base.service.fwdrrt.selectFwd -in $base.service.fwdrrt -anchor w -x 20 -y 30
    place $base.service.fwdrrt.selectRrt -in $base.service.fwdrrt -anchor w -x 20 -y 50
    place $base.service.fwdrrt.selectInt -in $base.service.fwdrrt -anchor w -x 20 -y 70
    place $base.service.fwdrrt.tul -in $base.service.fwdrrt -height 8 -width 8 -anchor c -x 390 -y 30
    if {$app(h450,fwdrrt) == 0} {
        place $base.service.fwdrrt.cond -in $base.service.fwdrrt -anchor n -relx 0.5 -y 90
        place $base.service.fwdrrt.act -in $base.service.fwdrrt -anchor c -x 330 -y 150
        place $base.service.fwdrrt.dea -in $base.service.fwdrrt -anchor c -x 396 -y 150
        place $base.service.fwdrrt.entAddr -in $base.service.fwdrrt -anchor n -relx 0.5 -y 40
    }
    if {$app(h450,fwdrrt) == 1} {
        place $base.service.fwdrrt.res -in $base.service.fwdrrt -anchor n -relx 0.5 -y 90
        place $base.service.fwdrrt.rrt -in $base.service.fwdrrt -anchor c -x 330 -y 150
        place $base.service.fwdrrt.entAddr -in $base.service.fwdrrt -anchor n -relx 0.5 -y 40
    }
    if {$app(h450,fwdrrt) == 2} {
        place $base.service.fwdrrt.cond -in $base.service.fwdrrt -anchor n -relx 0.5 -y 90
        place $base.service.fwdrrt.intr -in $base.service.fwdrrt -anchor c -x 330 -y 150
    }
    place $base.service.hold.act -in $base.service.hold -anchor c -x 330 -y 150
    place $base.service.hold.ret -in $base.service.hold -anchor c -x 396 -y 150
    place $base.service.hold.near -in $base.service.hold -anchor w -x 310 -y 70

    place $base.service.prkpku.selectPrk -in $base.service.prkpku -anchor w -x 20 -y 30
    place $base.service.prkpku.selectPku -in $base.service.prkpku -anchor w -x 20 -y 50
    if {$app(h450,prkpku) == 0} {
            place $base.service.prkpku.actPrk      -in $base.service.prkpku -anchor c -x 330 -y 150
            place $base.service.prkpku.enablePrk   -in $base.service.prkpku -anchor w -x 310 -y 70
            place $base.service.prkpku.alertingPrk -in $base.service.prkpku -anchor w -x 310 -y 90
            place $base.service.prkpku.insEP       -in $base.service.prkpku -anchor n -relx 0.475 -y 65
            place $base.service.prkpku.delEP       -in $base.service.prkpku -anchor n -relx 0.525 -y 65
            place $base.service.prkpku.eps         -in $base.service.prkpku -anchor n -relx 0.5 -y 90
            place $base.service.prkpku.parkingLotAddr -in $base.service.prkpku -anchor n -relx 0.5 -y 40
    }
    if {$app(h450,prkpku) == 1} {
            place $base.service.prkpku.actPku      -in $base.service.prkpku -anchor c -x 330 -y 150
            place $base.service.prkpku.calls       -in $base.service.prkpku -anchor n -relx 0.5 -y 90
    }

    place $base.service.wait.act -in $base.service.wait -anchor c -x 330 -y 150
    place $base.service.wait.disable -in $base.service.wait -anchor w -x 310 -y 70

    place $base.service.mwi.selectMes -in $base.service.mwi -anchor w -x 20 -y 30
    place $base.service.mwi.selectCB -in $base.service.mwi -anchor w -x 20 -y 50
    place $base.service.mwi.selectIntr -in $base.service.mwi -anchor w -x 20 -y 70
    if {$app(h450,indication) == 0} {
    place $tmp(h450Tab).service.mwi.message -in $tmp(h450Tab).service.mwi -anchor c -x 330 -y 150
    place $tmp(h450Tab).service.mwi.deactivateMes -in $tmp(h450Tab).service.mwi -anchor c -x 396 -y 150
    }
    if {$app(h450,indication) == 1} {
    place $tmp(h450Tab).service.mwi.callBack -in $tmp(h450Tab).service.mwi -anchor c -x 330 -y 150
    place $tmp(h450Tab).service.mwi.deactivateCB -in $tmp(h450Tab).service.mwi -anchor c -x 396 -y 150
    }
    if {$app(h450,indication) == 2} {
    place $tmp(h450Tab).service.mwi.interogate -in $tmp(h450Tab).service.mwi -anchor c -x 330 -y 150
    }
    place $base.service.mwi.sendTo -in $base.service.mwi -anchor n -relx 0.5 -y 40

    place $base.service.nameID.type -in $base.service.nameID -anchor n -relx 0.5 -y 90
    place $base.service.nameID.entName -in $base.service.nameID -anchor n -relx 0.5 -y 40

    place $base.service.comp.a1 -in $base.service.comp -anchor w -x 10 -y 30
    place $base.service.comp.a2 -in $base.service.comp -anchor w -x 10 -y 50
    place $base.service.comp.b -in $base.service.comp -anchor w -x 10 -y 70
    place $base.service.comp.cancel -in $base.service.comp -anchor c -x 396 -y 150
    place $base.service.comp.activate -in $base.service.comp -anchor c -x 330 -y 150
    place $base.service.comp.ringout -in $base.service.comp -anchor c -x 264 -y 150
    place $base.service.comp.resume -in $base.service.comp -anchor c -x 198 -y 150
    place $base.service.comp.execPossible -in $base.service.comp -anchor c -x 50 -y 150

    place $base.service.comp.entAddr -in $base.service.comp -anchor n -relx 0.5 -y 40
    place $base.service.comp.call -in $base.service.comp -anchor n -relx 0.5 -y 70
    place $base.service.comp.callLab -in $base.service.comp -anchor n -relx 0.3 -y 70
    place $base.service.comp.serv -in $base.service.comp -anchor n -relx 0.5 -y 100
    place $base.service.comp.servLab -in $base.service.comp -anchor n -relx 0.3 -y 100
    place $base.service.comp.busy -in $base.service.comp -anchor w -x 310 -y 90

    place $base.service.offer.entAddr -in $base.service.offer -anchor n -relx 0.5 -y 40
    place $base.service.offer.selectSend -in $base.service.offer -anchor w -x 20 -y 30
    place $base.service.offer.selectRet -in $base.service.offer -anchor w -x 20 -y 50
    if {$app(h450,offer)} {
        place $base.service.offer.rua -in $base.service.offer -anchor c -x 330 -y 150
        place $base.service.offer.in -in $base.service.offer -anchor n -relx 0.5 -y 90
    } else {
        place $base.service.offer.send -in $base.service.offer -anchor c -x 330 -y 150
        place $base.service.offer.out -in $base.service.offer -anchor n -relx 0.5 -y 90
    }
    place $base.service.offer.overide -in $base.service.offer -anchor w -x 310 -y 70
    place $base.service.offer.full -in $base.service.offer -anchor w -x 310 -y 90

    place $base.service.intr.selectFR -in $base.service.intr -anchor w -x 20 -y 30
    place $base.service.intr.selectIR -in $base.service.intr -anchor w -x 20 -y 50
    place $base.service.intr.selectSM -in $base.service.intr -anchor w -x 20 -y 70
    if {$app(h450,intrusion) == 0} {
        place $tmp(h450Tab).service.intr.fr -in $tmp(h450Tab).service.intr -anchor c -x 330 -y 150
    }
    if {$app(h450,intrusion) == 1} {
        place $tmp(h450Tab).service.intr.ir -in $tmp(h450Tab).service.intr -anchor c -x 330 -y 150
        place $tmp(h450Tab).service.intr.post -in $tmp(h450Tab).service.intr -anchor n -relx 0.5 -y 90
    }
    if {$app(h450,intrusion) == 2} {
        place $tmp(h450Tab).service.intr.sm -in $tmp(h450Tab).service.intr -anchor c -x 330 -y 150
        place $tmp(h450Tab).service.intr.callID -in $tmp(h450Tab).service.intr -anchor n -relx 0.55 -y 90
        place $tmp(h450Tab).service.intr.getCallID -in $tmp(h450Tab).service.intr -anchor n -relx 0.45 -y 90
    }
    place $base.service.intr.sendTo -in $base.service.intr -anchor n -relx 0.5 -y 40
    place $base.service.intr.pos -in $base.service.intr -anchor w -x 310 -y 70
    place $base.service.intr.iso -in $base.service.intr -anchor w -x 310 -y 90
    place $base.service.intr.use -in $base.service.intr -anchor w -x 310 -y 110
    place $base.service.intr.callHandle -in $base.service.intr -anchor c -x 30 -y 150

    if { $app(h450,action) != "" } {
        grid $app(h450,action) -in $base.service -row 1 -column 0 -sticky news }

    ########
    # BIND #
    ########
    placeHeader $base.reply "Reply"
    placeHeader $base.service "Supplementary Services"

    $base.service.comp.serv.01 add radiobutton -label <None> -value <None> -variable tmp(h450,compFoundService)
    h4509SetSide $app(h450,completion)
}

proc h450replyButtons {base} {
    global tmp
    set base "$base.reply"

    if {$tmp(h450,ask)} {
        $base.conf config -state disable
        $base.rejc config -state disable
    } else {
        $base.conf config -state normal
        $base.rejc config -state normal
    }
}

proc h4505DelGropupInd {callID} {
    global app tmp
    set callMenu $tmp(h450Tab).service.prkpku.calls.01

    if { [$callMenu index end] == "none" } return
    $callMenu activate "none"
    set tmp(h450,callPku) "Pick Call"

    for {set i 0} {$i <= [$callMenu index end]} {incr i} {
        set callEnt [$callMenu entrycget $i -value]
        if { [string match "* $callID *" $callEnt] } {
            $callMenu delete $i
            return
        }
    }
}

proc h4509SetSide {side} {
    global app tmp
    set base "$tmp(h450Tab).service.comp"

    # 0 - User A Busy
    # 1 - User A NoResponse
    # 2 - User B
    switch $side {
        0 {
            $base.activate config -state normal
            $base.resume config -state normal
            $base.ringout config -state normal
            $base.execPossible config -state disable
            place $base.canRetainServ -in $base -anchor w -x 310 -y 70
            place forget $base.retainServ
            place forget $base.retainConn
        }
        1 {
            $base.activate config -state normal
            $base.resume config -state normal
            $base.ringout config -state normal
            $base.execPossible config -state disable
            place $base.canRetainServ -in $base -anchor w -x 310 -y 70
            place forget $base.retainServ
            place forget $base.retainConn
        }
        2 {
            $base.activate config -state disable
            $base.resume config -state disable
            $base.ringout config -state disable
            $base.execPossible config -state normal
            place forget $base.canRetainServ
            place $base.retainServ -in $base -anchor w -x 310 -y 70
            place $base.retainConn -in $base -anchor w -x 310 -y 50
        }
    }
}

proc h4509AddService {service} {
    global tmp
    set w $tmp(h450Tab).service.comp
    $w.call.01 add radiobutton -label $service -value [lindex $service 1] -variable tmp(h450,compService)
    $w.serv.01 add radiobutton -label $service -value [lindex $service 1] -variable tmp(h450,compFoundService)
}

proc h4509RemoveService {service} {
    global tmp
    set wFrame $tmp(h450Tab).service.comp
    set w $wFrame.call.01

    if {$tmp(h450,compFoundService) == $service} {set tmp(h450,compFoundService) <None>}
    if {$tmp(h450,compService) == $service} {set tmp(h450,compService) <None>}

    for {set i 0} {1} {incr i} {
        if {[$w entrycget $i -value] == $service} {
            $w delete $i
            $wFrame.serv.01 delete [expr $i+1]
            break
        }
    }
}

proc callSelect:updt { menuWidget varName selectCommand } {
    global app tmp

    eval "set curSel $$varName"

    $menuWidget delete 0 end

    foreach call [.test.calls.list get 0 end] {
        $menuWidget add radiobutton -indicatoron 0 -value $call -variable $varName -label $call \
            -command $selectCommand
    }

    set index [lsearch -exact [.test.calls.list get 0 end] $curSel]

    if { $index != -1 } {
        set $varName $curSel
    } else {
        set $varName "Select Call"
    }
}

proc getConnectedCall { } {
    # Get the list and find a call whose state is connected
    set i [lsearch -glob [.test.calls.list get 0 end] "* Connected*"]
    return [.test.calls.list get $i]
}

proc getNextConnectedCall { lastCall } {
    # Get the list and find a call whose state is connected
    set i [lsearch [.test.calls.list get 0 end] $lastCall]
    incr i
    set i [lsearch -glob [.test.calls.list get $i end] "* Connected*"]
    return [.test.calls.list get $i]
}

proc findCallID { callID } {
    foreach call [.test.calls.list get 0 end] {
        if {[string equal [H450.getCallID "$call"] $callID]} {
            return "$call"
        }
    }
    return "call not found"
}

proc findIncomingCallID { callID } {
    foreach call [.test.calls.list get 0 end] {
        if {[string match "*(in)*" $call]} {
            if {[string equal [H450.getCallID "$call"] $callID]} {
                return "$call"
            }
        }
    }
    return "call not found"
}

proc findOutgoingCallID { callID } {
    foreach call [.test.calls.list get 0 end] {
        if {[string match "*(out)*" $call]} {
            if {[string equal [H450.getCallID "$call"] $callID]} {
                return "$call"
            }
        }
    }
    return "call not found"
}

