from gui import *
from queueManager import QueueManager
from serialManager import SerialManager


def main():

    queue_manager = QueueManager()
    serial_manager = SerialManager(queue_manager)
    serial_manager.fill_queue()
    Gui(queue_manager)


if __name__ == "__main__":
    main()
