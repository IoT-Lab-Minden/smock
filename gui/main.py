from gui import *
from queueManager import QueueManager
from serialManager import SerialManager


def main():

    queue_manager = QueueManager()
    SerialManager(queue_manager)
    Gui(queue_manager)


if __name__ == "__main__":
    main()
