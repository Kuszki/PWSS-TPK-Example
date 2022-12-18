/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Simple file transfer project example                                   *
 *  Copyright (C) 2021  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the  Free Software Foundation, either  version 3 of the  License, or   *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This  program  is  distributed  in the hope  that it will be useful,   *
 *  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have  received a copy  of the  GNU General Public License   *
 *  along with this program. If not, see http://www.gnu.org/licenses/.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*! \brief Plik źródłowy serwera.
 *  \file
 *
 */

#include "server.hpp"

SERVER::SERVER(void)
{
	cout << "Constructing server...\tOK\n";
}

SERVER::~SERVER(void)
{
	cout << "Destroying server...\tOK\n";
}

bool SERVER::start(const string& addr, const uint16_t port, const int queue)
{
	static const int yes = 1; // Zmienna do ustawienia opcji `SO_REUSEADDR`
	if (m_sock) this->stop(); // Zatrzymaj serwer, jeśli jest aktywny

	// Stwórz socket - IPv4, TCP
	cout << "Creating socket...\t";
	int sock = ::socket(PF_INET, SOCK_STREAM, 0);

	if (sock == -1) return false;
	else cout << "OK\n";

	sockaddr_in sin;
	int res;

	// Uzupełnij strukturę adresu
	memset(&sin, 0, sizeof(sin));
	sin.sin_port = ::htons(port);
	sin.sin_family = AF_INET;

	cout << "Filling struct...\t";

	// Konwertuj adres z łańcucha do liczby
	res = inet_pton(AF_INET, addr.c_str(), &(sin.sin_addr));

	if (res <= 0) return false;
	else cout << "OK\n";

	cout << "Setting options...\t";

	// Ustaw opcję ponownego użycia adresu
	res = ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (res == -1) return false;
	else cout << "OK\n";

	cout << "Binding address...\t";

	// Połącz gniazdo z adresem
	res = ::bind(sock, (sockaddr*) &sin, sizeof(sin));

	if (res == -1) return false;
	else cout << "OK\n";

	cout << "Start listening...\t";

	// Rozpocznij nasłuchiwanie
	res = ::listen(sock, queue);

	if (res == -1) return false;
	else cout << "OK\n\n";

	// Uzupełnij pola i dodaj gniazdo do listy `poll`
	m_terminate = false;
	m_sock = sock;
	m_sockets.push_back(
	{
		sock,
		POLLIN,
		0
	});

	return true;
}

void SERVER::stop(void)
{
	cout << "Stopping server...\t";

	m_sockets.clear(); // Wyczyść listę `poll`
	m_clients.clear(); // Wyczyść listę klientów

	this->close(); // Zamknij gniazdo

	cout << "OK\n";

	// Wszystkie połączenia klientów zostaną
	// automatycznie zamknięte. Zajmie się
	// tym destruktor struktury CLIENT::~CLIENT()
}

void SERVER::end(int signal)
{
	cout << "\nTerminating server:\t" << signal << '\n';

	m_terminate = true; // Zakończ po kolejnej pętli
}

bool SERVER::loop(int timeout)
{
	// Sprawdź stan połączeń pod kątem możliwości ich obsługi
	if (poll(m_sockets.data(), m_sockets.size(), timeout) > 0)
	{
		// Pierwszy element na liście to serwer
		const auto& serv = m_sockets.front();

		// Jeśli serwer jest gotowy do odczytu (czeka nowy klient)
		if (serv.revents & POLLIN)
		{
			sockaddr_in sin; // Struktura pomocnicza na adres
			socklen_t size = sizeof(sin); // Długość adresu

			// Akceptuj nowe połączenie do serwera
			int sock = ::accept(serv.fd, (sockaddr*) &sin, &size);

			// Jeśli połączenie jest prawidłowe dodaj je do listy klientów
			if (sock != -1) on_accept(sock);
		}

		// Zacznij iteracje od pierwszego klienta
		auto i = m_sockets.begin() + 1;

		// Obsługuj kolejne połączenia aż do końca listy
		while (i != m_sockets.end())
		{
			// Jeśli w połączeniu wystąpił błąd/zostało zamknięte - zwolnij zasoby
			if (i->revents & (POLLHUP | POLLERR)) i = on_disconnect(i);

			// Jeśli połączenie jest gotowe do odczytu i oczekuje się na nagłówek,
			// pobierz jego fragment i przetwórz go w celu skompletowania nagłówka
			else if (m_clients[i->fd].state == STATE::Waiting &&
				    i->revents & POLLIN) i = on_header(i);

			// Jeśli połączenie jest gotowe do odczytu i oczekuje się na dane pliku,
			// pobierz kolejny fragment i zapisz go do pliku związanego z klientem
			else if (m_clients[i->fd].state == STATE::Uploading &&
				    i->revents & POLLIN) i = on_upload(i);

			// Jeśli połączenie jest gotowe do zapisu i dostępne są dane w pliku,
			// odczytaj kolejny fragment danych i wyślij go do oczekującego klienta
			else if (m_clients[i->fd].state == STATE::Downloading &&
				    i->revents & POLLOUT) i = on_download(i);

			else ++i; // Jeśli nie trzeba podejmować żadnej akcji przejdź do kolejnego klienta
		}
	}

	return !m_terminate; // Zwróć stan serwera - `false` gdy trzeba zakończyć serwer
}

