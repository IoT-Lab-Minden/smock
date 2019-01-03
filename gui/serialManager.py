from message import Message
import time
from threading import Lock
import serial
import serial.tools.list_ports
import configparser


class SerialManager:
    def __init__(self, queue_manager):
        self.__queue_manager = queue_manager
        self.__command = ""
        self.__text = ""
        self.__mutex = Lock()
        self.__serial_device = self.__find_serial_device()
        print(self.__serial_device.is_open)

    def write_to_controller(self, message):
        self.__mutex.acquire(True)
        print("Command Code: " + str(message.get_command_code()))
        print("Message: " + message.get_text())
        self.__mutex.release()

    def fill_queue(self):
        first_byte = True
        mutex_acquired = False
        while True:
            if not mutex_acquired:
                self.__mutex.acquire(True)
                mutex_acquired = True

            letter = self.__serial_device.read()
            if first_byte:
                self.__command = letter
                first_byte = False
            elif letter != "!":
                self.__text = self.__text + str(letter)
            else:
                message = Message(self.__command, self.__text)
                self.__mutex.release()
                mutex_acquired = False
                self.__queue_manager.write_queue(message)
                first_byte = True
                self.__command = ""
                self.__text = ""

    def __find_serial_device(self):
        config = configparser.ConfigParser()
        config.read("./config/smock.cfg")
        if 'COMPORT' in config['DEFAULT']:
            ser = serial.Serial(config['DEFAULT']['COMPORT'], 115200)
        else:
            config['DEFAULT'] = {}
            config['DEFAULT']['COMPORT'] = "COM4"
            with open("./config/smock.cfg", "w") as config_file:
                config.write(config_file)
            ports = serial.tools.list_ports.comports()
            ser = serial.Serial(ports[0].device, 115200)
        return ser


