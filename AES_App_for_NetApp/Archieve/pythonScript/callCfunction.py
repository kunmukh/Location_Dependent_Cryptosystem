#!/usr/bin/env python

from subprocess import call
import threading
import time

class ThreadingCallingCFunction(object):
    """ 
    The run() method will be started and it will run in the background
    until the application exits.
    """

    def __init__(self, interval=40):
        """ Constructor
        :type interval: int
        :param interval: Check interval, in seconds
        """
        self.interval = interval

        thread = threading.Thread(target=self.run, args=())
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
		example = ThreadingCallingCFunction()
		time.sleep(30)	
		print('Exiting')


if __name__ == "__main__":
	main()