from command import Command
from computerStatus import ComputerStatus
from message import Message
import ctypes


class TaskManager:
    def __init__(self, user_manager, queue_manager, serial_manager):
        self.__user_manager = user_manager
        self.__queue_manager = queue_manager
        self.__serial_manager = serial_manager
        self.__user32 = ctypes.windll.User32

    def read_tasks(self):
        while True:
            message = self.__queue_manager.read_queue(Command.PASSWORD)
            if message.get_text() != "nothing":
                self.send_password_to_controller(message.get_text())
            message = self.__queue_manager.read_queue(Command.COMPUTER_STATUS)
            if message.get_text() != "nothing":
                pass

    def send_password_to_controller(self, uid):
        if self.__is_locked():
            user = self.__user_manager.get_user_with_uid(uid)
            if user != -1:
                message = Message(Command.PASSWORD, user.get_password())
                self.__serial_manager.write_to_controller(message)
        else:
            print("sende Passwort: 41")
            message = Message(Command.COMPUTER_STATUS, ComputerStatus.UNLOCKED)
            self.__serial_manager.write_to_controller(message)

    def __send_computer_status(self):
        if self.__is_locked():
            message = Message(Command.COMPUTER_STATUS, ComputerStatus.LOCKED)
        else:
            message = Message(Command.COMPUTER_STATUS, ComputerStatus.UNLOCKED)
        self.__serial_manager.write_to_controller(message)

    def __get_current_process(self):
        hwnd = self.__user32.GetForegroundWindow()

        pid = ctypes.c_ulong(0)
        self.__user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))

        window_title = ctypes.create_string_buffer("\x00" * 512)
        self.__user32.GetWindowTextA(hwnd, ctypes.byref(window_title), 512)

        return window_title.value

    def __is_locked(self):
        return self.__get_current_process() == "Windows-Standardsperrbildschirm"
