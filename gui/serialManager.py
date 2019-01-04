from message import Message
import time
from threading import Lock
import serial
import serial.tools.list_ports
import configparser
from command import Command


class SerialManager:
    def __init__(self, queue_manager):
        self.__queue_manager = queue_manager
        self.__command = b""
        self.__text = b""
        self.__mutex = Lock()
        self.__serial_device = self.__find_serial_device()

    def write_to_controller(self, message):
        self.__mutex.acquire(True)
        print(message.get_command_code())
        print(message.get_text())
        text = message.get_command_code()
        text += message.get_text()
        self.__serial_device.write(text)
        self.__mutex.release()

    def fill_queue(self):
        first_byte = True
        mutex_acquired = False
        while True:
            time.sleep(0.05)
            if not mutex_acquired:
                self.__mutex.acquire(True)
                mutex_acquired = True

            #TODO endlosschleife in read?
            letter = self.__serial_device.read(1)
            if letter != b'':
                if first_byte:
                    self.__command = letter
                    first_byte = False
                elif letter != Command.ENDING_SYMBOL.value:
                    self.__text += letter
                else:
                    message = Message(self.__command, self.__text)
                    print("Controller Message")
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

    def __find_serial_device(self):
        config = configparser.ConfigParser()
        config.read("./config/smock.cfg")
        if 'COMPORT' in config['DEFAULT']:
                ports = serial.tools.list_ports.comports()
                if len(ports) > 0:
                    ser = serial.Serial(config['DEFAULT']['COMPORT'], 115200, timeout=0.1)
                else:
                    ser = "hallo"
        else:
            config['DEFAULT'] = {}
            config['DEFAULT']['COMPORT'] = "COM4"
            with open("./config/smock.cfg", "w") as config_file:
                config.write(config_file)
            ports = serial.tools.list_ports.comports()
            ser = serial.Serial(ports[0].device, 115200, timeout=0.1)
        return ser
