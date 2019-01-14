from src.contoller.listenerUserInterface import *
from multiprocessing.connection import Client
from threading import Lock
from src.contoller.command import Command
from enum import Enum
import time
from src.gui.gui import Gui


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
    def __init__(self, locked):
        """
        This initializes the Interface between the service script and the user script.

        Args:
            locked: the callback function to check if the user screen is locked
        """
        self.__connection = Client(ADDRESS)
        self.__mutex = Lock()
        self.gui = None
        self.is_locked = locked

    def polling(self):
        """
        Checks if a message is sent by the service script. If a message is in the buffer, it reads the command and
        reacts according to the given command.
        """
        while True:
            time.sleep(0.1)
            refresh = False
            self.__mutex.acquire(True)
            if self.__connection.poll():
                data = self.__connection.recv()
                if data == str(Command.COMPUTER_STATUS):
                    multi_user = self.__connection.recv()
                    ret = str(self.is_locked(multi_user))
                    self.__connection.send(ret)
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

    def check_if_user_exists(self, name):
        """
        Asks the service script if a the user exists

        Args:
            name: the name of the user that is asked for

        Returns:
            True, when user exists, otherwise False
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.USER_EXISTING)
        self.__connection.send(name)
        data = self.__connection.recv() == "True"
        self.__mutex.release()
        return data

    def get_username(self, index):
        """
        Asks the service script for the username of a user and returns the username.

        Args:
            index: index of the user in the list

        Returns:
            username of the user
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.GET_USERNAME)
        self.__connection.send(str(index))
        data = self.__connection.recv()
        self.__mutex.release()
        return data

    def get_uid(self, index):
        """
        Asks the service script for the uid of a user and returns the uid.

        Args:
            index: index of the user in the list

        Returns:
            uid of the user
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.GET_UID)
        self.__connection.send(str(index))
        data = self.__connection.recv()
        self.__mutex.release()
        return data

    def set_new_data(self, index, user_name, user_pwd):
        """
        Sends the new data of a user to the service script. This will edit the user data in the service script.

        Args:
            index:
            user_name:
            user_pwd:
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.SET_NEW_DATA)
        self.__connection.send(str(index) + ";" + user_name + ";" + user_pwd)
        # wait for acknowledge
        self.__connection.recv()
        self.__mutex.release()

    def write_to_controller(self, msg):
        """
        Sends the service script the code to forward a message to the serial device and the message that is send to the
        serial device

        Args:
            msg: message that is send to the service script
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.WRITE_TO_CONTROLLER)
        self.__connection.send(msg)
        self.__mutex.release()

    def add_user(self, username, pwd, uid):
        """
        Sends the service script the information of adding a user and the data of the user.

        Args:
            username: username of the new user
            pwd: password of the username
            uid: uid of the username
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.ADD_USER)
        self.__connection.send(username + ";" + pwd + ";" + uid)
        self.__mutex.release()

    def delete_user(self, index):
        """
        Sends the service script the information to delete a user and also the index of that user.

        Args:
            index: the index of the user in the list
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.DELETE_USER)
        self.__connection.send(str(index))
        self.__mutex.release()

    def get_number_of_users(self):
        """
        Gets the number of users that are in the user list from the service script

        Returns:
            The number of users that exist
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.GET_NUMBER_OF_USERS)
        data = int(self.__connection.recv())
        self.__mutex.release()
        return data

    def find_serial_device(self):
        """
        Sends code of finding a serial device to the service script
        """
        self.__mutex.acquire(True)
        self.__connection.send(Codes.FIND_SERIAL_DEVICES)
        self.__mutex.release()

    def update_gui(self, uid):
        """
        updates the uid label for the gui, when the user is in the add user window.

        Args:
            uid: the uid that replaces the old label
        """
        self.gui.update_uid_label(uid)

