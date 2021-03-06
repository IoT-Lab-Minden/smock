import time
from src.contoller.queueManager import QueueManager
from src.contoller.serialManager import SerialManager
from src.contoller.userManager import UserManager
from src.contoller.taskManager import TaskManager
from threading import Thread
from src.contoller.listenerUserInterface import ListenerUserInterface


def main():
    """
    This starts the program Smock. It is initializing the QueueManager, SerialManager, UserManager, TaskManager
    and the ListenerUserInterface. It also starts three threads. One to read the messages from the serial input, One to
    read the tasks that are given from the Smock device and one to open the connection to the user script.
    """
    queue_manager = QueueManager()
    user_manager = UserManager()
    serial_manager = SerialManager(queue_manager, user_manager)
    user_interface = ListenerUserInterface(user_manager, serial_manager)
    task_manager = TaskManager(user_manager, queue_manager, serial_manager, user_interface)

    t_ui_manager = Thread(target=user_interface.run)
    t_ui_manager.daemon = True
    t_ui_manager.start()

    t_serial_read = Thread(target=serial_manager.fill_queue)
    t_serial_read.daemon = True
    t_serial_read.start()

    t_task_manager = Thread(target=task_manager.read_tasks)
    t_task_manager.daemon = True
    t_task_manager.start()

    while True:
        time.sleep(0.1)
        user_interface.check_for_new_msg()


if __name__ == "__main__":
    main()
