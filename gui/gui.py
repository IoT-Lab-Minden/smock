from tkinter import *
from os import remove
from command import Command
from message import Message
from threading import Thread


class Gui:
    """
    The Smock gui
    """

    def __init__(self, queue_manager, serial_manager, user_manager, task_manager):
        """
        The constructor of the class. Gives the task_manager the reference to itself.
        Builds the main Window of the gui and loads the user that are existing on the
        Computer to the ListBox.

        Args:
            queue_manager: A queueManager
            serial_manager: A serialManager
            user_manager: A userManager
            task_manager: A taskManager
        """
        self.__queue_manager = queue_manager
        self.__serial_manager = serial_manager
        self.__user_manager = user_manager

        task_manager.gui = self

        self.__add_window = None

        # create main window
        self.__root = Tk()

        self.__root.title("Smock")
        self.__set_window_size(self.__root, 230, 270)

        # Initialize frames
        self.top_frame = Frame(self.__root, pady=3)
        self.bottom_frame = Frame(self.__root, pady=3)

        # Layout frames
        self.top_frame.pack()
        self.bottom_frame.pack()

        # Initialize top frame
        self.list = Listbox(self.top_frame)

        # Initialize bottom frame
        self.btn_add = Button(self.bottom_frame, text="Hinzufügen", command=self.__add, width=11, padx=2)
        self.btn_edit = Button(self.bottom_frame, text="Editieren", command=self.__edit, width=11, padx=2)
        self.btn_delete = Button(self.bottom_frame, text="Löschen", command=self.__delete, width=11, padx=2)
        self.btn_refresh_controller = Button(self.bottom_frame, text="Gerät erkennen", command=self.__find_controller,
                                             width=11, padx=2)
        self.btn_quit = Button(self.bottom_frame, text="Beenden", command=self.__root.destroy, width=11, padx=2)

        # Layout top frame
        self.list.pack()

        # Layout bottom frame
        self.btn_add.grid(row=0, column=0, padx=10)
        self.btn_edit.grid(row=0, column=1, pady=5)
        self.btn_delete.grid(row=1, column=0, padx=10)
        self.btn_quit.grid(row=2, column=0, pady=5)
        self.btn_refresh_controller.grid(row=1, column=1)

        # load the users
        self.__user_manager.load_users_from_files()
        self.__refresh_list()

    @staticmethod
    def __set_window_size(tk, w, h):
        """
        Sets the size of the window and position the window to the center of the screen.

        Args:
            tk: The Window that has to be positioned
            w: width of window
            h: height of window
        """
        # gets screen width and height
        ws = tk.winfo_screenwidth()
        hs = tk.winfo_screenheight()

        # calculate position of window
        x = (ws / 2) - (w / 2)
        y = (hs / 2) - (h / 2)

        # sets the dimensions of the screen and where it is placed
        tk.geometry('%dx%d+%d+%d' % (w, h, x, y))

    @classmethod
    def notify(cls, message):
        """
        Opens a window with a message for a user.

        Args:
            message: The message displayed on the notify window
        """

        # Create a notify window with a message
        notify_win = Tk()
        cls.__set_window_size(notify_win, 300, 100)

        label = Label(notify_win, text=message)
        label.pack()
        btn = Button(notify_win, text="OK", command=notify_win.destroy)
        btn.pack()

        notify_win.mainloop()

    # Edit the active user of list. Only username and password
    def __edit(self):
        """
        The window to edit a user. User is selected from the listbox of the main window.
        Only able to edit username and password.
        """
        def set_user():
            # First check if username isn't already existing
            if self.__user_manager.check_if_user_exists(textfield_username.get()):
                self.notify("Der User existiert bereits")
            else:
                # Get the selected user from the listbox and edit its name by the text of the Entry widgets
                user = self.__user_manager.user_list[self.list.index(ACTIVE)]

                # Remove the file of the user
                remove("./users/" + user.get_username())

                user.set_username(textfield_username.get())
                user.set_password(textfield_password.get())

                # change the name of the file and its content
                with open("./users/" + user.get_username(), "w") as file_descriptor:
                    file_descriptor.write(user.get_password())
                    file_descriptor.write(user.get_uid())

                # refresh the listbox and destroy the editing window
                self.__refresh_list()
                edit_window.destroy()
                self.notify("Der User wurde erfolgreich bearbeitet!")

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
        """
        Opens a window for adding a user. The user needs to fill in his username, password and uid.
        Saves these information in a file. The filename is the username of the user.
        """
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
                self.notify("Der User existiert bereits")
            else:
                # check if textfield have inputs
                if len(textfield_username.get()) > 0 and len(textfield_password.get()) > 0:
                    self.__user_manager.add_user(textfield_username.get(), textfield_password.get(),
                                                 label_near_uid.cget("text"))

                    # add user to the listbox
                    self.list.insert(END, textfield_username.get())
                    self.__add_window.destroy()
                    self.notify("Der User wurde erfolgreich hinzugefügt!")
                else:
                    self.notify("Bitte geben Sie alle Nutzerdaten ein.")

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

    def __delete(self):
        """
        Deletes the user that is active in the listbox of the main window
        """
        self.__user_manager.delete_user(self.list.index(ACTIVE))
        self.__refresh_list()
        self.notify("Der User wurde erfolgreich gelöscht")

    def __refresh_list(self):
        """
        Refreshes the listbox of the main window.
        """
        self.list.delete(0, END)
        for user in self.__user_manager.user_list:
            self.list.insert(END, user.get_username())

    def update_uid_panel(self):
        pass

    def start(self):
        """
        Opens the main window.
        Should only be called once.
        """
        self.__root.mainloop()

    def __find_controller(self):
        """
        Searches for the serial_device
        """
        self.__serial_manager.find_serial_device()
