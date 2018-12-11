import time


class SerialManager:
    def __init__(self, serial_queue):
        self.__serial_queue = serial_queue

    def fill_queue(self):
        counter = 0
        while True:
            counter += 1
            if counter > 10:
                counter = 0
                self.__serial_queue.write_queue("!")
            else:
                self.__serial_queue.write_queue("s")
            time.sleep(1)