bool SERVER::is_started(void) const
{
	return m_sock > 0;
}

void SERVER::on_accept(int sock)
{
	cout << "Accepted client:\t" << sock << '\t'
		<< '(' << get_name(sock) << ')' << '\n';

	m_sockets.push_back({ sock, POLLIN | POLLHUP, 0 }); // Dodaj socket do listy `poll`
	m_clients.insert({ sock, sock }); // Dodaj klienta do listy klientów
}

SERVER::ITERATOR SERVER::on_header(SERVER::ITERATOR it)
{
	auto& client = m_clients[it->fd]; // Obiekt bieżącego klienta

	// Gdy w buforze na nagłówek jest mniej niż 32 B wolnego miejsca
	// rozszerz bufor nagłówka o dodatkowe 64 B na dane.
	if (client.cap - client.size < 32)
		client.resize(client.cap + 64);

	// Jeśli nie istnieje poprawny bufor na nagłówek - zakończ połączenie
	if (!client.buff) return on_disconnect(it);

	cout << "Recv header chunk from:\t" << it->fd << '\t';

	// Dopisz odebrane dane do bufora, przy czym pobierz maksymalnie
	// tyle bajtów danych, ile jest wolnego miejsca w buforze
	ssize_t rec = ::recv(it->fd,
					 client.buff + client.size,
					 client.cap - client.size,
					 0);

	cout << '(' << rec << " B" << ')' << '\n';

	// Jeśli wystąpił błąd lib zamknięto połączenie - zakończ połączenie
	// W przeciwnym razie zaktualizuj rozmiar danych w buforze
	if (rec <= 0) return on_disconnect(it);
	else client.size += rec;

	const auto pos_start = client.buff; // Początek bufora
	const auto pos_end = pos_start + client.size; // Koniec bufora
	const auto pos_nl = find(pos_start, pos_end, '\n'); // Pozycja nowej linii

	// Jeśli znaleziono w buforze znak nowej linii
	if (pos_nl != pos_end)
	{
		const auto pos_sp = find(pos_start, pos_nl, ' '); // Pozycja spacji

		// Jeśli nie znaleziono spacji (brak parametru) - zakończ połączenie
		// W przeciwnym razie zamień spację i znak nowej linii na '\0'
		if (pos_sp == pos_nl) return on_disconnect(it);
		else *pos_nl = *pos_sp = '\0';

		cout << "Completed header for:\t" << it->fd << '\t'
			<< '(' << pos_start << ':' << pos_sp+1 << ')' << '\n';

		// Oblicz ile danych znajduje się za nagłówkiem
		// ilość = rozmiar_danych - pozycja_nl - pozycja_start - 1
		const int left = client.size - (pos_nl - pos_start) - 1;

		// Jeśli komunikat to "UPLOAD"
		if (strcmp(pos_start, "UPLOAD") == 0)
		{
			// Pobierz jedynie nazwę pliku - ścieżka zostanie odrzucona
			const string name = filesystem::path(pos_sp + 1).filename();

			// Otwórz do zapisu plik o zadanej w parametrze nazwie i utnij go
			client.file.open(name, ios_base::out | ios_base::trunc | ios_base::binary);

			// Jeśli nie udało się otworzyć pliku - zakończ połączenie
			// W przeciwnym razie zapisz dane za nagłówkiem (jeśli są)
			if (!client.file.is_open()) return on_disconnect(it);
			else if (left > 0) client.file.write(pos_nl + 1, left);

			client.state = STATE::Uploading; // Zmień stan na odbiór pliku.
		}

		// Jeśli komunikat to "DOWNLOAD"
		else if (strcmp(pos_start, "DOWNLOAD") == 0)
		{
			// Pobierz jedynie nazwę pliku - ścieżka zostanie odrzucona
			const string name = filesystem::path(pos_sp + 1).filename();

			// Otwórz do odczytu plik o zadanej w parametrze nazwie
			client.file.open(name, ios_base::in | ios_base::binary);

			// Jeśli nie udało się otworzyć pliku - zakończ połączenie
			if (!client.file.is_open()) return on_disconnect(it);

			client.state = STATE::Downloading; // Zmień stan na wysyłanie pliku.

			// Od teraz sprawdzaj tylko gotowość do zapisu danych
			it->events = (it->events & ~POLLIN) | POLLOUT;
		}

		// Jeśli nie rozpoznano komunikatu zamknij połączenie
		else return on_disconnect(it);

		client.clean(); // Wyczyść bufor na nagłówek - nie będzie już potrzebny
	}

	return ++it; // Zwróć iterator na kolejne połączenie
}

