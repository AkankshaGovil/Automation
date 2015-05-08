package com.nextone.util;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.text.*;
import java.sql.*;
import com.nextone.common.ConsumeMouseGlassPane;

public class DatabaseEditor extends JFrame {
	  private Container parent;
	  private String driver, url, username, password;
	  private String [] tables;
	  private JTable [] jta;
	  private JPanel [] jpa;
	  private DETableModel [] detma;
	  private TableSorter [] detsa;
	  private JPopupMenu [] popupMenus;
	  private ConsumeMouseGlassPane cmgp;

	  public DatabaseEditor (Container c, String dr, String url, String user, String pass, String [] t) throws SQLException {
		 super();
		 this.parent = c;
		 this.driver = dr;
		 this.url = url;
		 this.username = user;
		 this.password = pass;
		 this.tables = t;

		 if (parent instanceof JFrame || parent instanceof Frame)
			setIconImage(((Frame)parent).getIconImage());

		 cmgp = new ConsumeMouseGlassPane();
		 setGlassPane(cmgp);

		 // some validation
		 if (driver == null || driver.equals(""))
			throw new SQLException("Please specify a database driver");
		 if (url == null || url.equals(""))
			throw new SQLException("Please specify a database URL");
		 if (username == null || username.equals(""))
			throw new SQLException("Please specify a username to login to the database");
		 if (password == null)
			password = "";
		 if (tables == null || tables.length == 0)
			throw new SQLException("Please specify the table(s) to display in the database");

		 // title of the frame
		 StringBuffer sb = new StringBuffer("Database Editor: ");
		 for (int i = 0; i < tables.length; i++)
			sb.append(tables[i] + " ");
		 setTitle(sb.toString());

		 jta = new JTable [tables.length];
		 jpa = new JPanel [tables.length];
		 detma = new DETableModel [tables.length];
		 detsa = new TableSorter [tables.length];
		 popupMenus = new JPopupMenu [tables.length];

		 // create the panels with tables
		 for (int i = 0; i < tables.length; i++) {
			// create a popup menu for this table
			popupMenus[i] = new JPopupMenu();
			JMenuItem jmi = new JMenuItem("Delete");
			jmi.setActionCommand("delete");
			TableActionListener tal = new TableActionListener(i);
			jmi.addActionListener(tal);
			popupMenus[i].add(jmi);

			jpa[i] = new JPanel();
			JPanel jp = jpa[i];  // convenient variable
			jp.setLayout(new BorderLayout());
			jp.setBorder(new EmptyBorder(3, 3, 3, 3));

			detma[i] = new DETableModel(i);
			detsa[i] = new TableSorter(detma[i]);
			TableSorter dets = detsa[i];
			jta[i] = new JTable(dets);
			JTable jt = jta[i];
			jt.addMouseListener(new TableMouseListener(i));

			dets.addMouseListenerToHeaderInTable(jt);
			jt.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
			jt.setShowGrid(false);
			jt.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

			JScrollPane jsp = new JScrollPane(jt);
			jsp.setViewportBorder(BorderFactory.createLineBorder(Color.black));
			jsp.getViewport().setBackground(Color.white);
			jt.addNotify();

			JPanel np = new JPanel();
			np.setLayout(new FlowLayout(FlowLayout.LEFT));
			np.add(new JLabel(tables[i] + ": ", JLabel.LEFT));
			JButton rb = new JButton("Refresh");
			rb.setToolTipText("Refresh the contents of this table");
			rb.setActionCommand("refresh-table");
			rb.addActionListener(tal);
			np.add(Box.createHorizontalStrut(5));
			np.add(rb);
			jp.add(np, BorderLayout.NORTH);
			jp.add(jsp, BorderLayout.CENTER);
		 }

		 // now add those panels to the main JFrame
		 Component mc = null;
		 if (tables.length == 1) {
			mc = jpa[0];
		 } else {
			for (int i = 0; i < tables.length; i++) {
			   if (i == 0) {
				  JSplitPane jsplp = new JSplitPane(JSplitPane.VERTICAL_SPLIT, true, jpa[0], jpa[1]);
				  jsplp.setOneTouchExpandable(true);
				  jsplp.setContinuousLayout(true);
				  jsplp.setResizeWeight(0.3);
				  mc = jsplp;
			   } else if (i > 1) {
				  JSplitPane jsplp = new JSplitPane(JSplitPane.VERTICAL_SPLIT, true, mc, jpa[i]);
				  jsplp.setOneTouchExpandable(true);
				  jsplp.setContinuousLayout(true);
				  jsplp.setResizeWeight(0.3);
				  mc = jsplp;
			   }
			}
		 }

		 getContentPane().add(mc);

		 Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
		 setSize(new Dimension(620, 460));
		 setBackground(Color.white);
		 setLocation( (d.width - getBounds().width)/2,
					  (d.height - getBounds().height)/2 );


		 addWindowListener(new WindowAdapter () {
			public void windowClosing (WindowEvent we) {
			   exit();
			}
		 });

		 // show this frame
		 setVisible(true);

		 // start the thread which refreshes the tables
		 refresh();
	  }

