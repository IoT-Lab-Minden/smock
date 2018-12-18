from queue import Queue
from message import Message


class QueueManager:
    def __init__(self):
        self.serial_queue = Queue()

    def read_queue(self):
        command_complete = False
        first_char = True
        tmp_text = ""
        message = Message("U", "nothing")
        while not self.serial_queue.empty() and not command_complete:
            # Reads from queue until no more bytes are expected
            byte = self.serial_queue.get()
            # Got a byte from the queue
            if first_char:
                first_char = False
                message.set_command_code(byte)
            elif byte is not "!":
                # byte is a char that is part of the message
                tmp_text = tmp_text + byte
            else:
                # byte is a terminal char that signals end of message
                message.set_text(tmp_text)
                command_complete = True

        return message

    # writes a byte into the queue
    def write_queue(self, byte):
        self.serial_queue.put(byte)
