from user import User
from os.path import isfile, join
from os import listdir


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

    def load_users_from_files(self):
        """
        Loads the users from all files that are in the directory ./users
        """
        local_path = "./users/"
        # Get all the filenames of the files in the local_path directory
        local_files = [f for f in listdir(local_path) if isfile(join(local_path, f))]
        for file in local_files:
            with open(local_path + file, "r") as file_descriptor:
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
        with open("./users/" + username, "w") as file_descriptor:
            file_descriptor.write(password + "\n")
            file_descriptor.write(uid)

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
        del self.user_list[index]

    def get_user_with_uid(self, uid):
        """
        Return the user that has the given uid, else return -1

        Args:
            uid:

        Returns:
            User with the given uid, else -1
        """
        for user in self.user_list:
            if user.get_uid() == uid:
                return user
        return -1
