from gui import *
from queueManager import QueueManager
from serialManager import SerialManager
from threading import Thread
from message import Message


def main():

    queue_manager = QueueManager()
    serial_manager = SerialManager(queue_manager)

    t_serial_read = Thread(target=serial_manager.fill_queue)
    t_serial_read.daemon = True
    t_serial_read.start()

    message = Message(1, "PASSWORT")

    t_serial_write = Thread(target=serial_manager.write_to_controller, args=(message,))
    t_serial_write.daemon = True
    t_serial_write.start()

    Gui(queue_manager, serial_manager)
    sys.exit()


if __name__ == "__main__":
    main()
