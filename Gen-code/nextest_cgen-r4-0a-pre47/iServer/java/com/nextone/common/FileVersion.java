package com.nextone.common;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.zip.*;
//import java.util.jar.*;


import thirdparty.ftp.*;
import thirdparty.tar.*;


/**
 * This class downloads a file and extracts the version information from
 * it. For softrom/ram files, the version is encoded inside the file, for
 * config, the version is calculated as a checksum
 */
public class FileVersion {
	  private DTServer server;
	  private int action, devType;
	  private String version;
	  private byte [] fileContents;
	  private boolean currentlyRetrieving;
	  private Object syncObject = new Object();
	  private FtpBean cachedFtpBean;

	  public FileVersion (int a, int d, DTServer s) {
		 this.action = a;
		 this.devType = d;
		 this.server = s;
	  }

	  private FtpBean openConnection () throws IOException {
		 if (server == null)
			throw new IOException("No server configured");
		 if (server.getServerAddress() == null)
			throw new IOException("No server address configured");
		 if (server.getUsername() == null ||
			 server.getUsername().equals(""))
			throw new IOException("No username configured");
		 if (server.getPassword() == null)
			throw new IOException("No password configured");

		 if (currentlyRetrieving)
			throw new IOException("Already retrieving file");

		 setCurrentlyRetrieving(true);

		 try {
			FtpBean ftp = new FtpBean();
			ftp.ftpConnect(server.getServerName(), server.getUsername(), server.getPassword());
			return ftp;
		 } catch (FtpException fe) {
			throw new IOException (fe.getMessage());
		 }
	  }

	  public void retrieveFile () throws IOException {
		 FtpBean ftp = null;

		 try {
			ftp = openConnection();
			String filename = server.getFilePath(devType);
			if (filename == null || filename.equals(""))
			   throw new IOException("no file name configured");
			if (action == MaintenanceRequest.DOWNLOAD_CONFIG_ACTION) {
			   fileContents = ftp.getAsciiFile(filename, "\n").getBytes();
			   // since we downloaded the file, let's cache the version
			   // for future use
			   version = calculateConfigVersion(new String(fileContents));
			}
		 } catch (Exception ie) {
			setCurrentlyRetrieving(false);
			throw new IOException(ie.getMessage());
		 }

		 setCurrentlyRetrieving(false);
		 try {
			ftp.close();
		 } catch (Exception e) {} // ignore any errors while closing...

		 if (action != MaintenanceRequest.DOWNLOAD_CONFIG_ACTION)
			throw new IOException("Retrieving file contents for action " + MaintenanceRequest.getActionString(action) + " not yet supported");
	  }

	  public void retrieveVersion () throws IOException {
		 FtpBean ftp = null;

		 try {
			ftp = openConnection();
			String filename = server.getFilePath(devType);
			if (filename == null || filename.equals(""))
			   throw new IOException("no file name configured");
			if (action == MaintenanceRequest.DOWNLOAD_CONFIG_ACTION) {
			   extractConfigVersion(ftp, filename);
			} else if (action == MaintenanceRequest.SAVE_CONFIG_ACTION) {
			   checkFileLocations(ftp, filename);
			} else {
			   extractSoftwareVersion(ftp, filename, devType);
			}
		 } catch (IOException ie) {
			setCurrentlyRetrieving(false);
			throw ie;
		 }

		 setCurrentlyRetrieving(false);
		 try {
			ftp.close();
		 } catch (Exception e) {} // ignore any errors while closing
	  }

	  private void setCurrentlyRetrieving (boolean b) {
		 synchronized (syncObject) {
			currentlyRetrieving = b;
		 }
	  }

	  // returns an array of Strings, [0] contains the parent directory and
	  // [1] contains the last name in the path
	  private String [] basename (String path) {
		 String separator = File.separator;
		 String [] result = new String [2];

		 // we don't handle the C:, D: junk
		 if (path.length() >=3 &&
			 path.charAt(1) == ':' &&
			 (path.charAt(2) == '/' || path.charAt(2) == '\\'))
			path = path.substring(3, path.length());
			

		 int index = path.lastIndexOf(separator);
		 if (index == -1) {
			// no separator found, means only one child exist
			result[0] = "";
			result[1] = path;
		 } else {
			result[0] = path.substring(0, index);
			result[1] = path.substring(index+1, path.length());
		 }

		 return result;
	  }

	  // checks if the filename given is a directory name and creates if it doesn't
	  // exist
	  private void checkFileLocations (FtpBean ftp, String filename) throws IOException {
		 Stack dirnames = new Stack();

		 // stay in the loop until the necessary directories are created/verified
		 while (true) {
			// first CD to the directory
			try {
			   if (filename.equals(""))
				  filename = "/";
			   ftp.setDirectory(filename);
			} catch (FtpException fe) {
			   // if CD to root failed, we cannot recover
			   if (filename.equals("/"))
				  throw new IOException("Error accessing the root directory: " + fe.getMessage());

			   // CD failed, shave of the last file entry in the directory string
			   // and try again
			   String [] result = basename(filename);
			   filename = result[0];
			   dirnames.push(result[1]);
			   continue;
			}   

			// CD was successfull, now create any of the filenames left in the stack
			String newdir = null;
			try {
			   while (!dirnames.empty()) {
				  newdir = (String)dirnames.pop();
				  ftp.makeDirectory(newdir);
				  ftp.setDirectory(newdir);
			   }
			} catch (FtpException fe) {
			   throw new IOException("Error accessing directory (" + newdir + "): " + fe.getMessage());
			}

			break;
		 }

		 version = "";
	  }

