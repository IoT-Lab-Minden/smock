class Message:
    def __init__(self,  command_code, text):
        self.__command_code = command_code
        self.__text = text

    def set_text(self, text):
        self.__text = text

    def set_command_code(self, command_code):
        self.__command_code = command_code

    def get_command_code(self):
        return self.__command_code

    def get_text(self):
        return self.__text
