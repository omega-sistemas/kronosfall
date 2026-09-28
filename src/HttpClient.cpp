// HttpClient — implementação sobre WinHTTP.
#include <winsock2.h>
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>
#include "HttpClient.h"
#include <vector>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")

namespace {
std::wstring widen(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

HttpResponse request(const std::string& method, const std::string& host, int port,
                     const std::string& path, const std::string& body,
                     const std::string& bearer, bool useTls) {
    HttpResponse out;
    HINTERNET hSession = WinHttpOpen(L"KronosfallClient/1.0",
                                     WINHTTP_ACCESS_TYPE_NO_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return out;
    // Timeouts (ms): resolve/connect/send/receive — sem isso uma rede ruim trava a
    // thread indefinidamente (e o destrutor do StoreClient esperando por ela).
    WinHttpSetTimeouts(hSession, 4000, 4000, 5000, 5000);

    HINTERNET hConnect = WinHttpConnect(hSession, widen(host).c_str(), (INTERNET_PORT)port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return out; }

    std::wstring wmethod = widen(method);
    std::wstring wpath   = widen(path);
    // WINHTTP_FLAG_SECURE => https (validação de certificado do Windows).
    // Sem ele, conexão em texto puro. Chamar com useTls=true apenas quando a
    // URL de API for https:// (ex.: gateway de produção com certificado real).
    DWORD flags = useTls ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, wmethod.c_str(), wpath.c_str(),
                                            nullptr, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return out; }

    std::wstring headers = L"Content-Type: application/json\r\n";
    if (!bearer.empty()) headers += L"Authorization: Bearer " + widen(bearer) + L"\r\n";

    BOOL ok = WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)-1L,
                                 (LPVOID)(body.empty() ? nullptr : body.data()),
                                 (DWORD)body.size(), (DWORD)body.size(), 0);
    if (ok) ok = WinHttpReceiveResponse(hRequest, nullptr);

    if (ok) {
        DWORD code = 0, sz = sizeof(code);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &code, &sz, WINHTTP_NO_HEADER_INDEX);
        out.status = (int)code;

        for (;;) {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &avail) || avail == 0) break;
            std::vector<char> buf(avail);
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, buf.data(), avail, &read) || read == 0) break;
            out.body.append(buf.data(), read);
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return out;
}
} // namespace

namespace HttpClient {

HttpResponse get(const std::string& host, int port, const std::string& path,
                 const std::string& bearer, bool useTls) {
    return request("GET", host, port, path, "", bearer, useTls);
}

HttpResponse post(const std::string& host, int port, const std::string& path,
                  const std::string& jsonBody, const std::string& bearer, bool useTls) {
    return request("POST", host, port, path, jsonBody, bearer, useTls);
}

void openBrowser(const std::string& url) {
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

} // namespace HttpClient
