from enum import Enum


class ComputerStatus(Enum):
    """
    Computer status enum
    """
    LOCKED = b"0"
    UNLOCKED = b"1"
