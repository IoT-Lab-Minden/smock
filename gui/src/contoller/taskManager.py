from src.contoller.command import Command
from src.contoller.computerStatus import ComputerStatus
from src.contoller.message import Message
import time

NEWLINE = 10


class TaskManager:
    """
    Reads the tasks from the queue_manager and when there are tasks it responds to them and does some stuff, given to
    the situation.
    """
    def __init__(self, user_manager, queue_manager, serial_manager, user_interface):
        """
        Constructor of the TaskManager.

        Args:
            user_manager: A userManager
            queue_manager: A queueManager
            serial_manager: A serialManager
        """
        self.__user_manager = user_manager
        self.__queue_manager = queue_manager
        self.__serial_manager = serial_manager
        self.__user_interface = user_interface

    def read_tasks(self):
        """
        Reads from each queue and when it gets a message it handles the message.
        """
        while True:
            time.sleep(0.01)
            message = self.__queue_manager.read_queue(Command.PASSWORD.value)
            if message.get_text() != "nothing":
                # handle password task
                self.send_password_to_controller(message.get_text())
            message = self.__queue_manager.read_queue(Command.COMPUTER_STATUS.value)
            if message.get_text() != "nothing":
                # handle computer status task
                self.__send_computer_status()
            message = self.__queue_manager.read_queue(Command.UID.value)
            if message.get_text() != "nothing":
                # handle uid tasks
                self.__update_add_window(message.get_text())

    def send_password_to_controller(self, uid):
        """
        Sends the password of the user with the given uid. Is the computer locked, the method processes.
        When the computer is unlocked, the computer status will be send to the serial device. When the system
        is a single user system, only the password is send to the serial device. If it is a system with several users,
        there will be send the username as well.

        Args:
            uid: The uid from the user with the password
        """
        found_user = False
        if self.__is_locked():
            user = self.__user_manager.get_user_with_uid(uid)
            if user != -1:
                password = user.get_password().encode()
                if password[-1] != NEWLINE:
                    password += b'\n'
                if self.__user_manager.contains_multiple_user():
                    username = (user.get_username() + "\n").encode()
                    message_text = password + username
                    message = Message(Command.PASSWORD.value, message_text)
                else:
                    message = Message(Command.PASSWORD.value, password)

                self.__serial_manager.write_to_controller(message)
                found_user = True

        if not found_user:
            message = Message(Command.COMPUTER_STATUS.value, ComputerStatus.UNLOCKED.value)
            self.__serial_manager.write_to_controller(message)

    def __send_computer_status(self):
        """
        Sends the computer status of the computer to the serial device.
        Either locked screen or unlocked.
        """
        if self.__is_locked():
            message = Message(Command.COMPUTER_STATUS.value, ComputerStatus.LOCKED.value)
        else:
            message = Message(Command.COMPUTER_STATUS.value, ComputerStatus.UNLOCKED.value)
        self.__serial_manager.write_to_controller(message)

    def __is_locked(self):
        """
        Returning the computer status

        Returns:
             Returns True if computer is locked, otherwise return False
        """
        return self.__user_interface.check_locked()

    def __update_add_window(self, uid):
        """
        Updates the uid label, when the user wants to add a new user.

        Args:
            uid: the uid that should be updated
        """
        self.__user_interface.send_gui_update(uid)
