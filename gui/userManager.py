from user import User
from os.path import isfile, join
from os import listdir


class UserManager:

    def __init__(self):
        self.user_list = []

    # Loads the users from all files that are in the directory ./users
    def load_users_from_files(self):
        local_path = "./users/"
        # Get all the filenames of the files in the local_path directory
        local_files = [f for f in listdir(local_path) if isfile(join(local_path, f))]
        for file in local_files:
            with open(local_path + file, "r") as file_descriptor:
                password = file_descriptor.readline()
                uid = file_descriptor.readline()

            username = file
            user = User(username, password, uid)
            self.user_list.append(user)

    def add_user(self, username, password, uid):
        # create new user and add it to the list of users
        user = User(username, password, uid)
        self.user_list.append(user)

        # Create a File for the user
        with open("./users/" + username, "w") as file_descriptor:
            file_descriptor.write(password + "\n")
            file_descriptor.write(uid)

    # checks if user exists in the user_list
    def check_if_user_exists(self, username):
        for user in self.user_list:
            if user.get_username() == username:
                return True
        return False

    # deletes a user from the list and deletes the user object
    def delete_user(self, index):
        del self.user_list[index]

    # return the user that has the given uid, else return -1
    def get_user_with_uid(self, uid):
        for user in self.user_list:
            if user.get_uid() == uid:
                return user
        return -1
