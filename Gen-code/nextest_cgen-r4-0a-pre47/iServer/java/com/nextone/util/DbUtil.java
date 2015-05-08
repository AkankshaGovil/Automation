package com.nextone.util;

import java.sql.*;
import java.io.*;

/**
 * contains methods used to access databases using JDBC
 */
public class DbUtil {
	  private DbUtil () {}  // no one should call this


	  /**
	   * extract all nested SQL exception strings and returns the result
	   */
	  public static String getAllSQLException (SQLException se) {
		 StringBuffer sb = new StringBuffer(100);
		 while (se != null) {
			sb.append(se.getMessage() + "\n");
			se = se.getNextException();
		 }
		 return sb.toString();
	  }

	  /**
	   * loads the JDBC driver, open the connection to the database and
	   * returns the connection handle
	   */
	  public static Connection getConnection (String driver, String url, String username, String password) throws SQLException {
		 // load the JDBC driver
		 try {
			Class.forName(driver).newInstance();
		 } catch (ClassNotFoundException ce) {
			throw new SQLException("Cannot find JDBC driver: " + ce.getMessage());
		 } catch (InstantiationException ie) {
			throw new SQLException("Cannot load JDBC driver: " + ie.getMessage());
		 } catch (IllegalAccessException ie) {
			throw new SQLException("Cannot load JDBC driver: " + ie.getMessage());
		 }

		 // open the connection
		 return DriverManager.getConnection(url, username, password);
	  }

	  /**
	   * given a PreparedStatement, parameter index, SQL type and the value,
	   * calls the appropriate setXXX method on the statment
	   */
	  public static void setValue (PreparedStatement stmt, int pi, int type, Object o) throws SQLException {
		 switch (type) {
			case Types.VARBINARY:
			case Types.LONGVARBINARY:
			  byte [] ba = null;
			  if (o instanceof byte[])
				 ba = (byte [])o;
			  else if (o instanceof String)
				 ba = ((String)o).getBytes();
			  else
				 throw new SQLException("Object passed (" + o.getClass().getName() + ") does not match type (" + type + ")\nShould either be a String or byte []");
			  ByteArrayInputStream bis = new ByteArrayInputStream(ba);
			  stmt.setBinaryStream(pi, bis, ba.length);
			  break;

			case Types.CHAR:
			case Types.VARCHAR:
			case Types.LONGVARCHAR:
			  stmt.setString(pi, o.toString());
			  break;

			case Types.DATE:
			  if (o instanceof java.sql.Date)
				 stmt.setDate(pi, (java.sql.Date)o);
			  else
				 throw new SQLException("Object passed (" + o.getClass().getName() + ") does not match type (" + type + ")");
			  break;

			case Types.TIME:
			  if (o instanceof java.sql.Time)
				 stmt.setTime(pi, (java.sql.Time)o);
			  else
				 throw new SQLException("Object passed (" + o.getClass().getName() + ") does not match type (" + type + ")");
			  break;

			case Types.TIMESTAMP:
			  java.sql.Timestamp ts = null;
			  if (o instanceof java.sql.Timestamp)
				 ts = (java.sql.Timestamp)o;
			  else if (o instanceof java.sql.Date)
				 ts = new java.sql.Timestamp(((java.sql.Date)o).getTime());
			  else if (o instanceof java.sql.Time)
				 ts = new java.sql.Timestamp(((java.sql.Time)o).getTime());
			  else
				 throw new SQLException("Object passed (" + o.getClass().getName() + ") does not match type (" + type + ")"); 
			  stmt.setTimestamp(pi, ts);
			  break;

			case Types.INTEGER:
			  if (o instanceof Integer)
				 stmt.setInt(pi, ((Integer)o).intValue());
			  else
				 throw new SQLException("Object passed (" + o.getClass().getName() + ") does not match type (" + type + ")");
			  break;

			case Types.SMALLINT:
			  if (o instanceof Short)
				 stmt.setShort(pi, ((Short)o).shortValue());
			  else
				 throw new SQLException("Object passed (" + o.getClass().getName() + ") does not match type (" + type + ")");
			  break;

			default:
			  throw new SQLException("Unsupported type (" + type + ") for parameter " + pi + "(" + o + ")");
		 }
	  }

	  /**
	   * returns a Thread which reads bytes from the inputstream and writes
	   * the bytes into the outputstream
	   */
	  public static Thread getReadWriteInThread (InputStream is, OutputStream os) {
		 return getReadWriteInThread(is, os, null);
	  }

	  /**
	   * returns a Thread which reads bytes from the inputstream and writes
	   * the bytes into the outputstream
	   */
	  public static Thread getReadWriteInThread (InputStream is, OutputStream os, String var) {
		 return new ReadWriteInThread(is, os, var);
	  }

}

class ReadWriteInThread extends Thread {
	  private InputStream is;
	  private OutputStream os;
	  private String varType;

	  ReadWriteInThread (InputStream is, OutputStream os, String var) {
		 this.is = is;
		 this.os = os;
		 this.varType = var;
	  }

	  public void run () {
		 int b = -1;
		 try {
			while ( (b = is.read()) != -1) {
			   os.write((byte)b);
			}
		 } catch (IOException ie) {
			// it would also get here if the InputStream being read is
			// closed by some other thread
		 }
	  }
}
	  