	  private void extractConfigVersion (FtpBean ftp, String filename) throws IOException {
		 try {
			String file = ftp.getAsciiFile(filename, "\n");
			fileContents = file.getBytes();
			version = calculateConfigVersion(new String(fileContents));
		 } catch (FtpException fe) {
			throw new IOException(fe.getMessage());
		 }
	  }

	  /**
	   * calculates the "version" (checksum) of the given config file
	   * @return a string upto 16 bytes long
	   */
	  private String calculateConfigVersion (String config) {
		 // get the checksum on the file
		 CRC32 crc = new CRC32();
		 crc.update(config.getBytes());
		 long cksum = crc.getValue();

//		 System.out.print("checksum: " + cksum + " string: ");
		 // now convert this checksum into a 16 byte long string
		 StringBuffer sb = new StringBuffer();
		 for (int i = 0; i < 16; i++) {
			// extract a nibble
			long value = cksum >>> (i*4);
			// move it into printable range(0x20 - 0x7e)
			int b = (int)((value & 0xf) | 0x40);
			// now this nibble is a printable ascii character
			Character c = new Character((char)b);
//			System.out.print(Integer.toBinaryString((int)(value & 0xf)) + ":" + Integer.toBinaryString(b) + ":" + c.charValue() + " ");
			sb.insert(0, c);
		 }


		 return sb.toString();
	  }

	  private void extractSoftwareVersion (FtpBean ftp, String filename, int devType) throws IOException {

		 if (devType == CommonConstants.DEVTYPE_I1000){

			 extract1000SoftwareVersion (ftp,filename, devType);
			 return;
		 }


		 BufferedReader br = null;
		 try {
			br = new BufferedReader(new InputStreamReader(new ByteArrayInputStream(ftp.getBinaryFile(filename))));
		 } catch (FtpException fe) {
			throw new IOException(fe.getMessage());
		 }

		 String buildType = null;
		 try {
			buildType = br.readLine();
		 } catch (IOException ie) {}
		 if (buildType == null) {
			setCurrentlyRetrieving(false);
			throw new IOException("The image did not contain the proper header information");
		 } else {
			// validate the build type...
			StringTokenizer st = new StringTokenizer(buildType, "=");
			if (st.countTokens() != 2) {
			   setCurrentlyRetrieving(false);
			   throw new IOException("The image did not contain the proper header information");
			}
			st.nextToken();
			String type = st.nextToken();
			String thisType = ConfigFile.getDeviceTypeString(devType);
			if (!type.startsWith(thisType))
			   throw new IOException("Image type (" + type + ") is not the expected type (" + thisType + ")");
		 }

		 String buildVersion = null;
		 try {
			buildVersion = br.readLine();
			br.close();
		 } catch (IOException ie) {}
		 if (buildVersion == null) {
			setCurrentlyRetrieving(false);
			throw new IOException("The image did not contain the proper header information");
		 }

		 // extract the version info
		 StringTokenizer st = new StringTokenizer(buildVersion, "=");
		 if (st.countTokens() != 2) {
			setCurrentlyRetrieving(false);
			throw new IOException("The image did not contain the proper header information");
		 }
		 st.nextToken();
		 version = st.nextToken();
	  }

	  private void extract1000SoftwareVersion (FtpBean ftp, String filename, int devType) throws IOException {
			 //Read the tar file
			File	installFile	=	new File("iedge1000install.tar");;
			try {
				ftp.getBinaryFile(filename, installFile.getName(),0);
				TarInputStream	tarFile	=	new TarInputStream(new FileInputStream(installFile));

				TarEntry	entry;
				version	=	"";
				while((entry	=	tarFile. getNextEntry()	)!=	null){
					if(entry.getName().equals("versionfile")){
						byte	[]bVersion	=	new byte[(int)entry.getSize()];
						if(tarFile.read(bVersion)	!=	-1)
							version	=	new String(bVersion);
					}

				}
				tarFile.close();
				installFile.delete();
			} catch (FtpException fe) {
				installFile.delete();
				throw new IOException(fe.getMessage());
			} catch (IOException ie) {
				installFile.delete();
				throw new IOException(ie.getMessage());
			}
			 return;

	  }

	  public String getVersion () {
		 return version;
	  }

	  private FtpBean getCachedFtpBean () throws IOException, FtpException {
		 // check if the connection is still up
		 if (cachedFtpBean != null) {
			try {
			   cachedFtpBean.getDirectory();
			} catch (FtpException fe) {
			   // connection is closed
			   cachedFtpBean = null;
			}
		 }

		 // open a new connection
		 if (cachedFtpBean == null) {
			cachedFtpBean = openConnection();
			String filename = server.getFilePath(devType);
			if (filename != null && !filename.equals(""))
			   cachedFtpBean.setDirectory(filename);
		 }

		 return cachedFtpBean;
	  }

	  public void setVersion (String ver) throws IOException {
		 version = ver;
	  }

	  public void saveConfig (String regid, String config) throws IOException {
		 if (config == null) {
			// means we have to close down the connection
			try {
			   if (cachedFtpBean != null) {
				  cachedFtpBean.close();
			   }
			} catch (Exception e) {}  // ignore any errors here
		 } else {
			// save the config to the ftp server
			try {
			   FtpBean ftp = getCachedFtpBean();
			   ftp.putAsciiFile(regid + ".cfg", config, "\n");
			} catch (FtpException fe) {
			   throw new IOException("Error storing configuration: " + fe.getMessage());
			}
		 }
	  }

	  public byte [] getFile () {
		 return fileContents;
	  }

	  public void setFile (byte [] b) {
		 fileContents = b;
	  }

}
