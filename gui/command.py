from enum import Enum


class Command(Enum):
    """
    Command enum for commands that are received or sent to the micro controller
    """
    PASSWORD = b'1'
    UID = b'2'
    OS = b'3'
    COMPUTER_STATUS = b'4'
    USER_QUANTITY = b'5'
    ENDING_SYMBOL = b'!'
