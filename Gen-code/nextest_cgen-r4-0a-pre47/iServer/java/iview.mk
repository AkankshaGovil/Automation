#
# this file contains all the defines related to building iView
#
IVIEW_UTIL_CLASSES =	com/nextone/util/ActionableTimer.class \
			com/nextone/util/AutoRefreshPanel.class \
			com/nextone/util/AutoScrollMenu.class \
                        com/nextone/util/BottomLabel.class \
                        com/nextone/util/CloneableDataInputStream.class \
                        com/nextone/util/DateChooser.class \
                        com/nextone/util/DateListener.class \
			com/nextone/util/DraggableListUI.class \
                        com/nextone/util/DraggableTree.class \
                        com/nextone/util/ExtensionFileFilter.class \
                        com/nextone/util/FileSaver.class \
			com/nextone/util/HTMLDisplayWindow.class \
			com/nextone/util/HyperLinkLabel.class \
			com/nextone/util/HyperLinkLabelListener.class \
                        com/nextone/util/IconFileView.class \
                        com/nextone/util/IPAddress.class \
                        com/nextone/util/AbstractMap.class\
                        com/nextone/util/HashMap.class\
                        com/nextone/util/LinkedHashMap.class\
			com/nextone/util/IPMask.class \
                        com/nextone/util/IPUtil.class \
                        com/nextone/util/LimitedDataInputStream.class \
                        com/nextone/util/LimitExceededException.class \
                        com/nextone/util/Line.class \
                        com/nextone/util/Logger.class \
                        com/nextone/util/NextoneMulticastSocket.class \
                        com/nextone/util/NextoneThreadGroup.class \
			com/nextone/util/NumberSpinner.class \
                        com/nextone/util/PopMessage.class \
                        com/nextone/util/ProgressStatus.class \
						com/nextone/util/ProgressDialog.class \
						com/nextone/util/StatusAnimation.class \
						com/nextone/util/FileAnimation.class \
						com/nextone/util/BookAnimation.class \
						com/nextone/util/ProgressAnimation.class \
                        com/nextone/util/QueueProcessor.class \
			com/nextone/util/RandomAccessObjectFile.class \
			com/nextone/util/RemoveListener.class \
                        com/nextone/util/SaveFileListener.class \
                        com/nextone/util/ScrollableDesktopPane.class \
                        com/nextone/util/SortableListModel.class \
                        com/nextone/util/SortableMutableTreeNode.class \
			com/nextone/util/SpinEvent.class \
			com/nextone/util/SpinListener.class \
			com/nextone/util/Spinner.class \
                        com/nextone/util/SplashScreen.class \
			com/nextone/util/StringArrayTransferable.class \
                        com/nextone/util/SysUtil.class \
                        com/nextone/util/TableMap.class \
                        com/nextone/util/TableSorter.class \
			com/nextone/util/TM.class \
                        com/nextone/util/TransferableTreeNode.class \
                        com/nextone/util/TreeRowExpander.class \
                        com/nextone/util/WeakEncryptionInputStream.class \
                        com/nextone/util/WeakEncryptionOutputStream.class \
			com/nextone/util/WindowsAltFileSystemView.class \
                        com/nextone/util/UDPServer.class \
                        com/nextone/util/UDPServerDoWork.class \
                        com/nextone/util/UDPServerWorker.class \
                        com/nextone/util/JMask/IPAddressField.class \
                        com/nextone/util/JMask/NumberField.class \
                        com/nextone/util/JMask/PhoneNumber.class \
                        com/nextone/util/JMask/RestrictivePasswordField.class \
                        com/nextone/util/JMask/RestrictiveTextField.class 


