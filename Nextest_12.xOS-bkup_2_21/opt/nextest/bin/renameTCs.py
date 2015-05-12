import sys
import commands
import os
import logging

###########################################################################
## Logging Enabled
###########################################################################
log = logging.getLogger('renameTCs')
hdlr = logging.FileHandler('renameTCs.log','w')
formatter = logging.Formatter('%(asctime)s %(levelname)s:%(message)s')
hdlr.setFormatter(formatter)
log.addHandler(hdlr)
log.setLevel(logging.INFO)
###########################################################################


##
## Expand the Testcase Name to its absolute path
## For e.g. 'eng_test.59562.obp-reg-inv-cdr' to 'eng_test.qms/59562.qms/obp-reg-inv-cdr.qmt'
##
def expandTC(testcase):

  log.info('expandTC function called')

  testcasePath = ""

  # 'eng_test.59562.obp-reg-inv-cdr' ==> ['eng_test', '59562', 'obp-reg-inv-cdr']
  TCBreak = testcase.split(".")
  # Length of TCBreak
  lengthTC = len(TCBreak)

  for i in range(lengthTC):
    if i != lengthTC-1:
      testcasePath += TCBreak[i]+".qms/"
    else:
      testcasePath += TCBreak[i]+".qmt"

  basePath = "/var/opt/nextest/tdb/"
  testcasePath = basePath + testcasePath

  return testcasePath


##
##
## This function renames The qmt file to qmt_bkp.
##
##
def modifyName(testcases, filename):

  log.info('TC ModifyName function is called: Backup is in Progress')

  for testcase in testcases:
 
    ## Remove any unwanted space if exist in the start or end of the test case
    testcase = testcase.strip()

    # Skip the Test Case which is starting with '#' and skip the blank line
    if testcase and testcase[0] == "#" or testcase == "":
      log.info('TC ' +testcase+' skipped')
      continue

    log.info('TC: '+testcase)

    testcasePath = expandTC(testcase)
    log.info('TC expanded: ' + testcase + ' => ' + testcasePath)

    tcExists = os.path.exists(testcasePath)
    if tcExists:
      log.info('TC found. '+testcasePath)
      statusMove = commands.getstatusoutput('mv '+testcasePath+' '+testcasePath+"_bkp")[0]
      # File Moved from orig file to backup.
      if statusMove == 0:
        log.info('TC moved.'+testcasePath+'_bkp')
      # Unable to move file
      else:
        log.error(' -- XX -- Unable to move.' )

      statusChown = commands.getstatusoutput('chown test:users '+testcasePath+"_bkp")[0]
      # File Ownder Changed
      if statusChown == 0:
        log.info(" TC owner changed. ")
      # Unable to change Owner
      else:
        log.error(" -- XX -- Unable to changeOwner: ")

    #File does not Exist
    else:
      log.error(" -- XX -- TC not found: "+testcasePath)

  log.info('ModifyName function exited.')

##
##
## This function renames back The qmt_bkp file to qmt file.
##
##
def modifyNameBack(testcases, filename):

  log.info('TC ModifyNameBack function is called: Restoration is in progress.')

  for testcase in testcases:
    ## Remove any unwanted space if exist in the start or end of the test case
    testcase = testcase.strip()

    # Skip the Test Case which is starting with '#' and skip the blank line
    if testcase and testcase[0] == "#" or testcase == "":
      log.info('TC ' +testcase+' skipped')
      continue

    log.info('TC: '+testcase)

    testcasePath = expandTC(testcase)
    log.info('TC expanded: ' + testcase + ' => ' + testcasePath)

    tcExists = os.path.exists(testcasePath+'_bkp')
    if tcExists:
      log.info('TC Backup found. '+testcasePath+'_bkp')
      statusMove = commands.getstatusoutput('mv '+testcasePath+"_bkp"+' '+testcasePath)[0]
      # File Moved from backup to orig file
      if statusMove == 0:
        log.info(" TC restored " + testcasePath)
      # Unable to move file
      else:
        log.error(" -- XX -- Unable to restore: ")

      statusChown = commands.getstatusoutput('chown test:users '+testcasePath)[0]
      # File Ownder Changed
      if statusChown == 0:
        log.info(" TC owner changed ")
      # Unable to change owner
      else:
        log.error(" -- XX -- Unable to changeOwner: ")

    #Backup File does not Exist
    else:
      log.error( " -- XX -- TC backup not found: "+testcasePath + '_bkp')

  log.info('ModifyNameBack function exited')


##
##
## Help function
##
##
def help():
    log.info('Printted help()')
    print "usage: python file.py file_TCs MoveToBackup/RestoreFromBackup"
    print "python renameTCs.py commonFailureOf71And80.txt MoveToBackup >> This will change the orig qmt file to backup"
    print "python renameTCs.py commonFailureOf71And80.txt RestoreFromBackup >> This will change the backup to orig file"
    print "Note: We will fire this script under tdb directory"
    print "Note: We will take the backup of production_components.qms, negative.qms, eng_test.qms before firing this script."


##
##
## Main Function
##
##
def main():

  log.info('Main function entered')

  if len(sys.argv) != 3:
    help()
    log.error('Command Line Arguments is not equal to 3')
    sys.exit(1)

  file = sys.argv[1]

  # If file does not exist
  if not os.path.exists(file):
    log.error( "File '" + sys.argv[1] + "' not found. ")
    sys.exit(1)

  log.info("File '" + sys.argv[1] + "' found. ")

  log.info("Reading File '" + sys.argv[1] + "' into the main memory")
  f = open(file)
  testcases = f.read().split("\n")
  # delete the last blank line
  del testcases[-1]
  f.close()

  mode = sys.argv[2]
  if mode == "MoveToBackup":
    modifyName(testcases, file)
  elif mode == "RestoreFromBackup":
    modifyNameBack(testcases, file)
  else:
    help()

  log.info("Done Modification. ")
    
##
##
## Call to Main Function
##
##
if __name__ == '__main__':
  main()
