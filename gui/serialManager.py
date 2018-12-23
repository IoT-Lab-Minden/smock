from message import Message
import time
from threading import Lock


class SerialManager:
    def __init__(self, queue_manager):
        self.__queue_manager = queue_manager
        self.__command = ""
        self.__text = ""
        self.__mutex = Lock()

    def write_to_controller(self, message):
        self.__mutex.acquire(1)
        print("Command Code: " + str(message.get_command_code()))
        print("Message: " + message.get_text())
        self.__mutex.release()

    def fill_queue(self):
        counter = 0
        first_byte = True
        mutex_acquired = False
        while True:
            if not mutex_acquired:
                self.__mutex.acquire(1)
                mutex_acquired = True
            if counter > 3:
                message = Message(self.__command, self.__text)
                self.__mutex.release()
                mutex_acquired = False
                self.__queue_manager.write_queue(message)
                first_byte = True
                self.__command = ""
                self.__text = ""
                counter = 0
            else:
                if first_byte:
                    self.__command = 2
                    first_byte = False
                else:
                    self.__text = self.__text + "s"
                time.sleep(1)

            counter += 1