IVIEW_COMMON_CLASSES =  com/nextone/common/AnimPanel.class \
			com/nextone/common/BoardData.class\
                        com/nextone/common/Bridge.class \
                        com/nextone/common/BridgeClientImpl.class \
                        com/nextone/common/BridgeException.class \
                        com/nextone/common/CallPlan.class \
                        com/nextone/common/CallPlanBinding.class \
                        com/nextone/common/CallPlanRoute.class \
			com/nextone/common/Commands.class \
                        com/nextone/common/CommonConstants.class \
                        com/nextone/common/CommonFunctions.class \
                        com/nextone/common/ConfigFile.class \
                        com/nextone/common/ConfigFile500Impl.class \
			com/nextone/common/ConfigFile1000Impl.class \
                        com/nextone/common/ConfigProvider.class \
                        com/nextone/common/ConfigRetriever.class \
                        com/nextone/common/ConsumeMouseFrame.class \
                        com/nextone/common/ConsumeMouseGlassPane.class \
                        com/nextone/common/CursorChanger.class \
			com/nextone/common/DataConsumer.class \
                        com/nextone/common/DTServer.class \
                        com/nextone/common/DropStatusSource.class \
			com/nextone/common/ExistException.class \
                        com/nextone/common/FileVersion.class \
                        com/nextone/common/GetListData.class \
                        com/nextone/common/Hello.class \
			com/nextone/common/i1000ConfigData.class \
			com/nextone/common/iEdge1000Constants.class \
                        com/nextone/common/IEdgeCore.class \
                        com/nextone/common/IEdgeList.class \
			com/nextone/common/InterruptException.class\
			com/nextone/common/iServerConfig.class \
                        com/nextone/common/LabelCellRenderer.class \
                        com/nextone/common/LimitedCommandComm.class \
                        com/nextone/common/MaintenanceRequest.class \
                        com/nextone/common/MaintenanceGroup.class \
			com/nextone/common/NetworkList.class \
                        com/nextone/common/NexToneFrame.class \
                        com/nextone/common/NexToneInternalFrame.class \
                        com/nextone/common/NexTonePropertiesPane.class \
                        com/nextone/common/NotProvisionedBridgeException.class \
                        com/nextone/common/OperationUnknownBridgeException.class \
                        com/nextone/common/PermissionProvider.class \
                        com/nextone/common/PingTCP.class \
                        com/nextone/common/ProvisionData.class \
			com/nextone/common/RouteData.class \
                        com/nextone/common/ReceiveReg.class \
			com/nextone/common/RedundInfo.class \
                        com/nextone/common/RegidPortPair.class \
                        com/nextone/common/SendMcast.class \
                        com/nextone/common/SingleInstance.class \
                        com/nextone/common/ShowSoftwareDownloadStatus.class \
			com/nextone/common/Task.class\
                        com/nextone/common/TimeoutProvider.class \
                        com/nextone/common/VersionMismatchBridgeException.class \
                        com/nextone/common/VpnGroupList.class \
                        com/nextone/common/VpnList.class

IVIEW_DHCP_SERVER_CLASSES = com/nextone/JUCon/DhcpServer/DhcpDataDialog.class \
                com/nextone/JUCon/DhcpServer/DhcpLease.class \
                com/nextone/JUCon/DhcpServer/DhcpRange.class \
                com/nextone/JUCon/DhcpServer/DhcpRangeDialog.class \
                com/nextone/JUCon/DhcpServer/DhcpServerData.class \
                com/nextone/JUCon/DhcpServer/DhcpServerConfig.class \
                com/nextone/JUCon/DhcpServer/DhcpSubnet.class \
                com/nextone/JUCon/DhcpServer/DhcpSubnetDialog.class \
                com/nextone/JUCon/DhcpServer/DhcpTreeEntry.class

IVIEW_DHCP_SERVER_1000_CLASSES = com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpDataDialog.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpLease.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpRange.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpRangeDialog.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpServerData.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpServerConfig1000.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpSubnet.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpSubnetDialog.class \
                com/nextone/JUCon/iEdge1000/Dhcpserver/DhcpTreeEntry.class