SERVER::ITERATOR SERVER::on_upload(ITERATOR it)
{
	cout << "Recv file chunk from:\t" << it->fd << '\t';

	// Odczytaj fragment pliku od klienta
	ssize_t rec = ::recv(it->fd, m_buff, sizeof(m_buff), 0);

	cout << '(' << rec << " B" << ')' << '\n';

	// Jeśli nie udało się odczytać żadnych danych - zakończ połączenie
	// W przeciwnym razie zapisz dane do pliku związanego z klientem
	if (rec <= 0) return on_disconnect(it);
	else m_clients[it->fd].file.write(m_buff, rec);

	return ++it; // Zwróć iterator na kolejne połączenie
}

SERVER::ITERATOR SERVER::on_download(ITERATOR it)
{
	auto& file = m_clients[it->fd].file; // Plik związany z klientem

	// Jeśli są jeszcze dane do odczytu to je obsłuż
	// Jeśli nie - zamknij połączenie
	if (!file.eof())
	{
		cout << "Sending file chunk to:\t" << it->fd << '\t';

		// Odczytaj fragment danych
		file.read(m_buff, sizeof(m_buff));

		// Pobierz odczytaną liczbę bajtów
		int rc = file.gcount();

		// Gdy nie odczytano danych - zakończ połączenie
		if (rc <= 0) return on_disconnect(it);

		// Wyślij wszystkie odczytane dane
		int sd = ::send(it->fd, m_buff, rc, 0);

		cout << '(' << sd << '/' << rc << " B" << ')' << '\n';

		// Sprawdź, czy udało się wysłać dane
		if (sd <= 0) return on_disconnect(it);

		// W przypadku niepełnego wysyłania cofnij
		// wskaźnik pliku do prawidłowego miejsca
		else if (sd < rc) file.seekg(sd - rc, ios::cur);
	}
	else return on_disconnect(it); // Zamknij połączenie

	return ++it; // Zwróć iterator na kolejne połączenie
}

SERVER::ITERATOR SERVER::on_disconnect(SERVER::ITERATOR it)
{
	cout << "Disconnecting client:\t" << it->fd << '\t'
		<< '(' << get_name(it->fd) << ')' << '\n';

	// Usuń obiekt klienta z mapy
	m_clients.erase(it->fd);

	// Usuń klienta z listy `poll` oraz
	// zwróć iterator na kolejne połączenie
	return m_sockets.erase(it);
}

SERVER::CLIENT::CLIENT(int fd)
: CLIENT() // Zainicjuj obiekt
{
	sock = fd; // Zainicjuj deskryptor gniazda
}

SERVER::CLIENT::CLIENT(void)
{
	buff = (char*) ::malloc(cap); // Zarezerwuj pamięć na bufor
}

SERVER::CLIENT::~CLIENT(void)
{
	if (buff) ::free(buff); // Jeśli bufor istnieje - zwolnij go
	if (sock) ::close(sock); // Jeśli gniazdo jest aktywne - zamknij je
}

SERVER::CLIENT::CLIENT(CLIENT&& c)
{
	file = move(c.file); // Przenieś obiekt reprezentujący plik

	state = c.state; // Skopiuj stan
	buff = c.buff; // Przenieś bufor
	size = c.size; // Skopiuj rozmiar
	cap = c.cap; // Skopiuj pojemność
	sock = c.sock; // Skopiuj deskryptor gniazda

	c.buff = nullptr; // Wyzeruj wskaźnik na bufor (został przeniesiony)
	c.sock = 0; // Wyzeruj deskryptor gniazda (zostało przeniesione)
}

char* SERVER::CLIENT::resize(size_t new_size)
{
	char* cpy = buff; // Skopiuj wskaźnik na dane (przyda się w przypadku błędu)

	// Gdy klient przekroczy dozwolony rozmiar bufora
	if (new_size > 1024) buff = nullptr; // Nie alokuj więcej danych
	else
	{
		size = min(size, new_size); // Oblicz nowy rozmiar danych
		cap = new_size; // Zapisz nową pojemność bufora

		buff = (char*) ::realloc(buff, new_size); // Realokuj bufor
	}

	// Jesli alokacja się nie udała
	if (!buff)
	{
		size = cap = 0; // Zapisz informacje o braku danych
		buff = nullptr; // Wyzeruj wskaźnik na bufor

		if (cpy) ::free(cpy); // Zwolnij stary bufor
	}

	return buff; // Zwróć wskaźnik na nowy bufor
}

void SERVER::CLIENT::clean(void)
{
	if (buff) ::free(buff); // Jeśli bufor istnieje - zwolnij go
	cap = size = 0; // Zapisz informacje o braku danych
	buff = nullptr; // Wyzeruj wskaźnik na bufor
}
