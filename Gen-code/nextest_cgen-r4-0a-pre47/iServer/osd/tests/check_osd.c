#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <check.h>
#include "nxosd.h"
Suite *s;
TCase *tc, *tc2, *tc3, *tc4, *tc5, *tc6;
SRunner *sr;


START_TEST(test_valid_hostname)
{
  char* hostname="www.induslogic.com";
  char buffer[8192];
  int  erno=0;
  struct hostent host;
  struct hostent* result;
  
  fprintf(stdout, "\nChecking for hostname %s\n", hostname);
  result = nx_gethostbyname_r( hostname, &host, buffer, 8192,&erno); 
  fail_unless(result == &host,"Couldn't get host address");
  
}
END_TEST

START_TEST(test_valid_rpcname)
{
  char* name="nfs";
  char buffer[8192];
  struct rpcent rpc;
  struct rpcent* result;
 
   
  result = nx_getrpcbyname_r( name, &rpc, buffer, 8192); 
 
  fail_unless(result == &rpc,"RPC server lookup failed for nfs");
  
}
END_TEST
/*
  nx_getrpcbyname_r should return NULL in case a invalid rpc service name
  is specified
 */
START_TEST(test_invalid_rpcname)
{
  char* name="";
  char buffer[8192];
  struct rpcent rpc;
  struct rpcent* result;
 
   
  result = nx_getrpcbyname_r( name, &rpc, buffer, 8192); 
  fail_unless(result == NULL,"Unexpected behaviour by nx_getrpcbyname_r for invalid rpc name");
  
}
END_TEST

START_TEST(test_invalid_hostname)
{
  char* hostname="nextthree";
  char buffer[1024];
  int  erno;
  struct hostent host;
  struct hostent* result;
   
 
  fprintf(stdout, "\n Checking for hostname %s\n", hostname);
  result = nx_gethostbyname_r( hostname, &host, buffer, 1024,&erno); 
  fprintf(stdout, "errno string %i\n", erno);

  if (erno == HOST_NOT_FOUND)
    {
    fprintf(stdout, "errno returns 'host not found'\n");
      }

  fail_unless((result == NULL && erno == HOST_NOT_FOUND),NULL);

  
}
END_TEST

START_TEST (test_strlcpy_full_copy)
{
  char *src ="source string";
  char dest[20];
  size_t ret; 
  
  ret=nx_strlcpy( dest, src, sizeof(dest));
  
  fail_unless( ret == strlen(src) && strcmp( src, dest) ==0, " String copy failed");

}
END_TEST

START_TEST (test_strlcpy_truncated_copy)
{
  char *src ="source string";
  char *expres = "source";
  char dest[7];
  size_t ret; 
  
  fprintf( stdout,"Copying source string \"%s \"to destination with insufficient buffer\n", src);
  ret=nx_strlcpy( dest, src, sizeof(dest));
  fprintf( stdout, "Expected result %s\n", expres);
  fprintf( stdout, "Result %s\n", dest);

  fail_unless( ret == strlen(src) && dest[sizeof(dest)-1] == '\0'  && strcmp( expres, dest) ==0, " Truncated string copy failed");

}
END_TEST

START_TEST (test_strlcat_full_concat)
{
  char *src ="source string";
  char *expres = "destination string source string";
  char dest[40];
  size_t ret, destlen, srclen;
 
  
  strncpy( dest, "destination string ", sizeof(dest));
  destlen = strlen(dest);
  srclen  = strlen(src);
  
  ret=nx_strlcat( dest, src, sizeof(dest));

 fail_unless( ret == ( srclen + destlen ) && ret == strlen(dest)  && strcmp( expres, dest) ==0, " String concat failed");

}
END_TEST


START_TEST (test_strlcat_truncated_concat)
{
  char *src ="source string";
  char *expres = "destination string source";
  char dest[26];
  size_t ret, destlen, srclen;
 
  
  strncpy( dest, "destination string ", sizeof(dest));
  destlen = strlen(dest);
  srclen  = strlen(src);
  
  ret=nx_strlcat( dest, src, sizeof(dest));

 fail_unless( ret == ( srclen + destlen ) && dest[sizeof(dest)-1] == '\0'  && strcmp( expres, dest) ==0, " Truncated string concat failed");

}
END_TEST

START_TEST( test_nx_sig2str_valid_signum)
{
  int ret;
  char sigdesc[30];
   
  ret = nx_sig2str( SIGHUP, sigdesc, sizeof(sigdesc));

   // string returned is implementation defined and also depends on the 
  // locale for solaris
  fail_unless( ret ==0 && strlen(sigdesc) != 0, "Signal description lookup failed for SIGHUP") ;
 
}
END_TEST

START_TEST( test_nx_sig2str_invalid_signum)
{
  int ret;
  char sigdesc[50];
 // choose an arbitary high value of signum
  ret = nx_sig2str( 200, sigdesc, sizeof(sigdesc));
  fail_unless( ret ==-1, "Incorrect behavior for invalid signal number") ;
 
}
END_TEST

START_TEST(test_nx_gethrtime)
{
  printf("ret val %lld \n", nx_gethrtime());
  printf("ret val %lld \n", nx_gethrtime());
  printf("ret val %lld \n", nx_gethrtime());
}
END_TEST
/*
  nx_mkdirp should return 0 if the path specified as argument
  is sucessfully created 
*/
START_TEST(test_nx_mkdirp)
{
  int retval;
  int rmfail=0;

  retval= nx_mkdirp("/tmp/dir1/dir2/dir3",0777);

  if ( !(rmdir("/tmp/dir1/dir2/dir3") == 0
	 &&  rmdir("/tmp/dir1/dir2") == 0
	 &&  rmdir("/tmp/dir1") == 0)
       )
    {
      rmfail=1;
    }


  fail_unless(retval==0 && rmfail == 0 ,"Making directory path failed");
}

END_TEST
static void run_suite()
{
  s = suite_create ("OSD");
  tc = tcase_create ("Re-entrant gethostbyname");
  tc2 = tcase_create("Strlcpy");
  tc3 = tcase_create("Strlcat");
  tc4 = tcase_create("Sig2str");
  tc5 = tcase_create ("Re-entrant getrpcbyname");
  tc6 = tcase_create ("Mkdirp");


  sr = srunner_create (s);
  suite_add_tcase(s, tc);
  suite_add_tcase(s, tc2);
  suite_add_tcase(s, tc3);
  suite_add_tcase(s, tc4);
  suite_add_tcase(s, tc5);
  suite_add_tcase(s, tc6);

  tcase_add_test (tc, test_valid_hostname);
  tcase_add_test (tc, test_invalid_hostname);


 
  tcase_add_test (tc2, test_strlcpy_full_copy);
  tcase_add_test (tc2, test_strlcpy_truncated_copy); 
 
  tcase_add_test (tc3, test_strlcat_truncated_concat); 
  tcase_add_test (tc3, test_strlcat_full_concat); 

  tcase_add_test (tc4, test_nx_sig2str_valid_signum); 
  tcase_add_test (tc4, test_nx_sig2str_invalid_signum); 

  tcase_add_test (tc5, test_valid_rpcname);
  tcase_add_test (tc5, test_invalid_rpcname);

  tcase_add_test (tc6, test_nx_mkdirp);

  srunner_run_all(sr, CK_VERBOSE);
  srunner_free(sr);
  suite_free(s);
  }

int main(void)
{
  run_suite();
  return 0; 
}