IVIEW_IEDGE500_CLASSES = com/nextone/JUCon/iEdge500/AccessControl.class \
                com/nextone/JUCon/iEdge500/CfgPanel.class \
                com/nextone/JUCon/iEdge500/DataVpnConfig.class \
                com/nextone/JUCon/iEdge500/DnsConfig.class \
                com/nextone/JUCon/iEdge500/EthernetConfig.class \
                com/nextone/JUCon/iEdge500/H323Config.class \
                com/nextone/JUCon/iEdge500/I500.class \
                com/nextone/JUCon/iEdge500/InternetAccessConfig.class \
                com/nextone/JUCon/iEdge500/IPFilterConfig.class \
                com/nextone/JUCon/iEdge500/ITSPConfig.class \
                com/nextone/JUCon/iEdge500/IVRConfig.class \
                com/nextone/JUCon/iEdge500/JitterConfig.class \
                com/nextone/JUCon/iEdge500/LinePortConfig.class \
                com/nextone/JUCon/iEdge500/NatConfig.class \
                com/nextone/JUCon/iEdge500/OfflineI500.class \
                com/nextone/JUCon/iEdge500/PhoneConfig.class \
                com/nextone/JUCon/iEdge500/PhonePortConfig.class \
                com/nextone/JUCon/iEdge500/PrefixConfig.class \
                com/nextone/JUCon/iEdge500/ServerConfig.class \
                com/nextone/JUCon/iEdge500/SoftwareDownload.class \
                com/nextone/JUCon/iEdge500/TabsPanel.class

IVIEW_PINGTEL_CLASSES = com/nextone/JUCon/PingTel/PingTel.class \
                com/nextone/JUCon/PingTel/ConfigTab.class \
                com/nextone/JUCon/PingTel/PingTelTabsPanel.class

IVIEW_VEGASTREAM_CLASSES = com/nextone/JUCon/VegaStream/VegaStream.class \
                com/nextone/JUCon/VegaStream/ConfigTab.class \
                com/nextone/JUCon/VegaStream/VegaStreamTabsPanel.class

IVIEW_IEDGE1000_CLASSES = com/nextone/JUCon/iEdge1000/AbstractCasSpanDialog.class \
                com/nextone/JUCon/iEdge1000/AbstractSpanDialog.class \
                com/nextone/JUCon/iEdge1000/AbstractPriSpanDialog.class \
                com/nextone/JUCon/iEdge1000/AnalogCard.class \
                com/nextone/JUCon/iEdge1000/Card.class \
                com/nextone/JUCon/iEdge1000/CasSpan.class \
                com/nextone/JUCon/iEdge1000/ConfigTab.class \
                com/nextone/JUCon/iEdge1000/CountryCode.class \
                com/nextone/JUCon/iEdge1000/DigitalCard.class \
                com/nextone/JUCon/iEdge1000/DigitalE1Card.class \
                com/nextone/JUCon/iEdge1000/DigitalT1Card.class \
                com/nextone/JUCon/iEdge1000/DigitalCardDialog.class \
                com/nextone/JUCon/iEdge1000/DnsConfigTab.class \
                com/nextone/JUCon/iEdge1000/DTMFIncallDuration.class \
                com/nextone/JUCon/iEdge1000/E1PriSpan.class \
                com/nextone/JUCon/iEdge1000/E1PriSpanDialog.class \
                com/nextone/JUCon/iEdge1000/FramingMode.class \
                com/nextone/JUCon/iEdge1000/H323ConfigTab.class \
                com/nextone/JUCon/iEdge1000/InvalidCountryCodeException.class \
                com/nextone/JUCon/iEdge1000/I1000.class \
                com/nextone/JUCon/iEdge1000/i1000TabsPanel.class \
                com/nextone/JUCon/iEdge1000/IServerConfigTab.class \
                com/nextone/JUCon/iEdge1000/LineCode.class \
                com/nextone/JUCon/iEdge1000/LineLength.class \
                com/nextone/JUCon/iEdge1000/LineTermination.class \
                com/nextone/JUCon/iEdge1000/LoopbackMode.class \
                com/nextone/JUCon/iEdge1000/PhoneConfig.class \
                com/nextone/JUCon/iEdge1000/OfflineI1000.class \
                com/nextone/JUCon/iEdge1000/NatConfig1000.class \
                com/nextone/JUCon/iEdge1000/IPFilterConfig1000.class \
                com/nextone/JUCon/iEdge1000/Port.class \
                com/nextone/JUCon/iEdge1000/OutgoingPortAllocStrategy.class \
                com/nextone/JUCon/iEdge1000/PortConfigData.class \
                com/nextone/JUCon/iEdge1000/PortConfigTab.class \
                com/nextone/JUCon/iEdge1000/PortConfigTreeEntry.class \
                com/nextone/JUCon/iEdge1000/PortConfigTreeCellRenderer.class \
                com/nextone/JUCon/iEdge1000/PrefixConfigTab.class \
                com/nextone/JUCon/iEdge1000/PriSpan.class \
                com/nextone/JUCon/iEdge1000/Q931Variant.class \
                com/nextone/JUCon/iEdge1000/SignalingMode.class \
                com/nextone/JUCon/iEdge1000/SoftwareDownloadTab.class \
                com/nextone/JUCon/iEdge1000/Span.class \
                com/nextone/JUCon/iEdge1000/SpanAutoConfigDialog.class \
                com/nextone/JUCon/iEdge1000/SpanMode.class \
                com/nextone/JUCon/iEdge1000/SpanTypeDialog.class \
                com/nextone/JUCon/iEdge1000/SwitchType.class \
                com/nextone/JUCon/iEdge1000/T1PriSpan.class \
                com/nextone/JUCon/iEdge1000/T1PriSpanDialog.class \
                com/nextone/JUCon/iEdge1000/T1CasSpanDialog.class \
                com/nextone/JUCon/iEdge1000/SipConfig.class

