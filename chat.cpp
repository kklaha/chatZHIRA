#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

void priem(SOCKET sock) {
	char buf[1024];
	while (true) {
		int bytes = recv(sock, buf, sizeof(buf)-1, 0);
		if (bytes > 0) {
			buf[bytes] = '\0';
			std::cout << "\r" << buf << "\n> " << std::endl;
		}
		else if (bytes == 0) {
			std::cout<< "Сервер отключился или клиент завершил отправку.\n";
			break;
		}
		else {
			std::cerr << "Ошибка приёма:\n";
			break;
		}
	}
}
int main() {
	setlocale(LC_ALL, "RU");
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa)!=0) {
		std::cerr << "Ошибка инициализации Winsock\n";
		return -1;
	}
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		std::cerr << "Ошибка инициализации сокета\n";
		closesocket(s);
		WSACleanup();
		return -1;
	}
	std::cout << "Введите имя: ";
	std::string name;
	std::getline(std::cin, name);
	send(s, name.c_str(), name.length(), 0);
	std::cout << "Чат запущен. Введите сообщения:\n> ";
	std::thread recvThread(priem, s);
	std::string msg;
	while (true) {
		std::getline(std::cin, msg);
		if (msg == "exit") {
			shutdown(s, SD_SEND);
			break;
		}
		std::string fullMsg = name + ": " + msg;
		send(s, fullMsg.c_str(), fullMsg.length(), 0);
		std::cout << "> ";
	}
	recvThread.join();

	// Закрываем сокет полностью
	closesocket(s);
	WSACleanup();
	return 0;

}
