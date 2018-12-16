import time


class SerialManager:
    def __init__(self, serial_queue):
        self.__serial_queue = serial_queue

    def fill_queue(self):
        counter = 0
        while True:
            if counter > 2:
                self.__serial_queue.write_queue("!")
                break
            else:
                self.__serial_queue.write_queue("s")
            counter += 1