	  private class TableActionListener implements ActionListener {
			private int index;

			TableActionListener (int i) {
			   index = i;
			}

			public void actionPerformed (ActionEvent ae) {
			   String cmd = ae.getActionCommand();
			   if (cmd.equals("delete"))
				  detma[index].deleteSelectedRows();
			   else if (cmd.equals("refresh-table"))
				  new Thread(new RefreshThread(index)).start();
//			   else
//				  System.out.println("unknown menu command received");
			}
	  }

	  private class TableMouseListener extends MouseAdapter {
			private int index;
			
			TableMouseListener (int i) {
			   index = i;
			}

			public void mousePressed (MouseEvent me) {
			   maybeShowPopup(me);
			}

			public void mouseReleased (MouseEvent me) {
			   maybeShowPopup(me);
			}

			private void maybeShowPopup (MouseEvent me) {
			   if (me.isPopupTrigger() &&
				   jta[index].getSelectionModel().isSelectedIndex(jta[index].rowAtPoint(me.getPoint())))
				  popupMenus[index].show(me.getComponent(), me.getX(), me.getY());
			}

	  }

	  private void exit () {
		 setVisible(false);
		 dispose();
		 parent.repaint();
	  }

	  public void setGlassCursor (int c) {
		 if (c == Cursor.WAIT_CURSOR)
			cmgp.setVisible(true);
		 else if (c == Cursor.DEFAULT_CURSOR)
			cmgp.setVisible(false);
		 setCursor(new Cursor(c));
	  }

	  // refresh all the tables
	  private void refresh () {
		 new Thread(new RefreshThread()).start();
	  }

	  private class DETableModel extends AbstractTableModel {
			private SimpleDateFormat df = new SimpleDateFormat("EEE MMM dd hh:mm:ssa yyyy z");
			private String [] columnNames;
			private int [] columnTypes;
			private int [] keys;  // keys in this table (column indexes)
			private java.util.List rows;
			private boolean readyToShow = false;
			private int index;  // index of this model in the array of models

			DETableModel (int i) {
			   index = i;
			}

			public int getColumnCount () {
			   if (readyToShow)
				  return columnNames.length;
			   return 0;
			}

			public int getRowCount () {
			   if (readyToShow)
				  return rows.size();
			   return 0;
			}

			public String getColumnName (int col) {
			   if (readyToShow)
				  return columnNames[col];
			   return "";
			}

			public Object getValueAt (int row, int col) {
			   if (!readyToShow || row < 0 || row >= rows.size())
				  return "";

			   return ((Object [])rows.get(row))[col];
			}

			public void setValueAt (Object o, int row, int col) {
			   // see if it is equal to the old value
			   Object oldObject = ((Object [])rows.get(row))[col];
			   if (oldObject == null && o == null ||
				   oldObject == null && o.equals("") ||
				   o.equals(oldObject))
				  return;

			   setGlassCursor(Cursor.WAIT_CURSOR);
			   new UpdateThread(row, col, o).start();
			}

			public void deleteSelectedRows () {
			   setGlassCursor(Cursor.WAIT_CURSOR);
			   new DeleteThread().start();
			}

