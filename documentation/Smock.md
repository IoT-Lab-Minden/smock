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

![CompositeDiagram](..\diagramme\Smock\CompositeDiagram.PNG)

# 1 Introduction

## 1.1 Description

Smock (Smart Computer Lock) is a device that locks your computer when you are not at your place and unlocks it when you are back at your place. Smock is able to operate on Windows 10 and Ubuntu. It's able to use smock with only one existing user on your computer, as well as using it on a computer with multiple users. Smock works with an RFID reader and an RFID tag. The user uses the tag to unlock device and takes away the tag when the user wants to lock the computer. When "User A" locks out of the computer, then another "User B" can log into the device with his tag. Smock will switch the user automatically and logs in the other user. 

## 1.2 Goals

### 1.2.1 Applications

Smock is used for Windows 10 and Ubuntu Computer.

Auch auf anderen Möglich -> spezielle anpassungen an das betriebs system

### 1.2.2 Motivation

People often forget their passwords of an account or choose easy to guess passwords. Smock prevents this and makes it possible to log in to your device without knowing the password.

### 1.2.3 Coverage

Smock is able to be used on Windows 10 and Ubuntu Computer. It is also possible have multiple user on a computer.

### 1.2.4 Unique Selling Point

Smock is the first device that allows you to log on or off to your computer using an RFID card.

### 1.2.5 Target

Users at all ages, that are using their computer for Work. Companies, that wants to connect users to a specific device.

# 2 Anforderungen

## 2.1 Funktionale Anforderungen

## 2.2 Nicht-funktionale Anforderungen 

## 2.3 Graphische Benutzerschnittstelle

# 3 Technische Beschreibung

## 3.1 Systemübersicht

### 3.1.1 Schnittstellen


## 3.5 Entwurf

### Klassendiagramme

### Sequenzdiagramme

# 4  Code Dokumentation
