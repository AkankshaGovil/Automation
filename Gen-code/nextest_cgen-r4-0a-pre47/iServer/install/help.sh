#!/bin/sh


##
## Generic help strings placed here.
##
## SR, 11/08/98.



GENERIC_PACK_HELP="This is a packaging script that provides a way to \
pack programs for distribution for ALOID and NETOID  \

The general procedure is as follows\: \
[1] The program queries for the type of distribution [Aloid/Netoid/Both]\
[2] The program then  queries for the version number for the appropriate\
   distribution, indicating the last revision released \
[3] Once the verion number has been picked, then it asks for additional \
    information, including README file\(s\), and credentials of the releasor \
[4] Once all the information has been gathered, then a package is put together \
"
    
export $GENERIC_PACK_HELP


