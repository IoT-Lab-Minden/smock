from tkinter import *
from user import *


class Gui:
    def __setWindowsize(self, tk, w, h):
        # get screen width and height
        ws = tk.winfo_screenwidth()
        hs = tk.winfo_screenheight()

        x = (ws / 2) - (w / 2)
        y = (hs / 2) - (h / 2)

        # set the dimensions of the screen
        # and where it is placed
        tk.geometry('%dx%d+%d+%d' % (w, h, x, y))

    def __notify(self, message):
        notifyWin = Tk()
        self.__setWindowsize(notifyWin, 250, 50)

        label = Label(notifyWin, text=message)
        label.pack()
        btn = Button(notifyWin, text="OK", command=notifyWin.destroy)
        btn.pack()

        notifyWin.mainloop()

    # Edit the active user of list. Only username and password
    def __edit(self):
        
        def setUser():
            user = self.userList[list.index(ACTIVE)]
            user.setUsername(textfieldUsername.get())
            user.setPassword(textfieldPassword.get())
            self.__refreshList()
            editWindow.destroy()
            self.__notify("Der User wurde erfolgreich bearbeitet!")

        editWindow = Tk()

        editWindow.title("Smock")
        self.__setWindowsize(editWindow, 200, 100)


        editTopFrame = Frame(editWindow)
        editBottomFrame = Frame(editWindow)

        Label(editTopFrame, text="Username: ").grid(row=0)
        Label(editTopFrame, text="Password: ").grid(row=1)

        textfieldUsername = Entry(editTopFrame)
        textfieldPassword = Entry(editTopFrame)

        textfieldUsername.grid(row=0, column=1)
        textfieldPassword.grid(row=1, column=1)

        Button(editBottomFrame, text="Bestätigen", command=setUser).pack()

        editWindow.mainloop()


    # Refresh the list, when user deleted or edited
    def __refreshList(self):
        list.delete(0, END)
        for user in self.userList:
            list.insert(END, user.getUsername())

    # Add a new user
    def __add(self):

        user = User()
        self.userList.append(user)
        self.list.insert(END, user.getUsername())
        self.__notify("Der User wurde erfolgreich hinzugefügt!")

    # Delete the active user from list
    def __delete(self):

        del self.userList[list.index(ACTIVE)]
        self.__refreshList()
        self.__notify("Der User wurde erfolgreich gelöscht")

    def __init__(self):

        self.userList = []

        # create main window
        self.root = Tk()

        self.root.title("Smock")
        self.__setWindowsize(self.root, 200, 270)

        # Initialize frames
        self.topFrame = Frame(self.root, pady=3)
        self.bottomFrame = Frame(self.root, pady=3)

        # Layout frames
        self.topFrame.pack()
        self.bottomFrame.pack()

        # Initialize top frame
        self.list = Listbox(self.topFrame)

        # Initialize bottom frame
        self.btnAdd = Button(self.bottomFrame, text="Hinzufügen", command=self.__add, width=9, padx=2)
        self.btnEdit = Button(self.bottomFrame, text="Editieren", command=self.__edit, width=9, padx=2)
        self.btnDelete = Button(self.bottomFrame, text="Löschen", command=self.__delete, width=9, padx=2)
        self.btnQuit = Button(self.bottomFrame, text="Beenden", command=self.root.destroy, width=9, padx=2)

        # Layout top frame
        self.list.pack()

        # Layout bottom frame
        self.btnAdd.grid(row=0, column=0)
        self.btnEdit.grid(row=0, column=1)
        self.btnDelete.grid(row=1, column=0)
        self.btnQuit.grid(row=1, column=1)

        self.root.mainloop()
