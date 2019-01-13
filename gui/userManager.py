from user import User
from os.path import isfile, join
from os import listdir
from os import remove
import os

USERS_PATH = "./users/"


class UserManager:
    """
    Handles the user in a user list. Also loads user from the user files
    """
    def __init__(self):
        """
        Constructor for the class
        Initializing the list.
        """
        self.user_list = []
        self.__load_users_from_files()

    def __load_users_from_files(self):
        """
        Loads the users from all files that are in the directory ./users
        """
        # Get all the filenames of the files in the local_path directory
        if not os.path.isdir(USERS_PATH):
            os.mkdir(USERS_PATH)
        local_files = [f for f in listdir(USERS_PATH) if isfile(join(USERS_PATH, f))]
        for file in local_files:
            with open(USERS_PATH + file, "r") as file_descriptor:
                # extract information from file
                password = file_descriptor.readline()
                uid = file_descriptor.readline()

            # create user and append to list
            username = file
            user = User(username, password, uid)
            self.user_list.append(user)

    def add_user(self, username, password, uid):
        """
        Adds a new user to the list. Also create a new file

        Args:
            username: username for new user
            password: password for new user
            uid: uid for new user
        """
        # create new user and add it to the list of users
        user = User(username, password, uid)
        self.user_list.append(user)

        # Create a File for the user
        self.create_user_file(user)

    def create_user_file(self, user):
        with open("./users/" + user.get_username(), "w") as file_descriptor:
            file_descriptor.write(user.get_password() + "\n")
            file_descriptor.write(user.get_uid())

    def check_if_user_exists(self, username):
        """
        Checks if a user exists in the list.

        Args:
            username: username that is checked for

        Returns:
            True if user exists, False otherwise
        """
        for user in self.user_list:
            if user.get_username() == username:
                return True
        return False

    def delete_user(self, index):
        """
        Deletes a user from the list and deletes the user object

        Args:
            index: user index in the list.
        """
        remove("./users/" + self.user_list[index].get_username())
        del self.user_list[index]

    def get_user_with_uid(self, uid):
        """
        Return the user that has the given uid, else return -1

        Args:
            uid: uid of the user

        Returns:
            User with the given uid, else -1
        """
        for user in self.user_list:
            if user.get_uid() == str(uid):
                return user
        return -1

    def contains_multiple_user(self):
        # TODO: change return value
        if len(self.user_list) > 1:
            return True
        else:
            return False

    def edit_user(self, index, username, pwd):
        self.user_list[index].set_password(pwd)
        if self.user_list[index].get_username() != username:
            remove("./users/" + self.user_list[index].get_username())
            self.user_list[index].set_username(username)
            self.create_user_file(self.user_list[index])
