import time
from threading import Lock
import serial
import serial.tools.list_ports
import configparser
from gui import Gui
from message import Message
import platform
from command import Command
import os
from userManager import UserManager

CONFIG_PATH = "./config/"
CONFIG_FILE = CONFIG_PATH + "smock.cfg"


class SerialManager:
    def __init__(self, queue_manager, user_manager):
        self.__queue_manager = queue_manager
        self.__user_manager = user_manager
        self.__command = b""
        self.__text = b""
        self.__mutex = Lock()
        self.__serial_device = serial.Serial()
        self.__serial_device.baudrate = 115200
        self.__serial_device.timeout = 0.1
        self.find_serial_device()

    def write_to_controller(self, message):

        while not self.__serial_device.is_open:
            time.sleep(1)

        self.__mutex.acquire(True)
        text = message.get_command_code()
        text += message.get_text()
        print(text)
        self.__serial_device.write(text)
        self.__mutex.release()

    def fill_queue(self):
        first_byte = True
        mutex_acquired = False

        while True:
            while not self.__serial_device.is_open:
                self.find_serial_device()
                time.sleep(1)

            time.sleep(0.05)
            if not mutex_acquired:
                self.__mutex.acquire(True)
                mutex_acquired = True

            letter = b''
            try:
                letter = self.__serial_device.read(1)
            except serial.serialutil.SerialException:
                self.__serial_device = serial.Serial()
                print("lost connection")

            if letter != b'':
                if first_byte:
                    self.__command = letter
                    first_byte = False
                elif letter != Command.ENDING_SYMBOL.value:
                    self.__text += letter
                else:
                    message = Message(self.__command, self.__text)
                    print(message.get_text())
                    self.__mutex.release()
                    mutex_acquired = False
                    self.__queue_manager.write_queue(message)
                    first_byte = True
                    self.__command = b""
                    self.__text = b""
            else:
                self.__mutex.release()
                mutex_acquired = False

    def find_serial_device(self):
        config = configparser.ConfigParser()
        config.read(CONFIG_FILE)
        if self.__serial_device.is_open:
            return True
            # Gui.notify("Das Gerät ist bereits angeschlossen")
        elif 'COMPORT' in config['DEFAULT']:
            ports = serial.tools.list_ports.comports()
            if len(ports) > 0:
                for port in ports:
                    if port[0] == config['DEFAULT']['COMPORT']:
                        self.__serial_device.port = config['DEFAULT']['COMPORT']
                        self.__serial_device.open()
                        self.send_os_to_controller()

            if not self.__serial_device.is_open:
                return False
            #     Gui.notify("Es wurde kein Smock Gerät gefunden.\n"
            #                "Vergewissern Sie sich, dass das Gerät angeschlossen ist\n"
            #                "und der COM Port in der Config richtig ist.")
        else:
            ports = serial.tools.list_ports.comports()
            if len(ports) > 0:
                self.__serial_device.port = ports[0].device
                self.__serial_device.open()
                self.send_os_to_controller()
                config['DEFAULT'] = {}
                config['DEFAULT']['COMPORT'] = self.__serial_device.port
                if not os.path.isdir(CONFIG_PATH):
                    os.mkdir(CONFIG_PATH)
                with open(CONFIG_FILE, "w") as config_file:
                    config.write(config_file)
            else:
                return False
                # Gui.notify("Es wurde kein Smock Gerät gefunden.\n"
                #            "Vergewissern Sie sich, dass das Gerät angeschlossen ist.\n")
        return True

    def send_os_to_controller(self):
        time.sleep(0.1)
        multi_user_byte = self.__multiple_user_byte()

        message = Message(Command.OS.value, str.encode(platform.system()[0]) + multi_user_byte)
        self.write_to_controller(message)

    def send_multiple_user_to_controller(self):
        multi_user_byte = self.__multiple_user_byte()
        message = Message(Command.USER_QUANTITY.value, multi_user_byte)
        self.write_to_controller(message)

    def __multiple_user_byte(self):
        if self.__user_manager.contains_multiple_user():
            return b"2"
        else:
            return b"1"
