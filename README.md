# 🚀 Event-Driven HTTP File Server in C

A lightweight **non-blocking HTTP file server** implemented in pure C using **WinSock** and the `select()` system call.
This project demonstrates low-level networking, event-driven architecture, and concurrent client handling without multithreading.

---

## 📌 Features

* ⚡ Non-blocking I/O using `select()`
* 👥 Handles multiple client connections
* 📂 Serves files over HTTP
* 🧠 Event-driven design (single-threaded)
* 📦 Sends proper HTTP headers
* 📄 Supports file download via browser

---

## 🧱 Tech Stack

* Language: **C**
* Networking: **WinSock2**
* I/O Model: **select()-based multiplexing**

---

## 📁 How It Works

1. Server listens on port **5001**
2. Uses `select()` to monitor:

   * New incoming connections
   * Read events (client requests)
   * Write events (file transfer)
3. Parses HTTP request (method + file path)
4. Opens requested file
5. Sends:

   * HTTP response headers
   * File data in chunks
6. Closes connection after transfer

---

## ▶️ How to Run

### 🔧 Requirements

* Windows OS
* GCC (MinGW) or Visual Studio

### 🛠️ Compile

```bash
gcc server.c -lws2_32 -o server
```

### ▶️ Run

```bash
./server
```

Server will start at:

```
http://127.0.0.1:5001
```

---

## 📥 Example Usage

Open in browser:

```
http://127.0.0.1:5001/index.html
```

Or using curl:

```bash
curl http://127.0.0.1:5001/index.html --output index.html
```

---

## 📊 Performance & Scalability Testing

The server was benchmarked using **Apache Bench (`ab`)**.

### 🧪 Test Setup

* Server: `127.0.0.1:5001`
* File: `index.html` (493 bytes)
* Tool: Apache Bench (`ab`)

---

### 📈 Results

| Concurrency | Requests | Req/sec | Avg Latency | Failed Requests | Status     |
| ----------- | -------- | ------- | ----------- | --------------- | ---------- |
| 1           | 100      | 87.44   | 11 ms       | 0               | ✅ Stable   |
| 5           | 500      | 102.46  | 48 ms       | 0               | ✅ Stable   |
| 10          | 1000     | 104.74  | 95 ms       | 0               | ✅ Stable   |
| 50          | 2000     | 22.57   | 2214 ms     | 0               | ❌ Degraded |

---

### 📌 Observations

* Stable performance up to **~10 concurrent clients**
* Peak throughput: **~104 requests/sec**
* At higher concurrency:

  * Throughput drops significantly
  * Latency increases sharply
* No failed requests → system remains functionally correct under load

---

### ⚠️ Limitations

* Fixed client limit (`MAX_USER = 10`)
* `select()` does not scale efficiently for large numbers of connections
* Blocking file I/O (`fread`)
* No MIME type handling
* Windows-only (WinSock)

---

### 🚀 Future Improvements

* Replace `select()` with:

  * `epoll` (Linux)
  * IOCP (Windows)
* Add MIME type support
* Improve HTTP parsing
* Implement non-blocking file I/O
* Add logging and monitoring
* Support persistent connections (Keep-Alive)

---

## 🧠 What I Learned

* Low-level socket programming in C
* Event-driven server design
* Handling multiple clients without threads
* HTTP protocol basics
* Performance benchmarking using Apache Bench
* Identifying and analyzing system bottlenecks

---

## 🎯 Conclusion

This project demonstrates a working **event-driven concurrent HTTP server** and highlights the practical limitations of `select()`-based designs under high load.

It serves as a strong foundation for building scalable, high-performance network systems.

---

## 📜 License

MIT License
