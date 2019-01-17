import time
from src.gui.gui import *
from src.gui.clientUserInterface import ClientUserInterface
from threading import Thread
import os


def is_locked(multi_user):
    """
    Works only for Ubuntu.
    Asks if the screen is locked. Returns the locked_status

    Returns:
        returns True if the screen is locked, else returns False
    """
    locked_string = os.popen("gdbus call -e -d com.canonical.Unity -o /com/canonical/Unity/Session -m "
                             "com.canonical.Unity.Session.IsLocked | grep -ioP \"(true)|(false)\"").read()
    print(locked_string)
    time.sleep(0.1)
    if "false\n" == locked_string:
        return False
    else:
        return True


def main():
    """
    This starts the program gui of Smock. It connects to the service script and let the user interact with the
    service script through the gui.
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
