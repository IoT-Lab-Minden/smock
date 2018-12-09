from tkinter import *
from user import *

userList = []


def setWindowsize(tk, w, h):

    # get screen width and height
    ws = root.winfo_screenwidth()
    hs = root.winfo_screenheight()

    x = (ws / 2) - (w / 2)
    y = (hs / 2) - (h / 2)

    # set the dimensions of the screen
    # and where it is placed
    tk.geometry('%dx%d+%d+%d' % (w, h, x, y))


def notify(message):
    notifyWin = Tk()
    setWindowsize(notifyWin, 250, 50)

    label = Label(notifyWin, text=message)
    label.pack()
    btn = Button(notifyWin, text="OK", command=notifyWin.destroy)
    btn.pack()

    notifyWin.mainloop()


# Edit the active user of list. Only username and password
def edit():

    def setUser():
        user = userList[list.index(ACTIVE)]
        user.setUsername(textfieldUsername.get())
        user.setPassword(textfieldPassword.get())
        refreshList()
        editWindow.destroy()
        notify("Der User wurde erfolgreich bearbeitet!")

    editWindow = Tk()

    editWindow.title("Smock")
    setWindowsize(editWindow, 200, 100)


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
def refreshList():
    list.delete(0, END)
    for user in userList:
        list.insert(END, user.getUsername())


# Add a new user
def add():
    user = User()
    userList.append(user)
    list.insert(END, user.getUsername())
    notify("Der User wurde erfolgreich hinzugefügt!")


# Delete the active user from list
def delete():
    del userList[list.index(ACTIVE)]
    refreshList()
    notify("Der User wurde erfolgreich gelöscht")


# create main window
root = Tk()

root.title("Smock")
setWindowsize(root, 200, 270)

# Initialize frames
topFrame = Frame(root, pady=3)
bottomFrame = Frame(root, pady=3)

# Layout frames
topFrame.pack()
bottomFrame.pack()

# Initialize top frame
list = Listbox(topFrame)

# Initialize bottom frame
btnAdd = Button(bottomFrame, text="Hinzufügen", command=add, width=9, padx=2)
btnEdit = Button(bottomFrame, text="Editieren", command=edit, width=9, padx=2)
btnDelete = Button(bottomFrame, text="Löschen", command=delete, width=9, padx=2)
btnQuit = Button(bottomFrame, text="Beenden", command=root.destroy, width=9, padx=2)

# Layout top frame
list.pack()

# Layout bottom frame
btnAdd.grid(row=0, column=0)
btnEdit.grid(row=0, column=1)
btnDelete.grid(row=1, column=0)
btnQuit.grid(row=1, column=1)

root.mainloop()