IVIEW_ISERVER_CLASSES = com/nextone/JUCon/iServer/AbstractShowLogFile.class \
		com/nextone/JUCon/iServer/AddressInfo.class \
                com/nextone/JUCon/iServer/CallingPlan.class \
                com/nextone/JUCon/iServer/CallingPlanTree.class \
		com/nextone/JUCon/iServer/CallRoute.class \
                com/nextone/JUCon/iServer/CallingRouteList.class \
                com/nextone/JUCon/iServer/CallPlanBindingDialog.class \
                com/nextone/JUCon/iServer/CallingPlanDialog.class \
                com/nextone/JUCon/iServer/ChoosePorts.class \
		com/nextone/JUCon/iServer/cfg/AdvancedConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/AravoxConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/BillingConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/CfgDialog.class \
		com/nextone/JUCon/iServer/cfg/ConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/FceConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/H323ConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/IpFilterConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/LoggingConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/NetworkListConfigDialog.class \
		com/nextone/JUCon/iServer/cfg/SipConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/SystemConfigPanel.class \
		com/nextone/JUCon/iServer/cfg/RedundsConfigPanel.class \
                com/nextone/JUCon/iServer/ClusteredIServer.class \
		com/nextone/JUCon/iServer/ClusterInfo.class \
                com/nextone/JUCon/iServer/IEdge.class \
                com/nextone/JUCon/iServer/IEdgeView.class \
                com/nextone/JUCon/iServer/IServer.class \
                com/nextone/JUCon/iServer/Maint/ActionPanel.class \
                com/nextone/JUCon/iServer/Maint/AutoDownloadConfig.class \
                com/nextone/JUCon/iServer/Maint/GroupPanel.class \
                com/nextone/JUCon/iServer/Maint/ListAutoDownloadConfig.class \
                com/nextone/JUCon/iServer/Maint/ListMaintenanceGroups.class \
                com/nextone/JUCon/iServer/Maint/ListMaintenanceLogs.class \
                com/nextone/JUCon/iServer/Maint/ListMaintenanceRequests.class \
                com/nextone/JUCon/iServer/Maint/LogFileInputStream.class \
                com/nextone/JUCon/iServer/Maint/MaintenanceGroupFrame.class \
                com/nextone/JUCon/iServer/Maint/MaintenanceListCellRenderer.class \
                com/nextone/JUCon/iServer/Maint/MaintenanceRequestFrame.class \
                com/nextone/JUCon/iServer/Maint/PanelProvider.class \
                com/nextone/JUCon/iServer/Maint/ServerPanel.class \
		com/nextone/JUCon/iServer/PortUsageGraphDataProvider.class \
                com/nextone/JUCon/iServer/QueryPanel.class \
                com/nextone/JUCon/iServer/RouteWizard.class \
                com/nextone/JUCon/iServer/ServerTabsPanel.class \
                com/nextone/JUCon/iServer/ShowAutoDownloadLogFile.class \
		com/nextone/JUCon/iServer/ShowDebugLogFileInternalFrame.class \
                com/nextone/JUCon/iServer/ShowIServerStatus.class \
                com/nextone/JUCon/iServer/ShowLogFile.class \
                com/nextone/JUCon/iServer/StaticClusteredIServer.class

