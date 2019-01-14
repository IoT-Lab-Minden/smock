from tkinter import *
from src.contoller.command import Command
from src.contoller.message import Message


class Gui:
    """
    The Smock gui
    """

    def __init__(self, client_user_interface):
        """
        The constructor of the class. Gives the task_manager the reference to itself.
        Builds the main Window of the gui and loads the user that are existing on the
        Computer to the ListBox.

        Args:
            client_user_interface: The client interface to the service script
        """
        self.__client_user_interface = client_user_interface
        self.__client_user_interface.gui = self

        self.add_window = None
        self.label_near_uid = None

        # create main window
        self.__root = Tk()

        self.__root.title("Smock")
        self.__set_window_size(self.__root, 230, 270)

        # Initialize frames
        self.top_frame = Frame(self.__root)
        self.bottom_frame = Frame(self.__root)

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

        # load users into the list
        self.refresh_list()

    @staticmethod
    def __set_window_size(tk, width, height):
        """
        Sets the size of the window and positions the window to the center of the screen.

        Args:
            tk: The Window that has to be positioned
            width: width of window
            height: height of window
        """
        # gets screen width and height
        width_screen = tk.winfo_screenwidth()
        height_screen = tk.winfo_screenheight()

        # calculate position of window
        x = (width_screen / 2) - (width / 2)
        y = (height_screen / 2) - (height / 2)

        # sets the dimensions of the screen and where it is placed
        tk.geometry('%dx%d+%d+%d' % (width, height, x, y))

    @classmethod
    def notify(cls, message):
        """
        Opens a window with a message for a user.

        Args:
            message: The message displayed on the notify window
        """

        # Create a notify window with a message
        notify_win = Tk()
        width = 300
        height = 100
        cls.__set_window_size(notify_win, width, height)

        # message for the user
        label = Label(notify_win, text=message)

        # destroys the notify window
        btn = Button(notify_win, text="OK", command=notify_win.destroy)

        label.pack()
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
            if self.__client_user_interface.check_if_user_exists(textfield_username.get()):
                self.notify("Der User existiert bereits")
            else:
                # Get the selected user from the listbox and edit its name by the text of the Entry widgets
                user_name = self.__client_user_interface.get_username(self.list.index(ACTIVE))
                uid = self.__client_user_interface.get_uid(self.list.index(ACTIVE))

                # Remove the file of the user

                user_name = textfield_username.get()
                user_pwd = textfield_password.get()
                self.__client_user_interface.set_new_data(self.list.index(ACTIVE), user_name, user_pwd)

                # refresh the listbox and destroy the editing window
                edit_window.destroy()
                self.notify("Der User wurde erfolgreich bearbeitet!")

        # Build the edit window
        edit_window = Toplevel(self.__root)

        edit_window.title("Smock")
        width = 200
        height = 100
        self.__set_window_size(edit_window, width, height)

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
            self.add_window.destroy()
            self.add_window = None

        def refresh_uid():
            # read from queue
            message = Message(Command.UID.value, b'')
            self.__client_user_interface.write_to_controller(message)

        def add_user():
            # First check if username isn't already existing
            if self.__client_user_interface.check_if_user_exists(textfield_username.get()):
                self.notify("Der User existiert bereits")
            else:
                # check if textfield have inputs
                if len(textfield_username.get()) > 0 and len(textfield_password.get()) > 0:
                    self.__client_user_interface.add_user(textfield_username.get(), textfield_password.get(),
                                                          self.label_near_uid.cget("text"))

                    # add user to the listbox
                    self.list.insert(END, textfield_username.get())
                    self.add_window.destroy()
                    self.notify("Der User wurde erfolgreich hinzugefügt!")
                else:
                    self.notify("Bitte geben Sie alle Nutzerdaten ein.")

        # Build the add Window
        self.add_window = Toplevel(self.__root)

        self.add_window.title("Smock")
        width = 300
        height = 100
        self.__set_window_size(self.add_window, width, height)

        add_top_frame = Frame(self.add_window)
        add_bottom_frame = Frame(self.add_window)

        add_top_frame.pack()
        add_bottom_frame.pack()

        label_username = Label(add_top_frame, text="Username: ")
        label_password = Label(add_top_frame, text="Password: ")
        label_uid = Label(add_top_frame, text="uid: ")

        textfield_username = Entry(add_top_frame)
        textfield_password = Entry(add_top_frame)
        self.label_near_uid = Label(add_top_frame, text="nothing")

        label_username.grid(row=0)
        label_password.grid(row=1)
        label_uid.grid(row=2)
        textfield_username.grid(row=0, column=1)
        textfield_password.grid(row=1, column=1)
        self.label_near_uid.grid(row=2, column=1)

        btn_confirm = Button(add_bottom_frame, text="Bestätigen", command=add_user)
        btn_cancel = Button(add_bottom_frame, text="Abbrechen", command=destroy_window)
        btn_refresh = Button(add_bottom_frame, text="uid aktualisieren", command=refresh_uid)

        btn_confirm.grid(row=0, column=1)
        btn_cancel.grid(row=0, column=0, padx=10)
        btn_refresh.grid(row=0, column=2, padx=10)

    def __delete(self):
        """
        Deletes the user that is active in the listbox of the main window and also notify the user manager
        """
        self.__client_user_interface.delete_user(self.list.index(ACTIVE))
        self.notify("Der User wurde erfolgreich gelöscht")

    def refresh_list(self):
        """
        Refreshes the listbox of the main window.
        """
        beginning = 0
        self.list.delete(beginning, END)
        length = self.__client_user_interface.get_number_of_users()
        for user in range(length):
            self.list.insert(END, self.__client_user_interface.get_username(user))

    def update_uid_label(self, uid):
        """
        Updates the label_id, when the user is using the add user window.

        Args:
            uid: uid that is shown on the label_uid
        """
        if self.add_window is not None:
            self.label_near_uid.config(text=str(uid))
            self.add_window.update()

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
        self.__client_user_interface.find_serial_device()
