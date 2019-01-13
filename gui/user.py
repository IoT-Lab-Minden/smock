class User:
    """
    Class for user.
    A user has a username, password and uid.
    """
    def __init__(self, username, password, uid):
        """
        Constructor for the class

        Args:
            username: username of user
            password: password of user
            uid: uid of user
        """
        self.__username = username
        self.__password = password
        self.__uid = uid

    def get_username(self):
        """
        Gets the username

        Returns:
            username
        """
        return self.__username

    def get_password(self):
        """
        Gets the password

        Returns:
            password
        """
        return self.__password

    def get_uid(self):
        """
        Gets the uid

        Returns:
            uid
        """
        return self.__uid

    def set_username(self, new_username):
        """
        Sets a new username for the user

        Args:
            new_username: new username of user
        """
        self.__username = new_username

    def set_password(self, new_password):
        """
        Sets a new password for the user

        Args:
            new_password: new password of user
        """
        self.__password = new_password