IVIEW_CLASSES = com/nextone/JUCon/AbstractUserPropertyData.class \
                com/nextone/JUCon/AbstractTabsPanel.class \
                com/nextone/JUCon/AddRemote.class \
                com/nextone/JUCon/AddRemoteI500.class \
		com/nextone/JUCon/AddRemoteIServer.class \
                com/nextone/JUCon/AddUserDialog.class \
                com/nextone/JUCon/AlarmDialog.class \
                com/nextone/JUCon/BoxPanel.class \
		com/nextone/JUCon/CallGraph.class \
                com/nextone/JUCon/CleanupDevices.class \
                com/nextone/JUCon/CommandComm.class \
                com/nextone/JUCon/Constants.class \
                com/nextone/JUCon/DBImport.class \
                com/nextone/JUCon/DBExport.class \
                com/nextone/JUCon/DeviceLayout.class \
                com/nextone/JUCon/DevicePermissions.class \
                com/nextone/JUCon/DocumentValueChangeListener.class \
                com/nextone/JUCon/EdgeDevice.class \
                com/nextone/JUCon/ExecuteFile.class \
		com/nextone/JUCon/GraphDataProvider.class \
                com/nextone/JUCon/ItemValueChangeListener.class \
                com/nextone/JUCon/JUCon.class \
                com/nextone/JUCon/JUConMain.class \
                com/nextone/JUCon/JUConThreadGroup.class \
                com/nextone/JUCon/JUConFile.class\
                com/nextone/JUCon/Menus.class \
                com/nextone/JUCon/NextoneDevice.class \
                com/nextone/JUCon/OfflineConfig.class \
                com/nextone/JUCon/OfflineEdge.class \
                com/nextone/JUCon/PartialConfig.class \
                com/nextone/JUCon/PasswordChangeDialog.class \
                com/nextone/JUCon/PasswordFile.class \
                com/nextone/JUCon/PasswordFileException.class \
                com/nextone/JUCon/PasswordEntry.class \
                com/nextone/JUCon/Permissions.class \
                com/nextone/JUCon/PhoneInput.class \
		com/nextone/JUCon/VpnInput.class\
                com/nextone/JUCon/PollInterval.class \
                com/nextone/JUCon/PropertyPanelDialog.class \
                com/nextone/JUCon/SaveConfig.class \
                com/nextone/JUCon/SaveFactory.class \
                com/nextone/JUCon/Saveable.class \
                com/nextone/JUCon/SaveToFile.class \
                com/nextone/JUCon/SetIPAddress.class \
		com/nextone/JUCon/ShowDebugLogFile.class \
                com/nextone/JUCon/ThirdPartyDevice.class \
                com/nextone/JUCon/Timeouts.class \
                com/nextone/JUCon/UnknownUserObjectException.class \
                com/nextone/JUCon/Updateable.class \
                com/nextone/JUCon/UserAdmin.class \
                com/nextone/JUCon/UserLoginDialog.class \
                com/nextone/JUCon/UserObjectFactory.class \
                com/nextone/JUCon/UserPermissions.class \
                com/nextone/JUCon/UserPermissionsPanel.class \
                com/nextone/JUCon/UserProperties.class \
                com/nextone/JUCon/UserPropertyData.class \
                com/nextone/JUCon/ValueChangeListener.class \
                com/nextone/JUCon/ValueChanger.class \
		com/nextone/JUCon/VPortGraphDataProvider.class \
		com/nextone/JUCon/VPortLogger.class

