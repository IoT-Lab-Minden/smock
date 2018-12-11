from gui import *
from queueManager import QueueManager
from serialManager import SerialManager


def main():

    serial_queue = QueueManager()
    serial_manager = SerialManager(serial_queue)
    serial_manager.fill_queue()
    Gui(serial_queue)


if __name__ == "__main__":
    main()
