from tkinter import *
from user import *


class Gui:
    @staticmethod
    def __set_window_size(tk, w, h):
        # get screen width and height
        ws = tk.winfo_screenwidth()
        hs = tk.winfo_screenheight()

        x = (ws / 2) - (w / 2)
        y = (hs / 2) - (h / 2)

        # set the dimensions of the screen
        # and where it is placed
        tk.geometry('%dx%d+%d+%d' % (w, h, x, y))

    def __notify(self, message):
        notify_win = Tk()
        self.__set_window_size(notify_win, 250, 50)

        label = Label(notify_win, text=message)
        label.pack()
        btn = Button(notify_win, text="OK", command=notify_win.destroy)
        btn.pack()

        notify_win.mainloop()

    # Edit the active user of list. Only username and password
    def __edit(self):

        def set_user():
            user = self.user_list[self.list.index(ACTIVE)]
            user.setUsername(textfield_username.get())
            user.setPassword(textfield_password.get())
            self.__refresh_list()
            edit_window.destroy()
            self.__notify("Der User wurde erfolgreich bearbeitet!")

        edit_window = Tk()

        edit_window.title("Smock")
        self.__set_window_size(edit_window, 200, 100)

        edit_top_frame = Frame(edit_window)
        edit_bottom_frame = Frame(edit_window)

        Label(edit_top_frame, text="Username: ").grid(row=0)
        Label(edit_top_frame, text="Password: ").grid(row=1)

        textfield_username = Entry(edit_top_frame)
        textfield_password = Entry(edit_top_frame)

        textfield_username.grid(row=0, column=1)
        textfield_password.grid(row=1, column=1)

        Button(edit_bottom_frame, text="Bestätigen", command=set_user).pack()

        edit_window.mainloop()

    # Refresh the list, when user deleted or edited
    def __refresh_list(self):
        self.list.delete(0, END)
        for user in self.user_list:
            self.list.insert(END, user.getUsername())

    # Add a new user
    def __add(self):

        user = User()
        self.user_list.append(user)
        self.list.insert(END, user.getUsername())
        self.__notify("Der User wurde erfolgreich hinzugefügt!")

    # Delete the active user from list
    def __delete(self):

        del self.user_list[self.list.index(ACTIVE)]
        self.__refresh_list()
        self.__notify("Der User wurde erfolgreich gelöscht")

    def __init__(self):

        self.user_list = []

        # create main window
        self.root = Tk()

        self.root.title("Smock")
        self.__set_window_size(self.root, 200, 270)

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
