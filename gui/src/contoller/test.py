import ctypes

user32 = ctypes.windll.User32

status = user32.GetForegroundWindow()
print(status)

while True:
    newStatus = user32.GetForegroundWindow()
    if newStatus != status:
        print(status)
        status = newStatus
