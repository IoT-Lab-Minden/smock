from gui import *
from queueManager import QueueManager
from serialManager import SerialManager
from userManager import UserManager
from taskManager import TaskManager
from threading import Thread
from message import Message
import platform
from command import Command


def main():

    queue_manager = QueueManager()
    serial_manager = SerialManager(queue_manager)
    user_manager = UserManager()
    task_manager = TaskManager(user_manager, queue_manager, serial_manager)

    message = Message(Command.OS, str.encode(platform.system()[0]))
    t_serial_write = Thread(target=serial_manager.write_to_controller, args=(message,))
    t_serial_write.daemon = True
    t_serial_write.start()

    t_serial_read = Thread(target=serial_manager.fill_queue)
    t_serial_read.daemon = True
    t_serial_read.start()

    t_task_manager = Thread(target=task_manager.read_tasks)
    t_task_manager.daemon = True
    t_task_manager.start()

    Gui(queue_manager, serial_manager, user_manager)
    sys.exit()


if __name__ == "__main__":
    main()
