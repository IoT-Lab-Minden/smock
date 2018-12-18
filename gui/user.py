class User:

    def __init__(self, username, password, uid):
        self.__username = username
        self.__password = password
        self.__uid = uid

    def get_username(self):
        return self.__username

    def get_password(self):
        return self.__password

    def get_uid(self):
        return self.__uid

    def set_username(self, new_username):
        self.__username = new_username

    def set_password(self, new_password):
        self.__password = new_password

    def print(self):
        print("Username: " + self.__username)
        print("Passwort: " + self.__password)
        print("uid: " + self.__uid)