			private String [] getSearchableColumnNames (int [] vindex) {
			   String [] s = new String [vindex.length];
			   for (int i = 0; i < s.length; i++)
				  s[i] = columnNames[vindex[i]];
			   return s;
			}

			private Object [] getSearchableColumnValues (int row, int [] vindex) {
			   Object [] s = new Object [vindex.length];
			   for (int i = 0; i < s.length; i++) {
				  s[i] = ((Object [])rows.get(row))[vindex[i]];
			   }
			   return s;
			}

			// keys[] has all searchable column indexes
			private int [] getValidSearchableIndexes (int row) {
			   int [] allindexes = new int [keys.length];
			   Arrays.fill(allindexes, -1);
			   int k = 0;
			   for (int i = 0; i < keys.length; i++) {
				  Object o = ((Object [])rows.get(row))[keys[i]];
				  if (o == null || o.equals(""))
					 continue;
				  allindexes[k++] = keys[i];
			   }

			   int [] vindex = new int [k];
			   for (int i = 0; i < k; i++) {
				  vindex[i] = allindexes[i];
			   }

			   return vindex;
			}

			public Class getColumnClass (int col) {
			   Object o = getValueAt(0, col);
			   if (o == null)
				  o = "";
			   return o.getClass();
			}

			// determines whether or not to show the table data
			public void setReadyToShow (boolean b) {
			   readyToShow = b;
			}

			public void setColumnNames (String [] sa) {
			   columnNames = sa;
			}

			public void setColumnTypes (int [] t) {
			   columnTypes = t;
			}

			public void setKeys (int [] k) {
			   keys = k;
			}

			public void setRows (java.util.List l) {
			   rows = l;
			}

			public boolean isCellEditable (int row, int col) {
			   Class c = getColumnClass(col);
			   if (c.isInstance(""))
				  return true;

			   return false;
			}

			private class DeleteThread extends Thread {
				  public void run () {
					 int [] irows = jta[index].getSelectedRows();
					 int [] srows = new int [irows.length];
					 for (int i = 0; i < srows.length; i++) {
						srows[i] = detsa[index].getActualRow(irows[i]);
					 }

					 if (srows.length <= 0) {
						setGlassCursor(Cursor.DEFAULT_CURSOR);
						return;
					 }

					 // open database connection
					 Connection conn = null;
					 try {
						conn = DbUtil.getConnection(driver, url, username, password);
					 } catch (SQLException se) {
						PopMessage.showError(DatabaseEditor.this, "Error opening database connection:\n" + DbUtil.getAllSQLException(se), se);
						setGlassCursor(Cursor.DEFAULT_CURSOR);
						return;
					 }

					 java.util.List drows = new ArrayList();
					 for (int c = 0; c < srows.length; c++) {
						int crow = srows[c];
						StringBuffer sb = new StringBuffer();
						sb.append("DELETE FROM ");
						sb.append(tables[index]);
						int [] vindex = getValidSearchableIndexes(crow);
						String [] scn = getSearchableColumnNames(vindex);
						for (int i = 0; i < scn.length; i++) {
						   if (i == 0)
							  sb.append(" WHERE ");
						   else
							  sb.append(" AND ");
						   sb.append(scn[i]);
						   sb.append(" = ?");
						}

						Object [] scv = getSearchableColumnValues(crow, vindex);
						try {
						   PreparedStatement deleteEntry = conn.prepareStatement(sb.toString());
						   for (int i = 0; i < scv.length; i++)
							  DbUtil.setValue(deleteEntry, i+1, columnTypes[vindex[i]], scv[i]);
						   if (deleteEntry.executeUpdate() < 1)
							  PopMessage.showError(DatabaseEditor.this, "Unable to delete row " + (c+1));
						   else
							  drows.add(rows.get(srows[c]));
						   deleteEntry.close();
						} catch (SQLException se) {
						   PopMessage.showError(DatabaseEditor.this, "Error while deleting row " + (c+1) + ":\n" + DbUtil.getAllSQLException(se), se);
						}
					 }

					 // close the database connection
					 try {
						conn.close();
					 } catch (SQLException se) {
						PopMessage.showError(DatabaseEditor.this, "Error closing database connection:\n" + DbUtil.getAllSQLException(se), se);
					 }

					 // delete things in the internal table
					 rows.removeAll(drows);
					 detsa[index].tableChanged(new TableModelEvent(detma[index]));
//					 fireTableRowsDeleted(drows[i], drows[i]);
					 fireTableDataChanged();

					 setGlassCursor(Cursor.DEFAULT_CURSOR);
				  }
			}

