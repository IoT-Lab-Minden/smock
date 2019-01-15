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



## Software processes

### Device

### Host

## Setup

## Conclusion

## Code Documentation

## Datasheet

