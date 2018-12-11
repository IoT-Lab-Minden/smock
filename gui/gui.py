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
            user.set_username(textfield_username.get())
            user.set_password(textfield_password.get())
            self.__refresh_list()
            edit_window.destroy()
            self.__notify("Der User wurde erfolgreich bearbeitet!")

        edit_window = Toplevel(self.root)

        edit_window.title("Smock")
        self.__set_window_size(edit_window, 200, 100)

        edit_top_frame = Frame(edit_window)
        edit_bottom_frame = Frame(edit_window)

        edit_top_frame.pack()
        edit_bottom_frame.pack()

        label_username = Label(edit_top_frame, text="Username: ")
        label_password = Label(edit_top_frame, text="Password: ")

        textfield_username = Entry(edit_top_frame)
        textfield_password = Entry(edit_top_frame)

        label_username.grid(row=0)
        label_password.grid(row=1)
        textfield_username.grid(row=0, column=1)
        textfield_password.grid(row=1, column=1)

        btn_confirm = Button(edit_bottom_frame, text="Bestätigen", command=set_user)
        btn_cancel = Button(edit_bottom_frame, text="Abbrechen", command=edit_window.destroy)

        btn_confirm.grid(row=0, column=1)
        btn_cancel.grid(row=0, column=0)

    # Refresh the list, when user deleted or edited
    def __refresh_list(self):
        self.list.delete(0, END)
        for user in self.user_list:
            self.list.insert(END, user.get_username())

    # Add a new user
    def __add(self):

        def add_user():
            user = User(textfield_username.get(), textfield_username.get(), label_near_uid.cget("text"))
            user.print()
            self.user_list.append(user)
            self.list.insert(END, user.get_username())
            edit_window.destroy()
            self.__notify("Der User wurde erfolgreich hinzugefügt!")

        edit_window = Toplevel(self.root)

        edit_window.title("Smock")
        self.__set_window_size(edit_window, 300, 100)

        add_top_frame = Frame(edit_window)
        add_bottom_frame = Frame(edit_window)

        add_top_frame.pack()
        add_bottom_frame.pack()

        label_username = Label(add_top_frame, text="Username: ")
        label_password = Label(add_top_frame, text="Password: ")
        label_uid = Label(add_top_frame, text="uid: ")

        textfield_username = Entry(add_top_frame)
        textfield_password = Entry(add_top_frame)
        label_near_uid = Label(add_top_frame, text="569f625c-fd1d-11e8-8eb2-f2801f1b9fd1")

        label_username.grid(row=0)
        label_password.grid(row=1)
        label_uid.grid(row=2)
        textfield_username.grid(row=0, column=1)
        textfield_password.grid(row=1, column=1)
        label_near_uid.grid(row=2, column=1)

        btn_confirm = Button(add_bottom_frame, text="Bestätigen", command=add_user)

        btn_confirm.pack()

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
        self.top_frame = Frame(self.root, pady=3)
        self.bottom_frame = Frame(self.root, pady=3)

        # Layout frames
        self.top_frame.pack()
        self.bottom_frame.pack()

        # Initialize top frame
        self.list = Listbox(self.top_frame)

        # Initialize bottom frame
        self.btn_add = Button(self.bottom_frame, text="Hinzufügen", command=self.__add, width=9, padx=2)
        self.btn_edit = Button(self.bottom_frame, text="Editieren", command=self.__edit, width=9, padx=2)
        self.btn_delete = Button(self.bottom_frame, text="Löschen", command=self.__delete, width=9, padx=2)
        self.btn_quit = Button(self.bottom_frame, text="Beenden", command=self.root.destroy, width=9, padx=2)

        # Layout top frame
        self.list.pack()

        # Layout bottom frame
        self.btn_add.grid(row=0, column=0)
        self.btn_edit.grid(row=0, column=1)
        self.btn_delete.grid(row=1, column=0)
        self.btn_quit.grid(row=1, column=1)

        self.root.mainloop()