IVIEW_IMAGES =  com/nextone/images/ArrowDown.gif \
                com/nextone/images/ArrowUp.gif \
                com/nextone/JUCon/auth.gif \
                com/nextone/images/BarGraph.gif \
                com/nextone/images/bigiconimg.jpg \
		com/nextone/images/blank-20.gif \
                com/nextone/JUCon/box.gif \
                com/nextone/JUCon/box2.gif \
                com/nextone/JUCon/box3.gif \
                com/nextone/JUCon/box4.gif \
                com/nextone/JUCon/Bulb.gif \
		com/nextone/images/Caution.gif \
                com/nextone/images/cell.gif \
                com/nextone/JUCon/Close.gif \
                com/nextone/images/Computer.gif \
		com/nextone/images/Data.gif \
                com/nextone/images/DataStore.gif \
                com/nextone/images/Document.gif \
                com/nextone/images/DocumentDraw.gif \
                com/nextone/images/DocumentOut.gif \
                com/nextone/images/fax.gif \
                com/nextone/images/FlowGraph.gif \
                com/nextone/images/g.gif \
		com/nextone/images/enumserver.gif \
		com/nextone/images/genip.gif \
                com/nextone/images/header.gif \
                com/nextone/JUCon/Help.gif \
                com/nextone/images/i.gif \
                com/nextone/images/i1000.gif \
                com/nextone/images/i1000T1AllActive.gif \
                com/nextone/images/i1000T1SomeActiveRed.gif \
                com/nextone/images/i1000T1SomeActiveYellow.gif \
                com/nextone/images/i1000T1SomeActiveBlue.gif \
                com/nextone/images/i1000T1SomeActiveBrown.gif \
                com/nextone/images/i1000T1InactiveRed.gif \
                com/nextone/images/i1000T1InactiveYellow.gif \
                com/nextone/images/i1000T1InactiveBlue.gif \
                com/nextone/images/i1000T1InactiveBrown.gif \
                com/nextone/images/i1000small.gif \
                com/nextone/images/iconimg.jpg \
                com/nextone/images/imob.gif \
                com/nextone/images/iserver.gif \
                com/nextone/images/iserverb.gif \
				com/nextone/images/iserverc_nd.gif \
                com/nextone/images/iserverc_d.gif \
                com/nextone/images/iserverc_dm.gif \
				com/nextone/images/iserverc_n.gif \
				com/nextone/images/iserverc_b.gif \
                com/nextone/images/iservers.gif \
                com/nextone/images/is.gif \
                com/nextone/images/ix.gif \
                com/nextone/images/LineGraph.gif \
                com/nextone/images/LeftArrow.gif \
                com/nextone/JUCon/Minus.gif \
                com/nextone/JUCon/New.gif \
                com/nextone/JUCon/Open.gif \
                com/nextone/images/Pin.gif \
                com/nextone/images/PingTelIcon.gif \
                com/nextone/images/Plug.gif \
                com/nextone/JUCon/Plus.gif \
                com/nextone/images/RightArrow.gif \
                com/nextone/JUCon/Save.gif \
                com/nextone/images/splash.gif \
		com/nextone/images/user.gif \
                com/nextone/images/v.gif \
                com/nextone/images/VegaStreamIcon.gif \
                com/nextone/images/JWizard.gif \
                com/nextone/images/WestArrow.gif \
                com/nextone/images/EastArrow.gif \
                com/nextone/JUCon/warn.gif

IVIEW_WIZARD_CLASSES =	com/nextone/wizard/AbstractLayout.class \
						com/nextone/wizard/DataCollectionModel.class \
						com/nextone/wizard/DeckLayout.class \
						com/nextone/wizard/DeckPanel.class \
						com/nextone/wizard/EdgeBorder.class \
						com/nextone/wizard/JWizard.class \
						com/nextone/wizard/PropertyDataModel.class \
						com/nextone/wizard/SequenceManager.class \
						com/nextone/wizard/UnitLayout.class \
						com/nextone/wizard/WizardImage.class \
						com/nextone/wizard/WizardPanel.class \
						com/nextone/wizard/WizardSequenceManager.class \
						com/nextone/wizard/WizardValidator.class \
						com/nextone/wizard/WizardNavigator.class \
						com/nextone/wizard/DataRequester.class 

IVIEW_DATABASE_CLASSES =com/nextone/database/Database.class\
						com/nextone/database/DbStatement.class\
						com/nextone/database/DbTable.class\
						com/nextone/database/ServerDB.class\
						com/nextone/database/XMLParser.class\
						com/nextone/database/ObjectFile.class \
						com/nextone/database/ProcessQuery.class \
						com/nextone/database/DBKeys.class \
						com/nextone/database/ResultSet.class 


IVIEW_SOUNDS =	com/nextone/util/chord.wav \
		com/nextone/util/ding.wav \
		com/nextone/util/tada.wav \
		ANIUsage.html \
		Resyntax.html
