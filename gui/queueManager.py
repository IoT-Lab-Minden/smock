from queue import Queue


class QueueManager:
    def __init__(self):
        self.serial_queue = Queue()

    def read_queue(self):
        command_complete = False
        while not command_complete:
            byte = self.serial_queue.get()
            if byte is not None:
                pass
