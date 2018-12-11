from gui import *
from queueManager import QueueManager


def main():

    serial_queue = QueueManager()
    Gui(serial_queue)


if __name__ == "__main__":
    main()