			private class UpdateThread extends Thread {
				  private int row, col;
				  private Object o;

				  UpdateThread (int row, int col, Object o) {
					 this.row = row;
					 this.col = col;
					 this.o = o;
				  }

				  public void run () {
					 StringBuffer sb = new StringBuffer();
					 sb.append("UPDATE ");
					 sb.append(tables[index]);
					 sb.append(" SET ");
					 sb.append(getColumnName(col));
					 sb.append(" = ?");
					 int [] vindex = getValidSearchableIndexes(row);
					 String [] scn = getSearchableColumnNames(vindex);
					 for (int i = 0; i < scn.length; i++) {
						if (i == 0)
						   sb.append(" WHERE ");
						else
						   sb.append(" AND ");
						sb.append(scn[i]);
						sb.append(" = ?");
					 }

					 Object [] scv = getSearchableColumnValues(row, vindex);
					 try {
						Connection conn = DbUtil.getConnection(driver, url, username, password);
						PreparedStatement updateEntry = conn.prepareStatement(sb.toString());
						DbUtil.setValue(updateEntry, 1, columnTypes[col], o);
						for (int i = 0; i < scv.length; i++)
						   DbUtil.setValue(updateEntry, i+2, columnTypes[vindex[i]], scv[i]);
						if (updateEntry.executeUpdate() != 1)
						   PopMessage.showError(DatabaseEditor.this, "Unable to update entry");
						else {
						   ((Object [])rows.get(row))[col] = o;
						   fireTableCellUpdated(row, col);
						}
						updateEntry.close();
						conn.close();
					 } catch (SQLException se) {
						PopMessage.showError(DatabaseEditor.this, "Error while updating entry:\n" + DbUtil.getAllSQLException(se), se);
					 }
					 setGlassCursor(Cursor.DEFAULT_CURSOR);
				  }
			}

	  }

	  // refreshes all the tables
	  private class RefreshThread extends JDialog implements Runnable {
			private JButton ok;
			private JTextArea text;
			private Thread currentThread;
			private WindowListener wl;
			private ActionListener al;
			private Connection conn = null;
			PreparedStatement stmt = null;
			ResultSet rs = null;
			private int tableIndex = -1;

			// updates all the tables
			RefreshThread () {
			   this(-1);
			}

			// updates the selected table
			RefreshThread (int tableIndex) {
			   super(DatabaseEditor.this, "Status", false);
			   this.tableIndex = tableIndex;
			   wl = new WindowAdapter() {
						public void windowClosing (WindowEvent we) {
						   setWindowCursor(Cursor.WAIT_CURSOR);
						   finishThread();
						   destroy();
						}
				  };
			   addWindowListener(wl);

			   ok = new JButton("OK");
			   ok.setEnabled(false);
			   al = new ActionListener() {
						public void actionPerformed (ActionEvent ae) {
						   destroy();
						}
				  };
			   ok.addActionListener(al);

			   text = new JTextArea(20, 3);
			   text.setEditable(false);
			   JScrollPane jsp = new JScrollPane(text);
			   JPanel cp = new JPanel();
			   cp.setLayout(new BoxLayout(cp, BoxLayout.Y_AXIS));
			   cp.add(jsp);
			   cp.add(Box.createVerticalStrut(10));
			   JPanel tp = new JPanel();
			   tp.setLayout(new FlowLayout(FlowLayout.CENTER));
			   tp.add(ok);
			   cp.add(tp);
			   getContentPane().add(cp);
			   setSize(new Dimension(450, 350));
			   setLocation(((int)DatabaseEditor.this.getLocation().getX() + (DatabaseEditor.this.getSize().width - getBounds().width)/2), ((int)DatabaseEditor.this.getLocation().getY() + (DatabaseEditor.this.getSize().height - getBounds().height)/2));
			   setVisible(true);
			}

