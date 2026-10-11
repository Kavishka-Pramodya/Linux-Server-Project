# Multi-Process HTTP Web Server in C

A lightweight, concurrent HTTP/1.1 web server implemented from scratch in **C** using raw POSIX sockets and the Linux system call interface (`fork`, `socket`, `bind`, `listen`, `accept`).

Developed as a first-year, second-semester **Computer Networks** academic project.

---

## 📌 Project Overview

This project explores low-level network communication, the TCP/IP stack, and the HTTP protocol without using external networking libraries or frameworks. It demonstrates how modern web servers accept incoming TCP connections, parse HTTP request headers, and deliver static web assets (`HTML`, `CSS`, `JavaScript`, and images) across the network.

### ✨ Key Features
- **Raw Socket Programming:** Implemented using standard POSIX networking APIs (`<sys/socket.h>`, `<netinet/in.h>`).
- **Concurrent Request Handling:** Multi-process architecture using `fork()` to handle concurrent client connections.
- **Zombie Process Prevention:** Configured `signal(SIGCHLD, SIG_IGN)` to automatically reap terminated child processes.
- **MIME Type Detection:** Dynamically resolves Content-Type headers for `.html`, `.css`, `.js`, `.png`, and `.jpg`.
- **Chunked File Streaming:** Streams binary and text assets in 1024-byte chunks using `fread()` and `send()`.
- **Custom 404 Error Handling:** Returns a styled HTTP 404 response when a requested resource does not exist.

---

## 🏗️ Architecture & Workflow

```
               +---------------------------------------+
               |         Client (Browser / cURL)       |
               +---------------------------------------+
                                  │
                                  │ TCP Handshake (Port 8080)
                                  ▼
               +---------------------------------------+
               |     Parent Process (server_fd)        |
               |  socket() -> bind() -> listen()       |
               +---------------------------------------+
                                  │
                                  │ accept() incoming connection
                                  ▼
                            fork() Process
                           /              \
           [Parent Process]                [Child Process (pid == 0)]
           - Closes client fd              - Closes server_fd
           - Loops to accept next          - recv() HTTP Request
                                           - Parse Method, Path, Protocol
                                           - Match MIME & fopen()
                                           - send() HTTP Headers + Body
                                           - close() & exit(0)
```

---

## 📁 Repository Structure

```
├── server.c      # Main HTTP server implementation (Socket & Process logic)
├── server        # Compiled Linux binary
├── index.html    # Default landing webpage served at route '/'
└── README.md     # Project documentation & security analysis
```

---

## 🚀 Getting Started

### Prerequisites
- **OS:** Linux (Ubuntu/Debian, Fedora, Arch) or Windows Subsystem for Linux (WSL).
- **Compiler:** GCC (GNU Compiler Collection) or Clang.
- **Tools:** `make` (optional), `curl`, web browser.

### 1. Compilation
Compile `server.c` using `gcc`:
```bash
gcc -o server server.c -Wall
```

### 2. Running the Server
Run the compiled executable:
```bash
./server
```

You should see the startup logs in your terminal:
```
Attached IP and Port for the Socket.....
Listening.....
```

### 3. Testing the Server
Open your web browser or run `curl`:

- **Browser:** Navigate to `http://localhost:8080/`
- **cURL:**
  ```bash
  curl -i http://localhost:8080/
  ```

---

## 🔍 Security Analysis & Known Vulnerabilities (Student Research)

During testing and post-development review of this project, a critical security vulnerability was discovered in the request routing mechanism:

### ⚠️ Directory / Path Traversal Vulnerability (CWE-22)

#### 1. Vulnerability Description
In `server.c`, the requested file path is extracted and parsed as follows:
```c
if(strcmp(filename, "/") == 0){
    file_to_open = "index.html";
} else {
    file_to_open = filename + 1; // Strips the leading '/' character
}

FILE *file = fopen(file_to_open, "rb");
```

Because `filename + 1` is passed directly to `fopen()` without path sanitization or canonicalization, an attacker can supply directory traversal sequences (such as `../`) to navigate out of the intended web root directory.

#### 2. Proof of Concept (PoC)
An attacker can request files outside the project directory using tools like `curl`, Burp Suite, or Netcat:
```bash
# Attempt to read Linux system configuration files:
curl -i http://localhost:8080/../../../../etc/passwd
```
If the server process runs with sufficient file read permissions, `fopen()` resolves the relative traversal path and serves sensitive system files (such as `/etc/passwd`) back to the client.

#### 3. Other Noted Observations
- **Buffer Overflow Risk in `sscanf`:** The line `sscanf(recv_msg, "%s %s %s", method, filename, protocol);` does not specify width limits for the destination buffers (`filename[100]`), creating a potential stack buffer overflow vulnerability if a client sends a URI longer than 100 characters.
- **Lack of HTTP Method Validation:** Any HTTP request method (e.g., `POST`, `DELETE`, `OPTIONS`) is processed identically to a `GET` request.

> **Note:** As this repository serves as an academic archive of my 1st-year coursework and learning journey, the source code has been preserved in its original state without altering the vulnerability.

---

### 🛡️ Recommended Mitigations (For Future Production Implementations)

To secure against Directory Traversal and related risks in C web servers:

1. **Path Canonicalization with `realpath()`:**
   Resolve the absolute canonical path of the target file using `realpath()` and verify that it strictly starts with the authorized base document root directory:
   ```c
   char resolved_path[PATH_MAX];
   if (realpath(file_to_open, resolved_path) != NULL) {
       if (strncmp(resolved_path, DOCUMENT_ROOT, strlen(DOCUMENT_ROOT)) != 0) {
           // Reject request: 403 Forbidden
       }
   }
   ```
2. **Path Sanitization:** Reject any URI containing substrings like `..` or null bytes (`%00`).
3. **Safe Parsing with Width Specifiers:**
   ```c
   sscanf(recv_msg, "%9s %99s %19s", method, filename, protocol);
   ```
4. **Principle of Least Privilege & Chroot:** Run the server under an unprivileged user account and isolate the server in a `chroot` jail or container.

---

## 📚 Academic Context & Learning Outcomes

- **Course:** Computer Networks (Year 1, Semester 2)
- **Topics Covered:**
  - Socket Lifecycle: `socket()`, `setsockopt()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`, `close()`.
  - Process Management: Concurrency via `fork()`, child process termination, and signal handling (`SIGCHLD`).
  - Network Protocols: RFC compliance of HTTP/1.1 headers and status codes.
  - Software Security: Practical awareness of input validation, path traversal, and memory safety in C.

---

## 📄 License
This repository is published for educational and portfolio demonstration purposes.
