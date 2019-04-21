#!/usr/bin/env python

from subprocess import call
import threading
import time

threads = []
class ThreadingCallingCFunction(object):
    """ 
    The run() method will be started and it will run in the background
    until the application exits.
    """

    def __init__(self, interval=1):
        """ Constructor
        :type interval: int
        :param interval: Check interval, in seconds
        """
        self.interval = interval

        thread = threading.Thread(target=self.run, args=())
        threads.append(thread)
        thread.daemon = True                            # Daemonize thread
        thread.start()                                  # Start the execution

    def run(self):
        """ Method that runs forever """
        while True:
            # Do something
            call(["./client"])
            #print('Doing something imporant in the background')
            time.sleep(self.interval)            

def main():
    while True:
        print('Restarting')
        example = ThreadingCallingCFunction()
        time.sleep(100)
        for x in threads:
                x.join()  
        


if __name__ == "__main__":
    main()