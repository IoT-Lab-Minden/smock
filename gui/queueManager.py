from queue import Queue
from command import Command


class QueueManager:
    def __init__(self):
        self.__password_query = Queue()
        self.__uid_query = Queue()

    def read_queue(self, command_code):
        if command_code == Command.PASSWORD:
            message = self.__get_entry_of_queue(self.__password_query)
        elif command_code == Command.UID:
            message = self.__get_entry_of_queue(self.__uid_query)
        else:
            message = "nothing"

        return message

    def __get_entry_of_queue(self, queue):
        try:
            message = queue.get(timeout=1000)
        except Queue.Empty:
            message = "nothing"
        else:
            queue.task_done()

        return message

    # writes a message into the responsible queue
    def write_queue(self, message):
        if message.get_command_code() == Command.PASSWORD:
            self.__password_query.put(message)
        elif message.get_command_code() == Command.UID:
            self.__uid_query.put(message)
        else:
            pass
