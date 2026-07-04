# 🚀 Event-Driven HTTP File Server in C

A lightweight **HTTP file server** implemented in pure C using Linux sockets and the `select()` system call.

This project demonstrates low-level networking, event-driven architecture, static file serving, basic HTTP parsing, and raw file upload handling without using any external web framework.

---

## 📌 Features

* ⚡ Event-driven I/O using `select()`
* 👥 Handles multiple client connections
* 📂 Serves files over HTTP using `GET`
* 📤 Supports raw file upload using `POST`
* 🌐 Can upload files from phone/browser over local Wi-Fi
* 🧠 Single-threaded design
* 📦 Sends HTTP response headers
* 🧾 Basic MIME type support
* 🛡️ Basic path traversal protection
* 🛡️ Basic upload filename safety checks
* 🐧 Linux POSIX socket API support

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

`public/index.html` is used as the default page for `/`.

Uploaded files are saved inside the `uploads/` folder.

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

   * `GET /` serves `public/index.html`
   * `GET /filename` serves a file from the server's current working directory
   * Sends HTTP response headers
   * Sends file data in chunks
5. For `POST /upload/<filename>` requests:

   * Reads `Content-Length`
   * Reads the raw request body
   * Saves the uploaded file inside `uploads/`
6. Closes the connection after the response

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

Create required folders:

```bash
mkdir -p public uploads
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

The server supports file serving using `GET`.

Opening:

```txt
http://127.0.0.1:5001/
```

serves:

```txt
public/index.html
```

Opening:

```txt
http://127.0.0.1:5001/example.txt
```

serves:

```txt
example.txt
```

from the server's current working directory.

Note: In the current version, only the default homepage is served from `public/index.html`. Other requested files are opened relative to the directory where the server is running.

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

| Method | Path               | Description                               |
| ------ | ------------------ | ----------------------------------------- |
| `GET`  | `/`                | Serves `public/index.html`                |
| `GET`  | `/filename`        | Serves file from current server directory |
| `POST` | `/upload/filename` | Uploads raw file into `uploads/`          |

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

## 🧾 MIME Type Support

The server includes basic MIME type handling for common file types:

| Extension        | Content-Type               |
| ---------------- | -------------------------- |
| `.html`          | `text/html`                |
| `.css`           | `text/css`                 |
| `.js`            | `application/javascript`   |
| `.png`           | `image/png`                |
| `.jpg` / `.jpeg` | `image/jpeg`               |
| `.txt`           | `text/plain`               |
| `.pdf`           | `application/pdf`          |
| unknown          | `application/octet-stream` |

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

## ⚠️ Limitations

* Uses `select()`, which does not scale well for very high connection counts
* Fixed client limit using `MAX_USER`
* File I/O is blocking
* HTTP parser is basic
* No authentication
* No HTTPS
* No multipart form-data parser yet
* No persistent connection support
* No fixed public root directory for all static files yet
* No advanced path normalization beyond basic checks
* Uploads should only be used on trusted local networks

---

## 🚀 Future Improvements

* Replace `select()` with `epoll`
* Improve HTTP request parsing
* Add a proper request state machine
* Serve all static files from a fixed root directory
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
* Serving files over HTTP
* Handling `GET` and `POST` requests
* Reading `Content-Length`
* Saving raw uploaded files
* Testing the server from browser, curl, and phone
* Understanding limitations of `select()`-based servers

---

## 🎯 Conclusion

This project is a simple but practical HTTP file server built from scratch in C.

It supports both downloading files from the server and uploading files to the server over HTTP. It is not meant to replace production servers like Nginx, but it is a strong learning project for understanding sockets, HTTP, event-driven networking, and server internals.

This project also acts as a foundation for building a more advanced Linux-based server using `epoll`, routing, config parsing, reverse proxying, and load balancing.

---

## 📜 License

MIT License