			private void setWindowCursor (int c) {
			   setCursor(new Cursor(c));
			}

			private void finishThread () {
			   if (currentThread == null)
				  return;
			   currentThread.interrupt();
			   try {
				  currentThread.join(2000);
			   } catch (Exception e) {}
			}

			private void destroy () {
			   setVisible(false);
			   removeWindowListener(wl);
			   ok.removeActionListener(al);
			   dispose();
			   setGlassCursor(Cursor.DEFAULT_CURSOR);
			}

			public void run () {
			   currentThread = Thread.currentThread();

			   // open database connection
			   text.append("\nConnecting to the database... ");
			   try {
				  conn = DbUtil.getConnection(driver, url, username, password);
			   } catch (SQLException se) {
				  text.append("failed\n");
				  closeAndReturn(se);
				  return;
			   }
			   text.append("success\n\n");

			   // for each table, update the entries 
			   int startTableIndex = 0;
			   int endTableIndex = tables.length;
			   if (tableIndex != -1) {
				  startTableIndex = tableIndex;
				  endTableIndex = tableIndex+1;
			   }
			   for (int i = startTableIndex; i < endTableIndex; i++) {
				  text.append("Retrieving entries for the " + tables[i] + " table... ");
				  try {
					 stmt = conn.prepareStatement(
						"SELECT * FROM " + tables[i]);
					 rs = stmt.executeQuery();
					 ResultSetMetaData rsmd = rs.getMetaData();
					 String [] cn = new String [rsmd.getColumnCount()];
					 int [] types = new int [rsmd.getColumnCount()];
					 int [] ikeys = new int [rsmd.getColumnCount()];
					 Arrays.fill(ikeys, -1);
					 int kk = 0;
					 for (int k = 0; k < cn.length; k++) {
						cn[k] = rsmd.getColumnName(k+1);
						types[k] = rsmd.getColumnType(k+1);
						if (rsmd.isSearchable(k+1)) {
						   ikeys[kk++] = k;
						}
					 }
					 int [] keys = new int [kk];
					 for (int k = 0; k < kk; k++) {
						keys[k] = ikeys[k];
					 }
					 java.util.List rows = new ArrayList();
					 int jj = 0;
					 while (rs.next()) {
						jj++;
//						System.out.println("\nreading row " + jj);
						Object [] row = new Object [cn.length];
						for (int k = 0; k < row.length; k++) {
//						   System.out.println("Row " + jj + " Column " + (k+1) + " is of type " + rsmd.getColumnType(k+1));
						   switch(rsmd.getColumnType(k+1)) {
							  case Types.BIGINT:
								row[k] = new Long(rs.getLong(k+1));
								break;
							  case Types.BIT:
								row[k] = new Boolean(rs.getBoolean(k+1));
								break;
							  case Types.BLOB:
								// right now we are treating this like
								// a character stream
								Blob bl = rs.getBlob(k+1);
								if (bl.length() < 65536)
								   row[k] = new String(bl.getBytes(0L, (int)bl.length()));
								else
								   row[k] = "Non-displayable content";
								break;
							  case Types.DATE:
								row[k] = rs.getDate(k+1);
								break;
							  case Types.DOUBLE:
								row[k] = new Double(rs.getDouble(k+1));
								break;
							  case Types.FLOAT:
								row[k] = new Float(rs.getFloat(k+1));
								break;
							  case Types.INTEGER:
								row[k] = new Integer(rs.getInt(k+1));
								break;
							  case Types.JAVA_OBJECT:
								row[k] = rs.getObject(k+1);
								break;
							  case Types.CHAR:
							  case Types.VARCHAR:
								row[k] = rs.getString(k+1);
								break;
							  case Types.LONGVARBINARY:
								row[k] = "";
								InputStream is = rs.getBinaryStream(k+1);
								if (rs.wasNull() || is == null)
								   break;
								ByteArrayOutputStream bos = new ByteArrayOutputStream();
								// we read this in a seperate thread, because
								// sometimes this read hangs forver
								Thread rwt = DbUtil.getReadWriteInThread(is, bos, "LONGVARBINARY");
								rwt.start();
								try {
								   rwt.join(3000); // block only for 3 seconds
								   is.close();
								   bos.close();
								} catch (Exception e) { }
								row[k] = bos.toString();  // bos may or may not contain the information
								break;
							  case Types.LONGVARCHAR:
								row[k] = "";
								is = rs.getAsciiStream(k+1);
								if (rs.wasNull() || is == null)
								   break;
								bos = new ByteArrayOutputStream();
								// we read this in a seperate thread, because
								// sometimes this read hangs forver
								rwt = DbUtil.getReadWriteInThread(is, bos, "LONGVARCHAR");
								rwt.start();
								try {
								   rwt.join(3000);  // block only for 3 seconds
								   is.close();
								   bos.close();
								} catch (Exception e) { }
								row[k] = bos.toString(); // bos may or may not contain the information
								break;
							  case Types.NULL:
								row[k] = "null";
								break;
							  case Types.NUMERIC:
								row[k] = rs.getBigDecimal(k+1);
								break;
							  case Types.SMALLINT:
								row[k] = new Short(rs.getShort(k+1));
								break;
							  case Types.TIME:
								row[k] = rs.getTime(k+1);
								break;
							  case Types.TIMESTAMP:
								row[k] = rs.getTimestamp(k+1);
								break;
							  case Types.TINYINT:
								row[k] = new Byte(rs.getByte(k+1));
								break;
							  default:
								row[k] = "Non-displayable content";
								break;
						   }
						}
						rows.add(row);
					 }
					 detma[i].setReadyToShow(false);
					 detma[i].setColumnNames(cn);
					 detma[i].setColumnTypes(types);
					 detma[i].setKeys(keys);
					 detma[i].setRows(rows);
					 detma[i].setReadyToShow(true);
					 detsa[i].tableChanged(new TableModelEvent(detma[i]));
					 detma[i].fireTableStructureChanged();
					 detma[i].fireTableDataChanged();
				  } catch (SQLException se) {
					 text.append("failed\n");
					 text.append(DbUtil.getAllSQLException(se) + "\n");
					 closeResultSet();
					 closeStatement();
					 continue;
				  }
				  closeResultSet();
				  closeStatement();
				  text.append("success\n");
			   }

			   closeAndReturn(null);
			   DatabaseEditor.this.repaint();
			}

