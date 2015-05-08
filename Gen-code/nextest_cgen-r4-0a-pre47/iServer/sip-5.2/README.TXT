*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
      README.TXT for SIP C Stack Release 5_2_0 on Platform SOLARIS
 
          Part Id         : 1-000-5-0208-0501-14-0502-000
 
      Hughes Software Systems 
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

This release contains
- SIP C Stack Source
- User's Manual
- API Document
- Sample Test Programs

The file FILES.LST contains an exhaustive list of all files in the release


1.Platform
*-*-*-*-*
The build provided in this release is ported for SOLARIS (SunOS 5.8) systems.


2.System Requirements
*-*-*-*-*-*-*-*-*-*-*
1. A host machine with flex and bison installed. 
2. No additional Solaris Requirements. 


3.Additional Requirements
*-*-*-*-*-*-*-*-*-*-*-*-*
The code has been compiled with the compilers :
      gcc (2.95.3)
      g++ (2.95.3)
      cc: Sun WorkShop 6 update 2 C 5.3 Patch 111679-08 2002/05/09
      CC: Sun WorkShop 6 update 2 C++ 5.3 2001/05/15

If the user wants to compile the grammar rules, then FLEX (2.5.4)
and BISON (1.28) must also be installed on the system.
Also, this stack release is compatible with Bison Version 1.75.


4.Documentation
*-*-*-*-*-*-*-*
Documentation is provided separately in the documents as mentioned below:-

	SipStack_5_2_UM_1_42.pdf       - Stack User Manual
	SipStack_5_2_API_2_8_Vol_1.pdf - Stack API reference manual Vol-1
	SipStack_5_2_API_2_8_Vol_2.pdf - Stack API reference manual Vol-2

The API document introduces the user to the API architecture as well 
as how to use the APIs provided. It should be sufficient for the user 
to begin using the sip stack.


5.Stack Library
*-*-*-*-*-*-*-* 
The directory lib contains the files 

libsipcore.a
libsipapi.a 
libsipserialize.a 


6.Build for Function Based Interface: 
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 
To install the sip stack, copy file hsssipstack_5_2_0_sol_src.tar.gz 
to the directory where you want to install and execute :
			gzip -d hsssipstack_5_2_0_sol_src.tar.gz
			tar -xvf hsssipstack_5_2_0_sol_src.tar 

This will create a directory sip_stack_5_2_src which will be
the root of the SIP Stack tree. 

    a) Directory Structure for source sub-directory
                 sip_stack_5_2_src     
                    |-- source   
                       |-- accessor
                       |-- parser
                       |-- common
                       |-- serialize 

    b) Directory Structure for test sub-directory
                 sip_stack_5_2_src     
                    |-- test
                       |-- bin
                       |-- parser 


    All test program binaries are stored in bin directory.
    By default,siptest and sendmessage will be stored in bin
    directory. When the performance applications are built 
    the following applications are also stored there :
		 (i)  encodetime
		 (ii) parsetime 
		execute ./configure script to make the initial Makefile 
		       and MakeVars files

   Please refer to User Manual for more detailed directory structure.


7.Sample Test Programs
*-*-*-*-*-*-*-*-*-*-*-* 
The main test program to aid the user in using SIP Stack is :

  siptest     - used to test behaviour of the SIP Stack.

In addition, following test applications can be built :

  encodetime  - used to determine the time taken by the Stack to encode 
	              a SIP Message.
  parsetime   - used to determine the time taken by the Stack to parse 
	              a SIP Message.
  perftest    - used to obtain the performance metrics of stack. This 
	              application does rudimentary call handling.
 
  sendmessage - used to send SIP messages to a specific port 



8.Compiling the Test Programs
*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 

Steps to make encodetime and parsetime utilities:
From the install directory
 1. make performance
The binaries will be made in the test/bin directory

Steps to make the perftest utility:
From the install directory
 1. cd test/perfTest
 2. make
The binary will be made in the test/perfTest/bin directory



9.Compilation Notes
*-*-*-*-*-*-*-*-*-*-*-*
While compiling ,following warnings might come :

warning : yy_fatal_error defined but not used
warning : yyunput defined but not used
warning : yy_flex_realloc defined but not used
warning : statement not reached
warning : comparison between signed and unsigned

If the stack is compiled with CC compiler version Sun WorkShop 6 
update 2 C++ 5.3 2001/05/15 the following warnings might be 
listed:
Warning (Anachronism): End of file while skipping text -- Check #if .. #endif pairing.


10.Viewing Source Code Files
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Please view all source code & .h files using a tab space of 4.
( In vi or vim or gvim editor, do a :se ts=4 ) 
The source code & .h files have been formatted for tab space 4 
and using different tab spaces may result in malformed indent 
while viewing


*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
