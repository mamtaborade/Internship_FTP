# Internship_FTP

# DAY04_Task(OUTPUT)
Task 1:- Modify the server to print client messages on the server terminal.
Task 2:- Modify the server to send a welcome message to the client immediately after connection. ( Hint :- use write() and accept() )

![image alt](https://github.com/mamtaborade/Internship_FTP/blob/883c693c07c4a7e740f46e5ba743f601018280ee/Day04_Task(OP).png)

# DAY05_Task(OUTPUT)
Task 3:-  Modify the server to terminate connection if client sends "exit".

![image alt](https://github.com/mamtaborade/Internship_FTP/blob/c62076674a956842f698fcc04ee82b6cffbd84c1/Day05_Task(OP).png)

# DAY06_Task(OUTPUT)
Task 1 – Log the HTTP request
Print the full raw HTTP request received from the client on the server terminal.
Task 2 – Change HTML content dynamically
Modify the HTML response to include:
Current date/time (use time() and strftime() in C).
Client’s IP address (from struct sockaddr_in after accept()).
Task 3 – Serve different responses based on URL path
If client requests /hello, respond with "Hello Page".
If /bye, respond with "Goodbye Page".
Otherwise, respond with "Default Page".

![image alt](https://github.com/mamtaborade/Internship_FTP/blob/3134e99fb8d87358b453ffa281acc08b2b827fe8/OUTPUT/Day-6_Task_bye.png)

# DAY08_Task(OUTPUT)
Task 1 – Show process details for each client
Print:
Child process PID
Client’s IP and port number
Connection start time
Task 2 – Keep child alive for multiple requests from same client
Instead of sending one message and closing:
Put a while loop inside the child process to read and respond until the client sends "exit".
This helps simulate a real persistent TCP connection.

![image alt](https://github.com/mamtaborade/Internship_FTP/blob/d55a95d6044f72cb64d8ed4e88b6394b3e66a6c9/OUTPUT/Day08_Task2(OP).png)
