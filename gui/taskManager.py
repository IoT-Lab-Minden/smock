from command import Command
from message import Message


class TaskManager:
    def __init__(self, user_manager, queue_manager, serial_manager):
        self.__user_manager = user_manager
        self.__queue_manager = queue_manager
        self.__serial_manager = serial_manager

    def read_tasks(self):
        while True:
            message = self.__queue_manager.read_queue(Command.PASSWORD)
            if message.get_text() != "nothing":
                self.send_password_to_controller(message.get_text())
            message = self.__queue_manager.read_queue(Command.COMPUTER_STATUS)
            if message.get_text() != "nothing":
                pass

    def send_password_to_controller(self, uid):
        user = self.__user_manager.get_user_with_uid(uid)
        if user != -1:
            message = Message(Command.PASSWORD, user.get_password())
            self.__serial_manager.write_to_controller(message)
