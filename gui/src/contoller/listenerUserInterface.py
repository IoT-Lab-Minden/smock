from src.gui.clientUserInterface import *
from multiprocessing.connection import Listener
from src.contoller.command import Command

ADDRESS = ('localhost', 6000)


class ListenerUserInterface:
    def __init__(self, user_manager, serial_manager):
        """
        Initialize the service script, that runs in the background of the system. It opens a port on the localhost, that
        the user script can connect to.

        Args:
            user_manager: A user manager, that manages the user lists
            serial_manager: A serial manager that communicates to the serial device
        """
        self.__listener = Listener(ADDRESS)
        self.__clients = []
        self.__user_manager = user_manager
        self.__serial_manager = serial_manager
        self.__mutex = Lock()

    def run(self):
        """
        Waits for a user script to connect to the service script. When the service script acknowledges a client, it adds
        the client to the list and waits for another client.
        """
        while True:
            client = self.__listener.accept()
            self.__mutex.acquire(True)
            self.__clients.append(client)
            self.__mutex.release()

    def check_for_new_msg(self):
        """
        Checks if a message is sent by the client. If the client is not found any more, the service system closes the
        connection to the client. It runs a command when a client sends a command to the service script. If the service
        script changes the user list, it sends the client to refresh the user list.
        """
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
        """
        This function compares the data that are send by the client and depending on what data is sent by the client, it
        acts according to the code.

        Args:
            c: client that send the message.
            data: Data that are send by the client

        Returns:
        """
        if data == Codes.USER_EXISTING:
            name = c.recv()
            user_exists = self.__user_manager.check_if_user_exists(name)
            c.send(user_exists)
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
            information = c.recv().split(";")
            self.__user_manager.add_user(information[0], information[1], information[2])
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
        """
        Asks the client if the computer is in a locked status or unlocked status.
        """
        self.__mutex.acquire(True)
        for c in self.__clients:
            if not c.closed:
                try:
                    c.send(str(Command.COMPUTER_STATUS))
                    c.send(str(self.__user_manager.contains_multiple_user()))
                    value = c.recv()
                    if value == "False":
                        self.__mutex.release()
                        return False
                except ConnectionResetError:
                    self.__clients.remove(c)
            else:
                self.__clients.remove(c)
        self.__mutex.release()
        return True

    def send_refresh(self):
        """
        Sends the client the information that he has to refresh its user list in the gui.
        """
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
        """
        Sends the uid to the client so it can update the uid label.

        Args:
            uid: uid that is send to the client
        """
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
