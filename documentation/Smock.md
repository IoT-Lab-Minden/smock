# SMOCK
**Sm**art C**o**mputer Lo**ck**

## What is SMOCK?

Smock is a device capable of locking or unlocking your computer, depending on weather you are at your workplace or not.  The device is also able to distinguish between differnt users of a computer and to log in the corresponding user.

This functionality should offer a large number of advantages for the user. First of all it is not longer necessary to remember a long password and it is also not necessary to type in this password. This simplifies and speeds up the process of logging in.

In addition to that, the computer logs itself off automatically, when the user moves away and secures his privacy. 

Another advantage is at working enviroments where different users work on the same computer but with different accounts. SMOCK enables the automatic logging in of the user who currently needs the computer.

Last but not least remains the possiblity to log in manualy by entering the username, if needed, and the password.

## Requirements

### Functional requirements

From the above mentioned goals we deduced six important use cases.

![UseCaseDiagram](..\diagramme\Smock\UseCaseDiagram.PNG)

The first three use cases are the mentioned goals. This usecase are all initiated by the user and executed by the SMOCK device. 

So that the device can provide this functionality, the next three use cases are needed. The user must have the possibility to register himself, so the device can identify him as user from the system. The device also needs to get the information needed by the system to log in. Furthermore, the user needs the oppurtunity to edit his registerted setting in case he made a mistake or changed his data in the system. Finally the user must have the opportunity to delete all his data from the system.

### Non-functional requirements

To limit the requirements we had to set different functional requirements.

First of all should the system be capable of recognizing and differntiate multiple users. If the device would not fulfill this, the privacy of every user could not be guaranteed.

Another very important requirement is the integrity of the system. The device should be __small and easy to plug or unplug__. When connecting the device to another computer, this device must not be capable of reading the user data of the other computer.

This two reqirements are the most important. If they are not met the system does not work and must not be used! **ändern**

In addition there are a few a little less important requirements.

The first one is the availability. When the computer runs the system should also work and be active, so that the user does not need to log in manually.

Moreover the efficiency is important. Additional to the host log in/out time should hardly pass time. The user should not be fast by logging in manually then the device is. Furthermore, the device should not use up to much energy.

Finally we have decided that the system should work on two different operating system. Firstly windows 10  and secondly ubuntu 16.04 should be supported.

__Python3.5??__

All system requiremnts, functional and non-functional, are shown in the image below:

![non-functional_requirements](..\diagramme\Smock\non-functional_requirements.png)

## Components

Thinking about differnt way to identifcate users we have decided to use RFID. RFID has multiple advantages for our system.

First of all the user just needs to carry around the RFID Tag. This Tags can be very small and they are available in many different variants. This makes it easy to carry them.

RFID Tag are also common in many companys as key cards. So many of the employed already have a RFID Tag.

Finally the tags can easly identificated and and be distinguished from other tags.

To communicate with the tag we need a RFID Reader. The reader has the task to read the ID from the tag. 

To controll the reader and to communicate with the computer we use a usb-capable microcontroller. The microcontroller handles the event when a new card is read by the RFID Reader, verifies the user and logs him. The same way works the logging off from the user.

![CompositeDiagram](..\diagramme\Smock\CompositeDiagram.PNG)

## Software Structure

### Device

The Software of the device is separted into two different parts. The first one which controls the MFRC522 RFID Reader mainly consits of one class, the __MRFC522__ class.

This class uses the SPI module provieded by the energia librarys to communicate with the host. The class hold the currently aviable uid in a UID struct. This struct contains an array of bytes that holds the ID. The struct also holds the length of the UID array.

The __MRFC522__ class offers the needed functions to setup and communicate with the reader. To handle different error values a StatusCode enumeration is used.

The second part controls the usb communication. The general communication between the host and the device is realized with a serial interface. The device is  represented by the __USBSerialDevice__. It contains a tUSBCDCDevice which is responsible for the serial communication. The tUSBCDCDevice type is provided by the usb Library from Texas Instruments and it is used to register the callbacks to handle different usb events. There are three Callbacks needed for the serial communication. The first one handles the general usb events like connecting or disconnecting the usb connection, the second one handles the event when data was received and the third one handles the event when data was transmitted.

The received data is written into a receive buffer and the data that should be transmitted needs to be written in a transmitt buffer.

To lock or unlock the host it is necessary to simulate a keyboard device. For this is the __USBKeyboardDevice__ class used. The class handles events received from the host and provides methods to send keys. The event handler callback method is registered with a tUSBHIDKeyboardDevice which is like the tUSBCDCDevice responible for the connection.

Since we are connecting two usb devices to the host via one usb port we need a composite device to combine them. The device is represened by the __USBCompDevice__ class. This class holds both structures of the devices which a responsible for the connection (the tUSBHIDKeyboardDevice  and the tUSBCDCDevice).  Events send from the host can be the releasing or pressing of special keys.

Because this class takes over the registration at the host the device also contains a descriptor data array that holds the informations needed by the host. The connection is like the other devices handled by a structure provided by the usb library. This tUSBCompositeDevice struct also registers a event handler for default usb events like connecting or disconnecting.

![ClassDiagram_rfid](..\diagramme\Smock\ClassDiagram_rfid.png)

![ClassDiagram_usb](..\diagramme\Smock\ClassDiagram_usb.png)

### Host

The software of the host is seperated in two different scripts. The first script is the service script and the second script is the user script.

The service script is for communicating with the serial device. The class that is talking to the serial device is the __SerialManager__. This class receives the messages from the serial device as well as writes messages to the serial device. When the __SerialManager__ receives a message it forwards the message to the __QueueManager__.

