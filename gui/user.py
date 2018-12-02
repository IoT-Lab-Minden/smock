from tkinter import *

class User:

    username = ""
    password = ""
    uuid = ""

    def __init__(self):
        self.username = "anon"
        self.password = "test"
        self.uuid = "teststring"

    def getUsername(self):
        return self.username

    def getPassword(self):
        return self.password

    def getUuid(self):
        return self.uuid

    def setUsername(self, newUsername):
        self.username = newUsername

    def setPassword(self, newPassword):
        self.password = newPassword

    def printUser(self):
        print("Username: " + self.username)
        print("Passwort: " + self.password)
        print("uuid: " + self.uuid)
