from queue import Queue
from message import Message
from command import Command


class QueueManager:
    def __init__(self):
        self.__password_query = Queue()
        self.__uid_query = Queue()

    def read_queue(self, command_code):
        if command_code == Command.PASSWORD.value:
            message = self.__get_entry_of_queue(self.__password_query)
        elif command_code == Command.UID.value:
            message = self.__get_entry_of_queue(self.__uid_query)
        else:
            message = Message("n", "nothing")

        return message

    def __get_entry_of_queue(self, queue):
        if not queue.empty():
            message = queue.get()
        else:
            message = Message("n", "nothing")

        return message

    # writes a message into the responsible queue
    def write_queue(self, message):
        print(Command.UID.value)
        if message.get_command_code() == Command.PASSWORD.value:
            self.__password_query.put(message)
        elif message.get_command_code() == Command.UID.value:
            self.__uid_query.put(message)
        else:
            pass
