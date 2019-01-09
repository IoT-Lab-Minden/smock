from listenerUserInterface import *
from multiprocessing.connection import Client
from threading import Lock
from command import Command
from enum import Enum
import ctypes
import time
from gui import Gui


class Codes(Enum):
    """
    Command enum for commands that are received or sent to the micro controller
    """
    GET_USERNAME = 'gu'
    USER_EXISTING = 'ue'
    GET_UID = 'uid'
    SET_NEW_DATA = 'snd'
    WRITE_TO_CONTROLLER = 'wtc'
    ADD_USER = 'au'
    DELETE_USER = 'du'
    GET_NUMBER_OF_USERS = 'gnou'
    FIND_SERIAL_DEVICES = 'fsd'
    GUI_UPDATE = 'update'
    REFRESH = 'r'


class ClientUserInterface:
    def __init__(self):
        self.__connection = Client(ADDRESS)
        self.__user32 = ctypes.windll.User32
        self.__mutex = Lock()
        self.gui = None

    def polling(self):
        while True:
            time.sleep(0.1)
            refresh = False
            self.__mutex.acquire(True)
            if self.__connection.poll():
                data = self.__connection.recv()
                if data == str(Command.COMPUTER_STATUS):
                    multit_user = self.__connection.recv()
                    self.__connection.send(str(self.is_locked(multit_user)))
                elif data == Codes.GUI_UPDATE:
                    uid = self.__connection.recv()
                    self.update_gui(uid)
                elif data == Codes.FIND_SERIAL_DEVICES:
                    if self.__connection.recv() == "True":
                        Gui.notify("Das Gerät ist bereits angeschlossen")
                    else:
                        Gui.notify("Es wurde kein Smock Gerät gefunden.\n"
                                   "Vergewissern Sie sich, dass das Gerät angeschlossen ist.\n")
                elif data == Codes.REFRESH:
                    refresh = True
            self.__mutex.release()
            if refresh:
                self.gui.refresh_list()

    def is_locked(self, multi_user):
        hwnd = self.__user32.GetForegroundWindow()

        pid = ctypes.c_ulong(0)
        self.__user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))

        window_title = ctypes.create_string_buffer(512)
        self.__user32.GetWindowTextA(hwnd, ctypes.byref(window_title), 512)

        if multi_user == "True":
            return window_title.value.decode('ASCII') == "" and hwnd == 0
        else:
            return window_title.value.decode('ASCII') == LOCK_WINDOW_NAME_ENGLISH or \
                   window_title.value.decode('ASCII') == LOCK_WINDOW_NAME_GERMAN

    def check_if_user_exists(self, name):
        self.__mutex.acquire(True)
        # self.__user_manager.check_if_user_exists(name)
        self.__connection.send(Codes.USER_EXISTING)
        self.__connection.send(name)
        data = self.__connection.recv() == "True"
        self.__mutex.release()
        return data

    def get_username(self, index):
        self.__mutex.acquire(True)
        # self.__user_manager.user_list[index].get_username()
        self.__connection.send(Codes.GET_USERNAME)
        self.__connection.send(str(index))
        data = self.__connection.recv()
        self.__mutex.release()
        return data

    def get_uid(self, index):
        self.__mutex.acquire(True)
        # self.__user_manager.user_list[index].get_uid()
        self.__connection.send(Codes.GET_UID)
        self.__connection.send(str(index))
        data = self.__connection.recv()
        self.__mutex.release()
        return data

    def set_new_data(self, index, user_name, user_pwd):
        self.__mutex.acquire(True)
        # self.__user_manager.user_list[index].set_username(name)
        # self.__user_manager.user_list[index].set_password(pwd)
        self.__connection.send(Codes.SET_NEW_DATA)
        self.__connection.send(str(index) + ";" + user_name + ";" + user_pwd)
        # wait for acknowledge
        self.__connection.recv()
        self.__mutex.release()

    def write_to_controller(self, msg):
        self.__mutex.acquire(True)
        # self.__serial_manager.write_to_controller(message)
        self.__connection.send(Codes.WRITE_TO_CONTROLLER)
        self.__connection.send(msg)
        self.__mutex.release()

    def add_user(self, username, pwd, uid):
        self.__mutex.acquire(True)
        # self.__user_manager.add_user(textfield_username.get(), textfield_password.get(),
        #                                          self.label_near_uid.cget("text"))
        # self.__serial_manager.send_multiple_user_to_controller()
        self.__connection.send(Codes.ADD_USER)
        self.__connection.send(username + ";" + pwd + ";" + uid)
        self.__mutex.release()

    def delete_user(self, index):
        self.__mutex.acquire(True)
        # self.__user_manager.delete_user(self.list.index(ACTIVE))
        self.__connection.send(Codes.DELETE_USER)
        self.__connection.send(str(index))
        self.__mutex.release()

    def get_number_of_users(self):
        self.__mutex.acquire(True)
        # len(self.__user_manager.user_list)
        self.__connection.send(Codes.GET_NUMBER_OF_USERS)
        data = int(self.__connection.recv())
        self.__mutex.release()
        return data

    def find_serial_device(self):
        self.__mutex.acquire(True)
        # self.__serial_manager.find_serial_device()
        self.__connection.send(Codes.FIND_SERIAL_DEVICES)
        self.__mutex.release()

    def update_gui(self, uid):
        self.gui.update_uid_label(uid)

