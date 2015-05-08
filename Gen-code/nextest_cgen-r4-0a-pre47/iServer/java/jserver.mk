#
# this file contains all the defines related to building jServer
#
JSERVER_UTIL_CLASSES = 	com/nextone/util/AbstractMap.class \
			com/nextone/util/DelayedQueueProcessor.class \
			com/nextone/util/DeltaTime.class \
                        com/nextone/util/HashMap.class \
                        com/nextone/util/IPMask.class \
                        com/nextone/util/IPUtil.class \
                        com/nextone/util/LimitedDataInputStream.class \
                        com/nextone/util/LimitExceededException.class \
                        com/nextone/util/LinkedHashMap.class \
                        com/nextone/util/NextoneMulticastSocket.class \
                        com/nextone/util/NextoneThreadGroup.class \
                        com/nextone/util/QueueProcessor.class \
                        com/nextone/util/SysUtil.class \
                        com/nextone/util/TCPServer.class \
                        com/nextone/util/TCPServerDoWork.class \
                        com/nextone/util/TCPServerWorker.class \
			com/nextone/util/TM.class \
                        com/nextone/util/UDPServer.class \
                        com/nextone/util/UDPServerDoWork.class \
                        com/nextone/util/UDPServerWorker.class \
                        com/nextone/util/WeakEncryptionInputStream.class \
                        com/nextone/util/WeakEncryptionOutputStream.class



#                        com/nextone/common/ConfigFile.class \
#			com/nextone/common/iEdge1000Constants.class \
#			com/nextone/common/i1000ConfigData.class \
#			com/nextone/common/ConfigFile500Impl.class \
#			com/nextone/common/ConfigFile1000Impl.class \
#                        com/nextone/common/ConfigProvider.class \
#			com/nextone/common/ConfigRetriever.class \
#                        com/nextone/common/DTServer.class \
#                        com/nextone/common/FileVersion.class \

JSERVER_COMMON_CLASSES= com/nextone/common/Bridge.class \
			com/nextone/common/BridgeClientImpl.class \
                        com/nextone/common/BridgeException.class \
                        com/nextone/common/CallPlan.class \
                        com/nextone/common/CallPlanBinding.class \
                        com/nextone/common/CallPlanRoute.class \
			com/nextone/common/Capabilities.class \
			com/nextone/common/Commands.class \
                        com/nextone/common/CommonConstants.class \
                        com/nextone/common/CommonFunctions.class \
                        com/nextone/common/ConsumeMouseFrame.class \
                        com/nextone/common/ConsumeMouseGlassPane.class \
                        com/nextone/common/CursorChanger.class \
                        com/nextone/common/DBObject.class \
			com/nextone/common/DataProvider.class \
			com/nextone/common/DataConsumer.class \
                        com/nextone/common/DropStatusSource.class \
			com/nextone/common/ExistException.class \
                        com/nextone/common/GetListData.class \
                        com/nextone/common/Hello.class \
                        com/nextone/common/IEdgeCore.class \
                        com/nextone/common/IEdgeGroup.class \
                        com/nextone/common/IEdgeList.class \
			com/nextone/common/iServerConfig.class \
                        com/nextone/common/LimitedCommandComm.class \
			com/nextone/common/MaintenanceGroup.class \
			com/nextone/common/MaintenanceRequest.class \
			com/nextone/common/NetworkList.class \
			com/nextone/common/NexToneFrame.class \
			com/nextone/common/NoEntryException.class \
                        com/nextone/common/NotProvisionedBridgeException.class \
                        com/nextone/common/OperationUnknownBridgeException.class \
                        com/nextone/common/PermissionProvider.class \
                        com/nextone/common/ProvisionData.class \
			com/nextone/common/RedundInfo.class \
                        com/nextone/common/RegidPortPair.class \
                        com/nextone/common/Registration.class \
			com/nextone/common/RouteData.class \
                        com/nextone/common/SingleInstance.class \
                        com/nextone/common/TimeoutProvider.class \
                        com/nextone/common/VpnGroupList.class \
                        com/nextone/common/VpnList.class

# com/nextone/JServer/ActionState.class \
#                  com/nextone/JServer/AutoDownloadDataCache.class \
#                  com/nextone/JServer/AutoDownload.class \
#                  com/nextone/JServer/AutoDownloadTask.class \
#                  com/nextone/JServer/DownloadServer.class \
#                  com/nextone/JServer/DownloadTask.class \
#                  com/nextone/JServer/StopDownloadTask.class

JSERVER_CLASSES = com/nextone/JServer/BridgeServer.class \
                  com/nextone/JServer/Constants.class \
                  com/nextone/JServer/DataCache.class \
                  com/nextone/JServer/JServer.class \
                  com/nextone/JServer/JServerMain.class \
                  com/nextone/JServer/JServerTCP.class \
                  com/nextone/JServer/LogServer.class \
                  com/nextone/JServer/LogTask.class \
		  com/nextone/JServer/MiscServer.class \
                  com/nextone/JServer/ProcessManagerClient.class \
                  com/nextone/JServer/SendListItem.class


#JSERVER_DB_CLASSES = com/nextone/database/DBKeys.class 
JSERVER_DB_CLASSES =
