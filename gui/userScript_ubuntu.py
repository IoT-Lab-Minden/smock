import time
from gui import *
from clientUserInterface import ClientUserInterface
from threading import Thread
import ctypes
from listenerUserInterface import ListenerUserInterface
import os


def is_locked(multi_user):
    return os.popen("gdbus call -e -d com.canonical.Unity -o /com/canonical/Unity/Session -m "
                    "com.canonical.Unity.Session.IsLocked | grep -ioP \"(true)|(false)\"").read()

def main():
    """
    This starts starts the program Smock. It is initializing the queueManager, serialManager, userManager, taskManager
    and the gui. It also starts three threads. One to read the messages from the serial input, One to read the tasks
    that are given from the Smock device and the last to text the Smock device about the using OS.
    """
    client_user_interface = ClientUserInterface(is_locked)
    gui = Gui(client_user_interface)

    t_ui_manager = Thread(target=client_user_interface.polling)
    t_ui_manager.daemon = True
    t_ui_manager.start()

    gui.start()
    sys.exit()


if __name__ == "__main__":
    main()
