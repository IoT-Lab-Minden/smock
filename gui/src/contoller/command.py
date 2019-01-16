from enum import Enum


class Command(Enum):
    """
    Command enum for commands that are received or sent to the micro controller
    """
    PASSWORD = b'p'
    UID = b'u'
    OS = b'o'
    COMPUTER_STATUS = b'l'
    USER_QUANTITY = b'q'
    ENDING_SYMBOL = b'!'
