import time
from threading import Lock
import serial
import serial.tools.list_ports
import configparser
from src.contoller.message import Message
import platform
from src.contoller.command import Command
import os

CONFIG_PATH = "../../config/"
CONFIG_FILE = CONFIG_PATH + "smock.cfg"
MULTIPLE_USER = b"2"
SINGLE_USER = b"1"
DELAY_TIME_READING = 0.05   # 50ms
DELAY_TIME_OS = 0.1


class SerialManager:
    """
    This is the class that manages the connection between the computer and the serial device.
    It reads messages from the serial device and stores them in a queue. It also writes messages to the serial device
    """
    def __init__(self, queue_manager, user_manager):
        """
        This is the constructor of the SerialManager. It contains a mutex so it's not possible that multiple threads
        communicate with the serial device. At first the serial device will be initialized, but the port is not known
        yet. The baudrate as well as the timeout is set for the serial device. The timeout is for reading from the
        serial device, when the timout expires, it stops reading from the serial device. The queue manager is necessary
        for storing received messages and the user manager has the information of how many users are using this program
        on the Client/Computer side. This is necessary to know, because the serial device acts differently according to
        the number of users.

        Args:
            queue_manager: This is the queue_manager where the messages are stored, which the SerialManager receives
            user_manager: This is the user_manager, that is sending the information of how many users are active.
        """
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
        """
        Writes a message to the serial device. The message contains a command code and the text that is send. The
        message send to the serial device is a byte array.

        Args:
            message: Message that will be written to the serial device
        """
        if not self.__serial_device.is_open:
            return

        self.__mutex.acquire(True)
        text = message.get_command_code()
        text += message.get_text()
        print(text)
        self.__serial_device.write(text)
        self.__mutex.release()

    def fill_queue(self):
        """
        Reads the bytes that are send from the serial device. It acquires a mutex if it's not acquired yet. Then it
        reads the first letter as byte. This will be saved as command code. After that it reads all the other bytes that
        the serial device sends. When the serial device sends a b"!" it means that this is the end of the message that
        is sent. Then the command code and the text is saved as Message.
        This function should be called by a separate thread. Because its an endless loop.
        """
        first_byte = True
        mutex_acquired = False

        while True:
            while not self.__serial_device.is_open:
                # No device connected so wait for 1 second
                self.find_serial_device()
                time.sleep(1)

            # Delay the call so other threads will be able to acquire the mutex as well
            time.sleep(DELAY_TIME_READING)

            if not mutex_acquired:
                # Acquire mutex
                self.__mutex.acquire(True)
                mutex_acquired = True

            letter = b''
            try:
                # Read one byte from smock device.
                letter = self.__serial_device.read(1)
            except serial.serialutil.SerialException:
                self.__serial_device = serial.Serial()
                print("lost connection")

            # Catch empty messages
            if letter != b'':
                if first_byte:
                    # The first byte of the message is the command_code
                    self.__command = letter
                    first_byte = False
                elif letter != Command.ENDING_SYMBOL.value:
                    # The sending byte is a letter of the message, which will be added to the text.
                    self.__text += letter
                else:
                    # Create the message
                    message = Message(self.__command, self.__text)
                    print(message.get_text())

                    # Release mutex for other threads
                    self.__mutex.release()

                    # Write message into queue
                    self.__queue_manager.write_queue(message)

                    # Refresh used variables for next message
                    mutex_acquired = False
                    first_byte = True
                    self.__command = b""
                    self.__text = b""
            else:
                # Release mutex for other threads when no found message
                self.__mutex.release()
                mutex_acquired = False

    def find_serial_device(self):
        """
        This function searches for a serial device. At first it looks into the config file for a already known COM port.
        If the COM port exists and a device is connected to the COM port, the function connects to the serial device and
        opens the port. If no config is available, the function searches for an available COM port and connects to it.
        After that it writes it in a config file.
        Returns:
            return True when found a serial device, otherwise false
        """
        if self.__serial_device.is_open:
            # device already connected
            return True
            # Gui.notify("Das Gerät ist bereits angeschlossen")

        config = configparser.ConfigParser()
        config.read(CONFIG_FILE)

        if 'COMPORT' in config['DEFAULT']:
            ports = serial.tools.list_ports.comports()
            if len(ports) > 0:
                for port in ports:
                    if port[0] == config['DEFAULT']['COMPORT']:
                        self.__serial_device.port = config['DEFAULT']['COMPORT']
                        self.__serial_device.open()
                        self.send_os_to_controller()

            if not self.__serial_device.is_open:
                return False
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
        return True

    def send_os_to_controller(self):
        """
        Writes the information of the os name to the serial device. Also sends a number to notify the serial device if
        the system is used by one person or multiple users. The serial device only wants the first letter os name.
        So "W" for Windows and "L" for Linux. The function has a little delay, because this function will be called
        immediately after opening the port. This prevents that the message is sent to early and the device doesn't
        read the message.
        """
        time.sleep(DELAY_TIME_OS)
        multi_user_byte = self.__multiple_user_byte()

        first_letter = 0
        message = Message(Command.OS.value, str.encode(platform.system()[first_letter]) + multi_user_byte)
        self.write_to_controller(message)

    def send_multiple_user_to_controller(self):
        """
        Sends the serial device the information, if the system is a system with multiple user or not.
        """
        multi_user_byte = self.__multiple_user_byte()
        message = Message(Command.USER_QUANTITY.value, multi_user_byte)
        self.write_to_controller(message)

    def __multiple_user_byte(self):
        """
        Returns the byte MULTIPLE_USER if the system has multiple user. Else it returns SINGLE_USER

        Returns:
            MULTIPLE_USER if system has more than one user. Else SINGLE_USER
        """
        if self.__user_manager.contains_multiple_user():
            return MULTIPLE_USER
        else:
            return SINGLE_USER
