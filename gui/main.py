from gui import *
from queueManager import QueueManager
from serialManager import SerialManager
from userManager import UserManager
from taskManager import TaskManager
from threading import Thread


def main():
    """
    This starts starts the program Smock. It is initializing the queueManager, serialManager, userManager, taskManager
    and the gui. It also starts three threads. One to read the messages from the serial input, One to read the tasks
    that are given from the Smock device and the last to text the Smock device about the using OS.
    """
    queue_manager = QueueManager()
    serial_manager = SerialManager(queue_manager)
    user_manager = UserManager()
    task_manager = TaskManager(user_manager, queue_manager, serial_manager)
    gui = Gui(queue_manager, serial_manager, user_manager, task_manager)

    t_serial_read = Thread(target=serial_manager.fill_queue)
    t_serial_read.daemon = True
    t_serial_read.start()

    t_task_manager = Thread(target=task_manager.read_tasks)
    t_task_manager.daemon = True
    t_task_manager.start()

    gui.start()
    sys.exit()


if __name__ == "__main__":
    main()