The __QueueManager__ puts the message, that it gets into seperate queues, depending on what message the QueueManager got. When another class asks to read a message from a queue, the class has to tell the QueueManager which command he is looking for. The QueueManager will look into the correct queue and returns the message, when a message exists.

A __Message__ contains a command code and the text. The command code is useful for seperating the messages and being aware, what the serial device wants. The text is the information of message.

The __TaskManager__ reads if there are any messages in the QueueManager. When the TaskManager reads a message it responds to this message proceeds according to what the command code says.

The __UserManager__ is responsible for loading the already existing users from files and also managing them. This means adding a new User, editing them or delete a User. The __User__ has the attributes username, password and uid.

The last class of the service script is the __ListenerUserInterface__. This class is the interface to the user script and uses ipv4 to communicate.

The user script is for the user. This opens a Gui that the user can use to add new User or edit/delete User. The __Gui__ has a main window and the different methods to answer the events that are triggered by pressing buttons. The user Script connects to the open port of the ListenerUserInterface of the service script. This happens in the class CLientUserInterface. The __ClientUserInterface__ has also the task to pass changes that are made by a user to the service script.

## Software processes

### Device

The main component of the device software is realized with a state machine.

![StateComputer](..\diagramme\Smock\StateComputer.PNG)

After starting the microcontroller goes into the *START* state. All events that come in, whether they come from the rfid reader or via the serial port, are discarded, except for the OS_SYSTEM event from the host. This message contains information about the operarting system and the number of users registerd to the software. Without this information the log in or off won't work because there are different schedules needed for a different number of users or different operating systems.

After receiving the needed information the host goes in the *LOCKED* state. This state is only left when a card is read by the rfid reader. The readed Uid from the tag is stored as currentUid. The device sends over the serial interface a request for the password and optional the username matching to the Uid.

Then the system goes in the state *WAIT_FOR_PW*. In this state the device waits for the answer of the host. There are two differnt way the host can answer. The first opportunity is that the device receives as HOST_STATUS message. This message indicates that the host is already logged in. In this case a user has unlocked the host manually and the device should not sing in the new user. The system goes back to the *LOCKED* state in that case and resets the currentUid to a default value.

The second possible answer is a message containg the the username and optinal the pasword. In this case the microcontroller uses the information to logg in the user using the simulated keyboard. The following state in this case is the *UNLOCKED* state.

The device stay in this state until the card is gone or a other card is read. There is a conuter used which is increased when no or a differnt card is read. If the counter reaches a certain value the card is identified as lost. In that case the system resets the currentUid to the default value and and reqeusts the host state via the serial interface. After that the host goes to the *VALIDATE_HOST_LOCKED_STATE*. The missing card counter is reset to zero when the card is detected again. The use of a counter is necessary because the reader does not always detects the card.

In the *VALIDATE_HOST_LOCKED_STATE* the device waits for the answer of the host to the status request. If the host returns that he is already locked out the state of the device is simply changed to *LOCKED*. Otherwise the device locks the host using the simulated keyboard and goes in the *LOCKED* state afterwards.

In every state are unexpected events discarded except for a few certain requests received via the serial interface.

The first one is the information that the number of registered users has changed. In this case the host sends a USER_QUANTITY code followed by 1 if there is only one usere registered and a 2 other wise.

The second on is when the operating system information is send again. When this happens the device stores updates the operating system and user quantity informations.

Finally the currently read UID can be requested from the host to connect it to a user. The device returns the the value of the currentUid if it does not hold the default value. In that case is a empty uid returned.

*gff. in unter punkten abläufe innerhalb der seriellen schnittstelle/tatstatur klasse beschreiben - oder in doku ausführlicher?*

### Host

#### Service Script

At first the service script will be started. This initializes the __QueueManager__, the __UserManager__, the __SerialManager__, the __ListenerUserInterface__ and the __TaskManager__. Then there start three different threads. 
The first thread opens the port of the ListenerUserInterface and waits for incoming connections from the user script. When a user connects to the port, the user will be stored in a client list of the ListenerUserInterface.
The next thread is calling a function from the SerialManager. This function is called __SerialManager.fill_queue()__ and waits for recieving bytes from the serial device. The messages will be stored as __Message__ in the queue with the function __QueueManager.write_queue()__.
The last thread calls the function __TaskManager.read_tasks()__. Depending on what command code the TaskManager reads from the queue it calls the functions __TaskManager.send_password_to_controller()__, __TaskManager.send_computer_status()__ oder __TaskManager.update_add_window()__.
After that three threads the function __ListenerUserInterface.check_for_new_msg()__ is called in an endless loop.

#### User Script

After the service script is started, the user script can be started. At first the __ClientUserInterface__ and the __Gui__ are initialized. Then a thread is started. It calls the function __ClientUserInterface.polling()__. After this thread is called the gui will be started. After that the user has a few possibillities to continue.
The first is to add a new __User__. Therefore the User presses the button and then the function __Gui.add()__ is called. This will open a new window, where the user can type in the account information of a user. The user can also hold a new RFID tag at the reader and connect this tag to the user account. When the user filled in all information, then at first the function __ClientUserInterface.check_if_user_exists()__ is called so there won't be a duplicate user. Then the user will be added to the UserManager in the service script. At first the __ClientUserInterface.client_user_interface.add_user()__ is called. This will pass the information to the service script. There it will call the function __UserManager.add_user()__.
If the user wants to edit an account.




## Setup

### Windows

To set up the program for windows python needs to be installed on the host. Pthon version

### Ubuntu



## Conclusion

## Code Documentation

## Datasheets

