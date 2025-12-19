#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;
std::mutex mtx;

void translirovanie(const std::string& msg,SOCKET sender=0) {
	std::lock_guard<std::mutex> lock(mtx);
	for (SOCKET s : clients) {
		if (s != sender) {
			send(s, msg.c_str(), msg.length(), 0);
		}
	}
}
void obrabotchik(SOCKET cl) {
	char bufName[256];
	int imbyte = recv(cl, bufName, sizeof(bufName), 0);
	if (imbyte <= 0) {
		closesocket(cl);
		return;
	}
	bufName[imbyte] = '\0';
	std::string name(bufName);
	{
		std::lock_guard<std::mutex> lock(mtx);
		clients.push_back(cl);
	}
	std::cout << "[" << name << "] подключился. Всего: " << clients.size() << "\n";
	translirovanie("[" + name + "] присоединился к чату.\n");
	char mainbuf[1024];
	while (true) {
		int msgbytes = recv(cl, mainbuf, sizeof(mainbuf), 0);
		if (msgbytes > 0) {
			mainbuf[msgbytes] = '\0';
			std::string fullMsg = "[" + name + "] " + std::string(mainbuf) + "\n";
			translirovanie(fullMsg, cl);
		}
		else if (msgbytes == 0) break;
		else {
			std::cerr << "[" << name << "] ошибка соединения: " << "\n";
			break;
		}
	}
	{
		std::lock_guard<std::mutex> lock(mtx);
		clients.erase(
			std::remove(clients.begin(), clients.end(), cl),
			clients.end()
		);
	}
	std::cout << "[" << name << "] отключился. Осталось: " << clients.size() << "\n";
	translirovanie("[" + name + "] покинул чат.\n");
	closesocket(cl);
}
int main() {
	setlocale(LC_ALL, "RU");
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
		std::cerr << "Ошибка инициализации Winsock\n";
		return -1;
	}
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		std::cerr << "Ошибка создания сокета\n";
		WSACleanup();
		return 1;
	}
	sockaddr_in sadr;
	sadr.sin_family = AF_INET;
	sadr.sin_port = htons(12345);
	sadr.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (sockaddr*)&sadr, sizeof(sadr)) == INVALID_SOCKET) {
		std::cerr << "Ошибка связи\n";
		closesocket(s);
		WSACleanup();
		return -1;
	}
	if (listen(s, SOMAXCONN) == INVALID_SOCKET) {
		std::cerr << "Ошибка прослушивания\n";
		closesocket(s);
		WSACleanup();
		return -1;
	}
	while (true) {
		SOCKET client = accept(s, nullptr, nullptr);
		if (client == INVALID_SOCKET) {
			std::cerr << "Ошибка accept\n";
			continue;
		}
		std::thread(obrabotchik, client).detach();
	}
	closesocket(s);
	WSACleanup();
	return 0;
}