from enum import Enum


class Command(Enum):
    PASSWORD = b"1"
    UID = b"2"
    OS = b"3"
    COMPUTER_STATUS = b"4"
    ENDING_SYMBOL = b"!"
