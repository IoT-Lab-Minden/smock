from queue import Queue
from message import Message
from command import Command


class QueueManager:
    """
    Puts incoming messages into the correct queue and reads the queues to return the messages.
    """
    def __init__(self):
        """
        Construcor of the class. Initialize the different queues.
        """
        self.__password_query = Queue()
        self.__uid_query = Queue()
        self.__computer_status_query = Queue()

    def read_queue(self, command_code):
        """
        Reads a message from a queue. The queue will be selected by the command_code

        Args:
            command_code:

        Returns:
            A message if the queue has messages, otherwise returns a message with the text "nothing"

        """
        if command_code == Command.PASSWORD.value:
            message = self.__get_entry_of_queue(self.__password_query)
        elif command_code == Command.UID.value:
            message = self.__get_entry_of_queue(self.__uid_query)
        elif command_code == Command.COMPUTER_STATUS.value:
            message = self.__get_entry_of_queue(self.__computer_status_query)
        else:
            message = Message("n", "nothing")

        return message

    def __get_entry_of_queue(self, queue):
        """
        Reads from the queue that is used as param.

        Args:
            queue: queue that should be read from

        Returns:
            Returns a message if found, otherwise returns a message with the text "nothing"
        """
        # get the next entry of the queue that is given as param
        if not queue.empty():
            message = queue.get()
        else:
            message = Message("n", "nothing")

        return message

    def write_queue(self, message):
        """
        Stores a message into the correct queue. The command code of the Message decides in which
        queue the message will be put in.
*
        Args:
            message: Message that is stored into a queue
        """
        if message.get_command_code() == Command.PASSWORD.value:
            self.__password_query.put(message)
        elif message.get_command_code() == Command.UID.value:
            self.__uid_query.put(message)
        elif message.get_command_code() == Command.COMPUTER_STATUS.value:
            self.__computer_status_query.put(message)
