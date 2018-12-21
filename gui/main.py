from gui import *
from queueManager import QueueManager
from serialManager import SerialManager
import threading


def main():

    queue_manager = QueueManager()
    t_serial_manager = threading.Thread(target=SerialManager, args=(queue_manager,))
    t_serial_manager.start()
    Gui(queue_manager)


if __name__ == "__main__":
    main()
