class Message:
    """
    Class for messages.
    A message has a command code and a text.
    """
    def __init__(self,  command_code, text):
        """
        Constructor of the class

        Args:
            command_code: command code of the Message
            text: text of the message
        """
        self.__command_code = command_code
        self.__text = text

    def set_text(self, text):
        """
        Setter method of text attribute
        """
        self.__text = text

    def set_command_code(self, command_code):
        """
        Setter method of command_code attribute
        """
        self.__command_code = command_code

    def get_command_code(self):
        """
        Getter method of command_code attribute
        Returns:
             command_code
        """
        return self.__command_code

    def get_text(self):
        """
        Getter method of text attribute
        Returns:
             text
        """
        return self.__text
