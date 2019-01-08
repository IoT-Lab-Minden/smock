import ctypes
from threading import Lock
from clientUserInterface import *
from multiprocessing.connection import Listener
from command import Command

ADDRESS = ('localhost', 6002)
LOCK_WINDOW_NAME_GERMAN = "Windows-Standardsperrbildschirm"
LOCK_WINDOW_NAME_ENGLISH = "Windows Default Lock Screen"


class ListenerUserInterface:
    def __init__(self, user_manager, serial_manager):
        self.__listener = Listener(ADDRESS)
        self.__clients = []
        self.__user_manager = user_manager
        self.__serial_manager = serial_manager
        self.__mutex = Lock()

    def run(self):
        while True:
            client = self.__listener.accept()
            self.__mutex.acquire(True)
            self.__clients.append(client)
            self.__mutex.release()

    def check_for_new_msg(self):
        changed = False
        self.__mutex.acquire(True)
        for c in self.__clients:
            if not c.closed and c.poll():
                try:
                    data = c.recv()
                    if self.do_action(c, data):
                        changed = True
                except ConnectionResetError:
                    print("Client closed connection")
                    self.__clients.remove(c)
            elif c.closed:
                self.__clients.remove(c)
        self.__mutex.release()
        if changed:
            self.send_refresh()

    def do_action(self, c, data):
        if data == Codes.USER_EXISTING:
            name = c.recv()
            c.send(self.__user_manager.check_if_user_exists(name))
        elif data == Codes.GET_USERNAME:
            index = c.recv()
            c.send(self.__user_manager.user_list[int(index)].get_username())
        elif data == Codes.GET_UID:
            index = c.recv()
            c.send(self.__user_manager.user_list[int(index)].get_uid())
        elif data == Codes.SET_NEW_DATA:
            information = c.recv().split(";")
            index = int(information[0])
            self.__user_manager.edit_user(index, information[1], information[2])
            c.send("ready")
            return True
        elif data == Codes.WRITE_TO_CONTROLLER:
            msg = c.recv()
            self.__serial_manager.write_to_controller(msg)
        elif data == Codes.ADD_USER:
            informations = c.recv().split(";")
            self.__user_manager.add_user(informations[0], informations[1], informations[2])
            self.__serial_manager.send_multiple_user_to_controller()
            return True
        elif data == Codes.DELETE_USER:
            self.__user_manager.delete_user(int(c.recv()))
            self.__serial_manager.send_multiple_user_to_controller()
            return True
        elif data == Codes.GET_NUMBER_OF_USERS:
            c.send(len(self.__user_manager.user_list))
        elif data == Codes.FIND_SERIAL_DEVICES:
            found = self.__serial_manager.find_serial_device()
            c.send(Codes.FIND_SERIAL_DEVICES)
            c.send(str(found))
        return False

    def check_locked(self):
        self.__mutex.acquire(True)
        for c in self.__clients:
            if not c.closed:
                try:
                    c.send(str(Command.COMPUTER_STATUS))
                    infos = c.recv().split(";")
                    lock_window_name = infos[0]
                    window_code = infos[1]
                    if self.__user_manager.contains_multiple_user():
                        if lock_window_name == "" and window_code == "0":
                            self.__mutex.release()
                            return True
                    else:
                        if lock_window_name == LOCK_WINDOW_NAME_ENGLISH or lock_window_name == LOCK_WINDOW_NAME_GERMAN:
                            self.__mutex.release()
                            return True
                except ConnectionResetError:
                    self.__clients.remove(c)
            else:
                self.__clients.remove(c)
        self.__mutex.release()
        return False

    def send_refresh(self):
        self.__mutex.acquire(True)
        for c in self.__clients:
            if not c.closed:
                try:
                    c.send(Codes.REFRESH)
                except ConnectionResetError:
                    self.__clients.remove(c)
            else:
                self.__clients.remove(c)
        self.__mutex.release()

    def send_gui_update(self, uid):
        self.__mutex.acquire(True)
        for c in self.__clients:
            if not c.closed:
                try:
                    c.send(Codes.GUI_UPDATE)
                    c.send(uid)
                except ConnectionResetError:
                    self.__clients.remove(c)
            else:
                self.__clients.remove(c)
        self.__mutex.release()
