#pragma once
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
// Cross-platform sockets: Winsock on Windows, POSIX everywhere else.
#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET sock_t;
  #define CLOSE_SOCK closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <cstring>
  #ifdef __APPLE__
    #include <mach-o/dyld.h>
  #endif
  typedef int sock_t;
  #define CLOSE_SOCK close
#endif
using namespace std;

// Minimal HTTP/1.1 server on POSIX sockets. No external libraries.
// Single-threaded: handles one request at a time (fine for a demo).
// Routes:
//   /api/...  -> FoodRescueApi::handle()  (JSON in/out)
//   anything else -> static file from ./web/ (index.html for "/")

static string httpStatusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        default: return "OK";
    }
}

static string httpContentType(const string& path) {
    if (path.size() >= 5 && path.substr(path.size()-5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size()-4) == ".css") return "text/css";
    if (path.size() >= 3 && path.substr(path.size()-3) == ".js") return "application/javascript";
    if (path.size() >= 5 && path.substr(path.size()-5) == ".json") return "application/json";
    if (path.size() >= 4 && path.substr(path.size()-4) == ".png") return "image/png";
    return "application/octet-stream";
}

static bool httpSendAll(sock_t fd, const string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
        if (n <= 0) return false;
        sent += (size_t)n;
    }
    return true;
}

// Directory containing the running program (so web/ is found
// no matter which folder the user launches from).
static string exeDir() {
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    string p = (n > 0) ? string(buf, n) : string(".");
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t sz = sizeof(buf);
    string p = (_NSGetExecutablePath(buf, &sz) == 0) ? string(buf) : string(".");
#else
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    string p = (n > 0) ? string(buf, (size_t)n) : string(".");
#endif
    size_t pos = p.find_last_of("/\\");
    return (pos == string::npos) ? "." : p.substr(0, pos);
}

class HttpServer {
    FoodRescueApi* api;
    int port;
public:
    HttpServer(FoodRescueApi* a, int p = 8080) : api(a), port(p) {}

    void run() {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { cerr << "WSAStartup failed\n"; return; }
#endif
        sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
        if (srv == INVALID_SOCKET) { cerr << "socket() failed\n"; return; }
#else
        if (srv < 0) { cerr << "socket() failed\n"; return; }
#endif
        int opt = 1;
        setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (::bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {  // ::bind: avoid clash with std::bind on macOS Clang
            cerr << "bind() failed on port " << port << " (already in use?)\n";
            CLOSE_SOCK(srv); return;
        }
        if (listen(srv, 8) < 0) { cerr << "listen() failed\n"; CLOSE_SOCK(srv); return; }

        cout << "\n==============================================\n";
        cout << "  FoodRescue is running!\n";
        cout << "  Open in your browser: http://localhost:" << port << "\n";
        cout << "  Press Ctrl+C to stop.\n";
        cout << "==============================================\n";
        {   // startup sanity check: is the web folder where the exe is?
            ifstream probe(exeDir() + "/web/index.html", ios::binary);
            if (!probe) {
                cout << "  WARNING: web/index.html not found next to the program.\n"
                     << "  Make sure the 'web' folder sits beside the FoodRescue binary.\n";
            }
        }

        while (true) {
            sock_t cli = accept(srv, nullptr, nullptr);
#ifdef _WIN32
            if (cli == INVALID_SOCKET) continue;
#else
            if (cli < 0) continue;
#endif
            handleClient(cli);
            CLOSE_SOCK(cli);
        }
    }

private:
    // Read until we have full headers + (if present) the full body.
    static string readRequest(sock_t fd) {
        string req;
        char buf[4096];
        size_t headerEnd = string::npos;
        size_t contentLen = 0;
        while (true) {
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, (size_t)n);
            if (headerEnd == string::npos) {
                headerEnd = req.find("\r\n\r\n");
                if (headerEnd != string::npos) {
                    // parse Content-Length from headers
                    string hdrs = req.substr(0, headerEnd);
                    size_t cl = hdrs.find("Content-Length:");
                    if (cl != string::npos) {
                        size_t e = hdrs.find("\r\n", cl);
                        string val = hdrs.substr(cl + 15, e - cl - 15);
                        try { contentLen = (size_t)stoul(val); } catch (...) {}
                    }
                }
            }
            if (headerEnd != string::npos && req.size() >= headerEnd + 4 + contentLen) break;
            if (req.size() > 4 * 1024 * 1024) break; // 4MB cap
        }
        return req;
    }

    void sendResponse(sock_t fd, int code, const string& contentType, const string& body) {
        ostringstream h;
        h << "HTTP/1.1 " << code << " " << httpStatusText(code) << "\r\n"
          << "Content-Type: " << contentType << "\r\n"
          << "Content-Length: " << body.size() << "\r\n"
          << "Access-Control-Allow-Origin: *\r\n"
          << "Connection: close\r\n\r\n";
        httpSendAll(fd, h.str());
        httpSendAll(fd, body);
    }

    static map<string,string> parseHeaders(const string& req) {
        map<string,string> h;
        size_t he = req.find("\r\n\r\n");
        string hdrs = req.substr(0, he == string::npos ? req.size() : he);
        istringstream ss(hdrs);
        string line;
        getline(ss, line); // skip request line
        while (getline(ss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            size_t c = line.find(':');
            if (c == string::npos) continue;
            string name = line.substr(0, c), val = line.substr(c + 1);
            // trim + lowercase the header name
            size_t a = name.find_first_not_of(" \t"), b = name.find_last_not_of(" \t");
            if (a == string::npos) continue;
            name = name.substr(a, b - a + 1);
            for (char& ch : name) ch = (char)tolower(ch);
            a = val.find_first_not_of(" \t"); b = val.find_last_not_of(" \t");
            val = (a == string::npos) ? "" : val.substr(a, b - a + 1);
            h[name] = val;
        }
        return h;
    }

    void handleClient(sock_t fd) {
        string req = readRequest(fd);
        if (req.empty()) return;

        // Request line: METHOD /path HTTP/1.1
        istringstream rl(req);
        string method, path, ver;
        rl >> method >> path >> ver;
        if (method.empty() || path.empty()) return;

        size_t headerEnd = req.find("\r\n\r\n");
        string body = (headerEnd == string::npos) ? "" : req.substr(headerEnd + 4);

        cout << "[HTTP] " << method << " " << path << "\n";

        if (path.rfind("/api/", 0) == 0) {
            map<string,string> params;
            string ctype;
            // find Content-Type header (only need to know if JSON)
            size_t he = req.find("\r\n\r\n");
            string hdrs = req.substr(0, he == string::npos ? req.size() : he);
            if (hdrs.find("application/json") != string::npos && !body.empty()) {
                params = parseJsonBody(body);
            }
            auto res = api->handle(method, path, params, parseHeaders(req));
            sendResponse(fd, res.first, "application/json", res.second);
            return;
        }

        // Static files from <exe-dir>/web/ (works from any launch folder)
        if (path == "/") path = "/index.html";
        if (path.find("..") != string::npos) {
            sendResponse(fd, 404, "text/plain", "Not found");
            return;
        }
        string fpath = exeDir() + "/web" + path;
        ifstream f(fpath, ios::binary);
        if (!f) {
            sendResponse(fd, 404, "text/plain", "Not found: " + path);
            return;
        }
        ostringstream ss; ss << f.rdbuf();
        sendResponse(fd, 200, httpContentType(path), ss.str());
    }
};
