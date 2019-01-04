from tkinter import *
from os import remove
from command import Command
from message import Message
from threading import Thread


class Gui:

    def __init__(self, queue_manager, serial_manager, user_manager, task_manager):
        self.__queue_manager = queue_manager
        self.__serial_manager = serial_manager
        self.__user_manager = user_manager

        task_manager.gui = self

        self.__add_window = None

        # create main window
        self.__root = Tk()

        self.__root.title("Smock")
        self.__set_window_size(self.__root, 200, 270)

        # Initialize frames
        self.top_frame = Frame(self.__root, pady=3)
        self.bottom_frame = Frame(self.__root, pady=3)

        # Layout frames
        self.top_frame.pack()
        self.bottom_frame.pack()

        # Initialize top frame
        self.list = Listbox(self.top_frame)

        # Initialize bottom frame
        self.btn_add = Button(self.bottom_frame, text="Hinzufügen", command=self.__add, width=9, padx=2)
        self.btn_edit = Button(self.bottom_frame, text="Editieren", command=self.__edit, width=9, padx=2)
        self.btn_delete = Button(self.bottom_frame, text="Löschen", command=self.__delete, width=9, padx=2)
        self.btn_quit = Button(self.bottom_frame, text="Beenden", command=self.__root.destroy, width=9, padx=2)

        # Layout top frame
        self.list.pack()

        # Layout bottom frame
        self.btn_add.grid(row=0, column=0, padx=10)
        self.btn_edit.grid(row=0, column=1, pady=5)
        self.btn_delete.grid(row=1, column=0, padx=10)
        self.btn_quit.grid(row=1, column=1, pady=5)

        # load the users
        self.__user_manager.load_users_from_files()
        self.__refresh_list()

        # Start the gui
        self.__root.mainloop()

    @staticmethod
    def __set_window_size(tk, w, h):
        # set the size of the window and place the window in the middle of the screen
        # get screen width and height
        ws = tk.winfo_screenwidth()
        hs = tk.winfo_screenheight()

        x = (ws / 2) - (w / 2)
        y = (hs / 2) - (h / 2)

        # set the dimensions of the screen
        # and where it is placed
        tk.geometry('%dx%d+%d+%d' % (w, h, x, y))

    def __notify(self, message):
        # Create a notify window with a message
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
            # First check if username isn't already existing
            if self.__user_manager.check_if_user_exists(textfield_username.get()):
                self.__notify("Der User existiert bereits")
            else:
                # Get the selected user from the listbox and edit its name by the text of the Entry widgets
                user = self.__user_manager.user_list[self.list.index(ACTIVE)]

                # Remove the file of the user
                remove("./users/" + user.get_username())

                user.set_username(textfield_username.get())
                user.set_password(textfield_password.get())

                with open("./users/" + user.get_username(), "w") as file_descriptor:
                    file_descriptor.write(user.get_password())
                    file_descriptor.write(user.get_uid())

                self.__refresh_list()
                edit_window.destroy()
                self.__notify("Der User wurde erfolgreich bearbeitet!")

        # Build the edit window
        edit_window = Toplevel(self.__root)

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

        btn_confirm.grid(row=0, column=1, padx=10)
        btn_cancel.grid(row=0, column=0)

    # Add a new user
    def __add(self):

        def destroy_window():
            self.__add_window.destroy()
            self.__add_window = None

        def refresh_uid():
            # read from queue
            message = Message(Command.UID.value, b'')
            self.__serial_manager.write_to_controller(message)
            message = self.__queue_manager.read_queue(Command.UID.value)
            if message.get_text() != "nothing":
                label_near_uid.config(text=str(message.get_text()))
                self.__add_window.update()

        def add_user():
            # First check if username isn't already existing
            if self.__user_manager.check_if_user_exists(textfield_username.get()):
                self.__notify("Der User existiert bereits")
            else:
                # check if textfield have inputs
                if len(textfield_username.get()) > 0 and len(textfield_password.get()) > 0:
                    self.__user_manager.add_user(textfield_username.get(), textfield_password.get(),
                                                 label_near_uid.cget("text"))

                    # add user to the listbox
                    self.list.insert(END, textfield_username.get())
                    self.__add_window.destroy()
                    self.__notify("Der User wurde erfolgreich hinzugefügt!")
                else:
                    self.__notify("Bitte geben Sie alle Nutzerdaten ein.")

        # Build the add Window
        self.__add_window = Toplevel(self.__root)

        self.__add_window.title("Smock")
        self.__set_window_size(self.__add_window, 300, 100)

        add_top_frame = Frame(self.__add_window)
        add_bottom_frame = Frame(self.__add_window)

        add_top_frame.pack()
        add_bottom_frame.pack()

        label_username = Label(add_top_frame, text="Username: ")
        label_password = Label(add_top_frame, text="Password: ")
        label_uid = Label(add_top_frame, text="uid: ")

        textfield_username = Entry(add_top_frame)
        textfield_password = Entry(add_top_frame)
        label_near_uid = Label(add_top_frame, text="nothing")

        label_username.grid(row=0)
        label_password.grid(row=1)
        label_uid.grid(row=2)
        textfield_username.grid(row=0, column=1)
        textfield_password.grid(row=1, column=1)
        label_near_uid.grid(row=2, column=1)

        btn_confirm = Button(add_bottom_frame, text="Bestätigen", command=add_user)
        btn_cancel = Button(add_bottom_frame, text="Abbrechen", command=destroy_window)
        btn_refresh = Button(add_bottom_frame, text="uid aktualisieren", command=refresh_uid)

        btn_confirm.grid(row=0, column=1)
        btn_cancel.grid(row=0, column=0, padx=10)
        btn_refresh.grid(row=0, column=2, padx=10)

    # Delete the active user from list
    def __delete(self):

        self.__user_manager.delete_user(self.list.index(ACTIVE))
        self.__refresh_list()
        self.__notify("Der User wurde erfolgreich gelöscht")

    # Refresh the list, when user deleted or edited
    def __refresh_list(self):
        self.list.delete(0, END)
        for user in self.__user_manager.user_list:
            self.list.insert(END, user.get_username())

    def update_uid_panel(self):
        pass
