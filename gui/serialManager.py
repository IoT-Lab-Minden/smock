from message import Message


class SerialManager:
    def __init__(self, queue_manager):
        self.__queue_manager = queue_manager

    def fill_queue(self):
        counter = 0
        while True:
            if counter > 2:
                self.__queue_manager.write_queue("!")
                break
            else:
                self.__queue_manager.write_queue("s")
            counter += 1
