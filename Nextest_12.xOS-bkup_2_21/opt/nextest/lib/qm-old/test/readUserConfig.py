import os
import logging

class UserConfig:
    """Inject all the variables from /.nextest/userConfig.cfg to context dictionary """

    def __init__(self,context):
        userConfig = {}
        self.log = logging.getLogger('nextestlog')
        homepath = os.environ.get('HOME')
        ConfFilePath = homepath+'/.nextest/userConfig.cfg'
        if (os.path.exists(ConfFilePath)==True):
            self.log.debug("userConfig.cfg file exists")
            file = open(ConfFilePath, "r")
            # Verify whether the userConfig.cfg file exists
            try:
                exec file in userConfig
                file.close()
                # Remove the  additional entry that gets inserted in the dictionary
                # http://python.active-venture.com/ref/exec.html
                userConfig.pop('__builtins__') 
                for name in userConfig:
                    context['userConfig.'+str(name)] = str(userConfig[name])
            except Exception, e:
                self.log.error("Error - %s - userConfig.py is not executed" %str(e))
                                

