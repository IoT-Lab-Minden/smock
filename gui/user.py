class User:

    def __init__(self, username, password, uid):
        self.username = username
        self.password = password
        self.uid = uid

    def get_username(self):
        return self.username

    def get_password(self):
        return self.password

    def get_uuid(self):
        return self.uid

    def set_username(self, new_username):
        self.username = new_username

    def set_password(self, new_password):
        self.password = new_password

    def print(self):
        print("Username: " + self.username)
        print("Passwort: " + self.password)
        print("uuid: " + self.uid)
