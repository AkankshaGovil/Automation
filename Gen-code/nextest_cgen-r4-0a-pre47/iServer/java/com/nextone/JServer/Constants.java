package com.nextone.JServer;

import com.nextone.common.CommonConstants;

interface Constants extends CommonConstants {
  // communication timeouts
  public static final int GET_TIMEOUT = 3;
  public static final int SET_TIMEOUT = 20;

  public static final int MCAST_JSERVER_LISTEN_PORT = CommonConstants.MCAST_SEND_PORT;

  public static final int JSERVER_IEDGELIST_SERVER_PORT = 10106;
  public static final int JSERVER_CALLINGPLAN_SERVER_PORT = 10107;
  public static final int JSERVER_LOGFILE_SERVER_PORT = 10108;
  public static final int JSERVER_MISC_SERVER_PORT = 10109;

  public static final String dataDirName = "jserver-data";
  public static final String groupDirName = "groups";
  public static final String requestDirName = "requests";
  public static final String logDirName = "logs";
  public static final String autoDlDirName = "auto-download";

  //  dir where the db files are stored
  public static final String DB_DIR = "iserver_db";
  public static final String DB_DEFAULT_NAME = "iserver_db_bkup";
  public static final String [] dbFileExts = {"attrs.gdbm",
					      "cplan.gdbm",
					      "cbind.gdbm",
					      "croute.gdbm",
					      "iedge.gdbm",
					      "vpng.gdbm",
					      "vpns.gdbm",
					      "igrp.gdbm",
					      "realm.gdbm",
					      "vnet.gdbm",
					      "trigger.gdbm"
  };
  
  public static final String SYSLOG_CONF_FILE = "/etc/syslog.conf";
  public static final String DEBUG_LOG_SYSLOG_CLASS = "local1";
  public static final String CDR_LOG_SYSLOG_CLASS = "local2";
  public static final String H323_LOG_SYSLOG_CLASS = "local3";
  public static final String SYSLOG_ERROR_CLASS_SUFFIX = "err";
  public static final String SYSLOG_DEBUG_CLASS_SUFFIX = "debug";
  public static final String SYSLOGD_STOP_COMMAND = "/etc/init.d/syslog stop";
  public static final String SYSLOGD_START_COMMAND = "/etc/init.d/syslog start";
	public static final String HOTKNIFE_DETECT_COMMAND = "prtconf -v";
	public static final String HOTKNIFE_GREP_STRING = "pci1331,30";
	public static final String HOTKNIFE_DETECT_COMMAND_LINUX = "lspci -v";
	public static final String HOTKNIFE_GREP_STRING_LINUX = "Radisys";
	
  public static final String CDR_LOG_FILE   = "/var/log/DATA.CDR";
  public static final String CDR_FILE_NAME  = "DATA.CDR";

  // delay to wait before expecting GIS to be up (in milliseconds)
  public static final int GIS_UPTIME_DELAY  = 60000;

  // delay after which imported db files will be deleted (in msecs)
  public static final int DB_IMPORT_FILE_DELETE_TIME = 600000;
}
