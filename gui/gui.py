from tkinter import *
from user import *

userList = []

# Edit the active user of list. Only username and password
def edit():

    def setUser():
        user = userList[list.index(ACTIVE)]
        user.setUsername(textfieldUsername.get())
        user.setPassword(textfieldPassword.get())
        refreshList()
        editWindow.destroy()

    editWindow = Tk()

    editWindow.title("Smock")

    Label(editWindow, text="Username: ").grid(row=0)
    Label(editWindow, text="Password: ").grid(row=1)

    textfieldUsername = Entry(editWindow)
    textfieldPassword = Entry(editWindow)

    textfieldUsername.grid(row=0, column=1)
    textfieldPassword.grid(row=1, column=1)

    Button(editWindow, text="Bestätigen", command=setUser).grid(row=2)

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


# Delete the active user from list
def delete():

    # Message when user deleted successful
    def noticeMessage():
        deleteWin = Tk()

        label = Label(deleteWin, text="Der User wurde erfolgreich gelöscht")
        label.pack()
        btn = Button(deleteWin, text="OK", command=deleteWin.destroy)
        btn.pack()

        deleteWin.mainloop()

    del userList[list.index(ACTIVE)]
    refreshList()
    noticeMessage()


# Create Main Window
root = Tk()

root.title("Smock")
root.geometry("300x300")


# initialize Widgets
list = Listbox(root)
btnAdd = Button(root, text="Hinzufügen", command=add)
btnEdit = Button(root, text="Editieren", command=edit)
btnDelete = Button(root, text="Löschen", command=delete)
btnQuit = Button(root, text="Beenden", command=root.destroy)

# Create Design
list.pack()
btnAdd.pack()
btnEdit.pack()
btnDelete.pack()
btnQuit.pack()

root.mainloop()
