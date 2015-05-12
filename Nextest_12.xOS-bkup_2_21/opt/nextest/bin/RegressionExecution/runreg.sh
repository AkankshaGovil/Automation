sh runAuto.sh 2>&1 | tee -a RegressionAuto.log
sh runNeg.sh 2>&1 | tee -a RegressionNeg.log
sh runEng.sh 2>&1 | tee -a RegressionEng.log
