package com.nextone.util;

import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.StringTokenizer;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Category;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.Priority;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.RollingFileAppender;
import org.apache.log4j.net.SyslogAppender;

/**
 * Wrapper class for the log4j log facility.
 */
public class Logger {
  /**
   * Make sure the Logger is initially created.
   */
  static {
    new Logger();
  }

  /**
   * Single instance of the Category for the logger.
   */
  private static Category logger;

  /**
   * Path of the jar file.
   */
  private static String theJarFilePath = "";

  /**
   * Default properies file name
   */
  public static final String PROPS = "log4j.properties";

  /**
   * Default log file name
   */
  public static final String DEFAULT_LOG_FILE = "iView.log";

  /**
   * Create a logger using the default properties file.
   */
  public Logger() {
    this(PROPS);
  }

  /**
   * Create a logger with the given properties. Creates a
   * running file appender.
   */
  public Logger(String props) {
    logger = Category.getInstance(Logger.class.getName());
    theJarFilePath = getJarDirectory();
    
    // Debug configuration. If the properties files exist
    // use it, else default to a basic level.
    String file = theJarFilePath + props;
    if (new File(file).exists()) {
      PropertyConfigurator.configureAndWatch(file);
    } else {
      BasicConfigurator.configure();
      logger.setPriority(Priority.WARN);
      
      // Default to a rolling file
      setRollingFileAppender(DEFAULT_LOG_FILE);
    }
  }

  /**
   * Set the appender for the Logger to a rolling file appender.
   */
  public static void setRollingFileAppender(String file) {
    String path = theJarFilePath;
    if (path != null) {
      path += file;
      try {
        PatternLayout lay = new PatternLayout("%d (%r) %p [%t]: %m\n");
        RollingFileAppender app = new RollingFileAppender(lay, path);
        logger.addAppender(app);
        logger.setAdditivity(false);
      } catch (IOException ioe) {
        logger.info("Unable to create file appender for "+path+". "+ioe);
      }
    } else {
      logger.info("Unable to determine the Real Path, using root");
    }
  }

  /**
   * Set the appender for the Logger to a console appender.
   */
  public static void setConsoleAppender() {
    String path = theJarFilePath;
    PatternLayout lay = new PatternLayout("%d (%r) %p [%t]: %m\n");
    ConsoleAppender app = new ConsoleAppender(lay);
    logger.removeAllAppenders();
    logger.addAppender(app);
    logger.setAdditivity(false);
  }

  /**
   * Set the appender for the Logger to a syslog appender.
   */
  public static void setSyslogAppender(String host, int facility) {
    String path = theJarFilePath;
    PatternLayout lay = new PatternLayout("%d (%r) %p [%t]: %m\n");
    SyslogAppender app = new SyslogAppender(lay,host,facility);
    logger.removeAllAppenders();
    logger.addAppender(app);
    logger.setAdditivity(false);
  }

  /**
   * @return the directory where the jar files are stored in
   */
  public static String getJarDirectory () {
    String directory = "";
    
    // Determine the correct working path for the file
    // Using a known resource extract the install directory.
    // The path of the jar file will be given followed by '!' then the 
    // path of the resource.
    URL resourceurl = Logger.class.getResource("/com/nextone/util/Logger.class");

    if(resourceurl	==	null) {
      error("Unable to locate the resource");
      return "";
    }

    String d = resourceurl.getPath();

    StringTokenizer st = new StringTokenizer(d, "!");
    try {
       directory = new File(new URL(st.nextToken()).getPath()).getParent();
    } catch (MalformedURLException me) {
       System.err.println("Cannot parse directory "+me.toString()+"\n"+me.getMessage());
       directory = null;
    }

   return (directory == null)?"":directory + File.separator;
  }

  /////////////////////////////////////////////////////////////
  // Logging methods
  /////////////////////////////////////////////////////////////
  /**
   * Create a log entry with debug priority.
   */
  public static void debug (Object log) {
    logger.debug(log);
  }

  public static void debug (Object log, Throwable t) {
    logger.debug(log, t);
  }
  
  /**
   * Create a log entry with info priority.
   */
  public static void info (Object log) {
    logger.info(log);
  }

  public static void info (Object log, Throwable t) {
    logger.info(log, t);
  }

  /**
   * Create a log entry with warn priority.
   */
  public static void warn (Object log) {
    logger.warn(log);
  }

  public static void warn (Object log, Throwable t) {
    logger.warn(log, t);
  }

  /**
   * Create a log entry with error priority.
   */
  public static void error (Object log) {
    logger.error(log);
  }

  public static void error (Object log, Throwable t) {
    logger.error(log, t);
  }
  
  /**
   * Create a log entry with fatal priority.
   */
  public static void fatal(Object log) {
    logger.fatal(log);
  }

  public static void fatal(Object log, Throwable t) {
    logger.fatal(log, t);
  }
}

