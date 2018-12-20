from message import Message


class SerialManager:
    def __init__(self, queue_manager):
        self.__queue_manager = queue_manager
        self.__command = ""
        self.__text = ""
        self.__fill_queue()

    def __fill_queue(self):
        counter = 0
        first_byte = True
        while True:
            if counter > 3:
                message = Message(self.__command, self.__text)
                self.__queue_manager.write_queue(message)
                first_byte = True
                self.__command = ""
                self.__text = ""
                break
            else:
                if first_byte:
                    self.__command = 2
                    first_byte = False
                else:
                    self.__text = self.__text + "s"

            counter += 1
