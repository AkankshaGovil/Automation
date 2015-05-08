*************************************************************************************************
*************************************************************************************************
** 2000 NetNumber.com, Inc.
**
**
** The following information pertains to the contents of the files in the ENUMResolverC.zip
** file and the NetNumber ENUM Client Resolver API for Java v1.0.  It also explains how to setup
** and run the example program included with this package.
** 
**    Copyright (C) 2000  Doug Chan
**
**    This library is free software; you can redistribute it and/or
**    modify it under the terms of the GNU Lesser General Public
**    License as published by the Free Software Foundation; either
**    version 2.1 of the License, or (at your option) any later version.
**
**    This library is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**    Lesser General Public License for more details.
**
**    You should have received a copy of the GNU Lesser General Public
**    License along with this library; if not, write to the Free Software
**    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**
**    Doug Chan
**    NetNumber.com, Inc.
**    650 Suffolk Street
**    Suite 307
**    Lowell, MA  01854
**    dchan@netnumber.com
**
**************************************************************************************************
**************************************************************************************************

Contents of this enumres_c2_0.zip file:

- \src - contains source code necessary to build/compile the query executable
- \lib - empty until a build is done
- \include - contains header files necessary to build/compile the query executable
- \docs - contains docs for the Resolver API
	
- makefile - Sample makefile for solaris and linux


Building the ENUM resolver library under platforms other than Solaris and RedHat Linux
======================================================================================

The following does not apply to a Microsoft Windows platform:

Makefiles provided with this package pertain to Solaris and RedHat Linux 6.2 and greater.  To build the libENUM.a libary under a different platform you need to be aware of the following:

1) Our ENUM resolver client uses the BIND C library functions.  Explicitly it uses certain ns_ library
functions to parse the response from the server.  We have found that some operating systems do not
include this BIND support as part of their default OS.  If you do not have a later version of BIND (8.2 
or above) you can download the source from http://www.isc.org/products/BIND/bind8.html

2) Once you have downloaded and built the later version of BIND on your system, edit the makefile.  
The E164_RESOLVER_DIR should specify the path to the resolver code and BIND_BASE_DIR should contain 
the path to the BIND 8.2.2 directory.

3) Run make.

New to Version 2.0
=======================

Version 2 of the ENUM C Resolver API is more closely in line with the ENUM Java Resolver API.  The functions within version 1 have been collapsed into 1 primary function for querying the NetNumber ENUM service.  Version 2 provides the following functionality:


1) Given a properly formed E.164 telephone number and service filter, return the associated set of NAPTR or ENUM records.

2) Support for record sorting and NAPTR regular expression handling.

3)The ability to serially query one or more corporate, commercial and/or public ENUM domains. (Example e164.company.com, e164.com and e164.arpa)

4) An interface for querying for raw NAPTR records, as well as, ENUM records.



Note

