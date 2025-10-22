# UNIX Message Platform — C Project

A terminal-based **message distribution system** built in **C for UNIX/Linux**, designed to explore **interprocess communication**, **named pipes (FIFOs)**, and **process synchronization**.  
This project simulates a small-scale **messaging platform** where multiple users can exchange short messages organized by topics in real time.

## Overview

The project implements a client–server architecture with two main programs:

- **Manager** — Responsible for managing topics, users, and message distribution.  
- **Feed** — The user-facing client that allows sending and receiving messages from different topics.

The platform enables users to publish and subscribe to topics dynamically, with messages instantly distributed to all subscribers.  
It also supports **persistent messages** that are stored for a limited lifetime and automatically reloaded when the server restarts.

##  Technologies Used

- **C (GCC)** – Core language for implementation  
- **UNIX System Calls** – For file and process management  
- **Named Pipes (FIFOs)** – For interprocess communication between manager and feed  
- **Signals** – For event handling and process termination  
- **Makefile** – For compilation and cleanup automation  
- **Linux Terminal (Console UI)** – For user interaction  

## Main Features

- **Topic-based messaging** – users can publish or subscribe to specific topics.  
- **Real-time delivery** – messages sent to a topic are immediately distributed to subscribers.  
- **Persistent messages** – messages can remain available for a defined lifetime and are reloaded on restart.  
- **Administrative control** – allows monitoring, locking/unlocking topics, and removing users via the manager console.  
- **Multi-client simulation** –  multiple feeds (users) can run simultaneously from separate terminals.  
- **Safely termination** – both manager and feed processes handle shutdown safely and cleanly.  

## Commands
### In the feed client
- **topics** – List all available topics
- **msg \<topic\> \<duration\> \<message\>** – Send a message to a topic (duration = 0 for non-persistent)
- **subscribe \<topic\>** – Subscribe to a topic  
- **unsubscribe \<topic\>** – Unsubscribe from a topic  
- **exit** – Leave the platform

### In the manager console
- **users** – List all active users  
- **remove \<username\>** – Disconnect a user  
- **topics** – Show all topics and message counts  
- **show \<topic\>** – Display messages in a topic  
- **lock \<topic\>** – Block new messages in a topic  
- **unlock \<topic\>** – Unblock a topic  
- **close** – Shut down the system

## Environment Variables
The system uses an environment variable to store persistent messages:
```bash
export MSG_FICH=persistantMessagesFile.txt
```
This file stores active messages and reloads them when the manager restarts.

## How to Build and Run

### 1. Compile the project
```bash
make all
```

### 2. Start the Manager
```bash
./manager
```

### 3. Launch Feed Clients
```bash
./feed <username>
```
Each client can execute text commands to interact with the system.

---

*This work was completed as part of the “Operating Systems” course during the 2024/2025 academic year in a group of 2 members. (Grade: 10 out of 10)*
