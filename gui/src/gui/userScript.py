from src.gui.gui import *
from src.gui.clientUserInterface import ClientUserInterface
from threading import Thread
import ctypes

LOCK_WINDOW_NAME_GERMAN = "Windows-Standardsperrbildschirm"
LOCK_WINDOW_NAME_ENGLISH = "Windows Default Lock Screen"


def is_locked(multi_user):
    """
    Works only for Windows.

    Gets the window title that is in the foreground. If it is a single user system then it compares the window title
    with LOCK_WINDOW_NAME_GERMAN and LOCK_WINDOW_NAME_ENGLISH. If the window title is one of these, then the user is in
    locked state. If it is a multi user system, the window title has no title when in foreground, because there is no
    window in foreground at all.

    Args:
        multi_user: boolean value. If true then system is multi user

    Returns:
        returns True if the screen is locked, else returns False
    """
    user32 = ctypes.windll.User32
    window_number = user32.GetForegroundWindow()
    no_window_number = 0

    pid = ctypes.c_ulong()
    user32.GetWindowThreadProcessId(window_number, ctypes.byref(pid))

    buffer_size = 512
    window_title = ctypes.create_string_buffer(buffer_size)
    user32.GetWindowTextA(window_number, ctypes.byref(window_title), buffer_size)

    if multi_user == "True":
        return window_title.value.decode('ASCII') == "" and window_number == no_window_number
    else:
        return window_title.value.decode('ASCII') == LOCK_WINDOW_NAME_ENGLISH or \
               window_title.value.decode('ASCII') == LOCK_WINDOW_NAME_GERMAN


def main():
    """
    This starts starts the program gui of Smock. It connects to the service script and let the user interact with the
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