			private boolean closeStatement () {
			   // close the previous statement
			   if (stmt != null) {
				  try {
					 stmt.close();
				  } catch (SQLException se) {
					 text.append("\nError closing the last statement\n");
					 text.append(DbUtil.getAllSQLException(se) + "\n");
					 stmt = null;
					 return false;
				  }
			   }
			   stmt = null;
			   return true;
			}

			private boolean closeResultSet () {
			   // close the previous result set
			   if (rs != null) {
				  try {
					 rs.close();
				  } catch (SQLException se) {
					 text.append("\nError closing the last result set\n");
					 text.append(DbUtil.getAllSQLException(se) + "\n");
					 rs = null;
					 return false;
				  }
			   }
			   rs = null;
			   return true;
			}

			private void closeAndReturn (Exception se) {
			   if (se != null) {
				  if (se instanceof SQLException)
					 text.append(DbUtil.getAllSQLException((SQLException)se) + "\n");
				  else
					 text.append(se.getMessage() + "\n");
			   }

			   text.append("\nClosing result sets... ");
			   if (closeResultSet())
				  text.append("success\n");
			   text.append("Closing statements... ");
			   if (closeStatement())
				  text.append("success\n");

			   text.append("\nDisconnecting from the database... ");
			   try {
				  if (conn != null && !conn.isClosed()) {
					 // rollback any changes
					 conn.rollback();
					 // close database connection
					 conn.close();
					 text.append("success\n");
				  } else
					 text.append("not needed\n");
				  conn = null;
			   } catch (SQLException sqe) {
				  text.append("failed\n");
				  text.append(DbUtil.getAllSQLException(sqe) + "\n");
				  conn = null;
			   }

			   ok.setEnabled(true);
			}
	  }

}

