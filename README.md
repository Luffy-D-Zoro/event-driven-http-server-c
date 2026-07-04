# 🚀 Event-Driven HTTP File Server in C

A lightweight **HTTP file server** implemented in pure C using Linux sockets and the `select()` system call.

This project demonstrates low-level networking, event-driven architecture, static file serving, and raw file upload handling without using any external web framework.

---

## 📌 Features

* ⚡ Event-driven I/O using `select()`
* 👥 Handles multiple client connections
* 📂 Serves static files over HTTP using `GET`
* 📤 Supports raw file upload using `POST`
* 🌐 Can upload files from phone/browser over local Wi-Fi
* 🧠 Single-threaded design
* 📦 Sends HTTP response headers
* 🧾 Basic MIME type support
* 🛡️ Basic upload filename safety checks
* 🐧 Linux socket API support

---

## 🧱 Tech Stack

* Language: **C**
* OS: **Linux**
* Networking: **POSIX sockets**
* I/O Model: **select()-based multiplexing**
* Protocol: **HTTP/1.1 basics**

---

## 📁 Project Structure

```txt
.
├── server.c
├── public/
│   └── index.html
├── uploads/
├── Makefile
└── README.md
```

---

## 📁 How It Works

1. Server listens on port **5001**
2. Uses `select()` to monitor:

   * New incoming connections
   * Read events from clients
   * Write events for file transfer
3. Parses the HTTP request line:

   * Method
   * Path
   * HTTP version
4. For `GET` requests:

   * Finds the requested file inside `public/`
   * Sends HTTP headers
   * Sends file data in chunks
5. For `POST /upload/<filename>` requests:

   * Reads `Content-Length`
   * Reads raw request body
   * Saves uploaded file inside `uploads/`
6. Closes the connection after response

---

## ▶️ How to Run

### 🔧 Requirements

* Linux
* GCC
* Make

Install build tools on Arch Linux:

```bash
sudo pacman -S --needed base-devel gcc make
```

---

## 🛠️ Compile

```bash
make
```

Or manually:

```bash
gcc -Wall -Wextra -Werror -std=c11 -g server.c -o server
```

---

## ▶️ Run

```bash
make run
```

Or:

```bash
./server
```

Server starts at:

```txt
http://127.0.0.1:5001
```

---

## 📂 Serving Files

Place files inside the `public/` folder.

Example:

```txt
public/index.html
public/style.css
public/image.png
```

Open in browser:

```txt
http://127.0.0.1:5001/
```

or:

```txt
http://127.0.0.1:5001/index.html
```

---

## 📤 Uploading Files

The server supports raw file upload using:

```txt
POST /upload/<filename>
```

Uploaded files are saved inside:

```txt
uploads/
```

Example:

```bash
curl -X POST http://127.0.0.1:5001/upload/test.txt \
  -H "Content-Type: application/octet-stream" \
  --data-binary @test.txt
```

After upload:

```bash
cat uploads/test.txt
```

---

## 📱 Uploading From Phone

Make sure your phone and laptop are connected to the same Wi-Fi.

Find your laptop IP:

```bash
ip addr
```

Example IP:

```txt
192.168.29.169
```

Open this on your phone browser:

```txt
http://192.168.29.169:5001
```

The upload page allows selecting a file from the phone and sending it to the server.

Supported upload examples:

* Images: `.jpg`, `.png`, `.webp`
* Videos: `.mp4`, `.mkv`
* Audio: `.mp3`, `.wav`
* Documents: `.pdf`, `.txt`, `.md`
* Archives: `.zip`, `.tar.gz`
* Source files: `.c`, `.cpp`, `.h`

---

## 🌐 Supported HTTP Methods

| Method | Path               | Description                      |
| ------ | ------------------ | -------------------------------- |
| `GET`  | `/`                | Serves `public/index.html`       |
| `GET`  | `/filename`        | Serves file from `public/`       |
| `POST` | `/upload/filename` | Uploads raw file into `uploads/` |

---

## 🧪 Example Usage

### Download file

```bash
curl http://127.0.0.1:5001/index.html
```

### Upload file

```bash
curl -X POST http://127.0.0.1:5001/upload/notes.txt \
  -H "Content-Type: application/octet-stream" \
  --data-binary @notes.txt
```

---

## ⚠️ Limitations

* Uses `select()`, which does not scale well for very high connection counts
* Fixed client limit using `MAX_USER`
* File I/O is blocking
* HTTP parser is basic
* No authentication
* No HTTPS
* No multipart form-data parser yet
* No persistent connection support
* Uploads should only be used on trusted local networks

---

## 📊 Benchmarking

The earlier Windows WinSock version of this server was benchmarked using **Apache Bench (`ab`)**.

### 🧪 Previous Benchmark: WinSock + select() Version

Test setup:

* OS: Windows
* Server: `127.0.0.1:5001`
* File: `index.html` around 493 bytes
* Tool: Apache Bench (`ab`)

| Concurrency | Requests | Req/sec | Avg Latency | Failed Requests | Status     |
| ----------- | -------: | ------: | ----------: | --------------: | ---------- |
| 1           |      100 |   87.44 |       11 ms |               0 | ✅ Stable   |
| 5           |      500 |  102.46 |       48 ms |               0 | ✅ Stable   |
| 10          |     1000 |  104.74 |       95 ms |               0 | ✅ Stable   |
| 50          |     2000 |   22.57 |     2214 ms |               0 | ❌ Degraded |

### 📌 Observations

* The server was stable up to around **10 concurrent clients**.
* Peak throughput was around **104 requests/sec**.
* At higher concurrency, throughput dropped and latency increased sharply.
* No failed requests were observed, so the server remained functionally correct under load.

### ⚠️ Note

These benchmark results are from the earlier **Windows WinSock implementation**.
The current Linux version has added raw file upload support and should be benchmarked separately.

Future benchmarking should include:

* Static file download performance
* Raw file upload performance
* Different file sizes
* Higher concurrency tests
* Comparison between `select()` and future `epoll` version

---

## 🚀 Future Improvements

* Replace `select()` with `epoll`
* Improve HTTP request parsing
* Add proper request state machine
* Add multipart form-data upload support
* Add upload size limits
* Add authentication for uploads
* Add logging
* Add better MIME type detection
* Add directory listing
* Add config file support
* Add benchmarking results for the Linux version

---

## 🧠 What I Learned

* Linux socket programming in C
* Difference between Windows WinSock and POSIX sockets
* Using `select()` for handling multiple clients
* HTTP request parsing basics
* Serving static files over HTTP
* Handling `GET` and `POST` requests
* Reading `Content-Length`
* Saving raw uploaded files
* Testing server from browser, curl, and phone
* Understanding limitations of `select()`-based servers

---

## 🎯 Conclusion

This project is a simple but practical HTTP file server built from scratch in C.

It supports both downloading files from the server and uploading files to the server over HTTP. It is not meant to replace production servers like Nginx, but it is a strong learning project for understanding sockets, HTTP, event-driven networking, and server internals.

This project also acts as a foundation for building a more advanced Linux-based server using `epoll`, routing, config parsing, reverse proxying, and load balancing.

---

## 📜 License

MIT License
