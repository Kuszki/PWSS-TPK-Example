/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  {description}                                                          *
 *  Copyright (C) 2020  Łukasz "Kuszki" Dróżdż  lukasz.kuszki@gmail.com    *
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

#include "client.hpp"

CLIENT::CLIENT(void)
{
	cout << "Constructing client...\tOK\n";
}

CLIENT::~CLIENT(void)
{
	cout << "Destroying client...\tOK\n";
}

bool CLIENT::connect(const string& addr, const uint16_t port)
{
	if (m_sock) this->disconnect();

	const string ports = to_string(port);
	int sockfd(0);

	addrinfo hints, *servinfo, *p;

	// Uzupełnij strukturę podpowiedzi
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	cout << "Translating host...\t";

	// Pobierz informacje o hoście zgodnie z podpowiedziami
	const int ret = getaddrinfo(addr.c_str(), ports.c_str(), &hints, &servinfo);

	if (ret != 0) return false;
	else cout << "OK\n";

	cout << "Connecting socket...\t";

	// Wyszukaj pierwszy pasujący adres
	for (p = servinfo; p != nullptr; p = p->ai_next)
	{
		// Na podstawie danych spróbuj utworzyć gniazdo
		if ((sockfd = ::socket(p->ai_family,
						   p->ai_socktype,
						   p->ai_protocol)) == -1)
		{
			sockfd = 0; // Gdy błąd - przejdź dalej
			continue; // do kolejnego elementu
		}

		// Spróbuj nawiązać połaczenie
		if (::connect(sockfd,
				    p->ai_addr,
				    p->ai_addrlen) == -1)
		{
			::close(sockfd); // Gdy błąd - zamknij
			sockfd = 0; // gniazdo po czym przejdź
			continue; // dalej do kolejnego elemenut
		}
		else break; // Gdy wszystko OK pomiń resztę elementów
	}

	// Zwolnij zasoby informacji o adresie
	freeaddrinfo(servinfo);

	if (sockfd == 0) return false;
	else cout << "OK\n";

	this->m_sock = sockfd;

	return true;
}

void CLIENT::disconnect(void)
{
	close(); // Zamknij gniazdo
}

int CLIENT::download(const string& path, const string& dest)
{
	cout << "Opening local file...\t";

	// Pobierz nazwę pliku i otwórz lokalny plik
	const string name = filesystem::path(path).filename();
	fstream file(dest, ios_base::out | ios_base::trunc | ios_base::binary);

	if (!file.is_open()) return -1;
	else cout << "OK\n";

	// Wygeneruj nagłówek
	const string header = "DOWNLOAD " + name + '\n';

	size_t count(0); // Licznik wszystkich danych
	ssize_t rec(0); // Licznik danych w pakiecie

	cout << "Downloading file...\t";

	// Wyślij nagłówek do serwera
	if (send_all(m_sock, header.c_str(), header.size()))
	{
		// Odbierz fragment pliku z serwera
		while ((rec = ::recv(m_sock, m_buff, sizeof(m_buff), 0)) > 0)
		{
			file.write(m_buff, rec); // Zapisz go do pliku
			count += rec; // Dodaj do licznika długość danych
		}

		// W przypadku błędu podczas odbierania lub zamknięcia
		// połaczenia przez serwer - rozłącz się
		if (rec <= 0) disconnect();
	}

	// Rozłącz się po wykonaniu zadania
	this->disconnect();

	if (count) cout << "OK\n";
	else cout << "FAIL\n";

	return count;
}

int CLIENT::upload(const string& path, const string& src)
{
	cout << "Opening local file...\t";

	// Pobierz długość pliku i otwórz lokalny plik
	const auto size = filesystem::file_size(src);
	fstream file(src, ios_base::in | ios_base::binary);

	if (!file.is_open()) return -1;
	else cout << "OK\n";

	// Pobierz nazwę pliku i wygeneruj nagłówek
	const string name = filesystem::path(path).filename();
	const string header = "UPLOAD " + name + '\n';

	size_t count(0); // Licznik wszystkich danych
	bool fail = false; // Stan błędu

	cout << "Uploading file...\t";

	// Wyślij nagłówek do serwera
	if (send_all(m_sock, header.c_str(), header.size()))
	{
		// Odczytuj plik i wysyłaj fragmenty do serwera
		while (!file.eof() && !fail)
		{
			file.read(m_buff, sizeof(m_buff)); // Odczytaj fragment
			const size_t chunk = file.gcount(); // Pobierz jego rozmiar

			if (chunk <= 0) break; // Gdy nic nie odczytano przerwij pętlę
			else if (send_all(m_sock, m_buff, chunk)) count += chunk;
			else break; // Gdy nue udało się wysłać danych przerwij pętlę
		}
	}

	// Rozłącz się po wykonaniu zadania
	this->disconnect();

	if (count == size) cout << "OK\n";
	else cout << "FAIL\n";

	return count;
}

bool CLIENT::is_connected(void) const
{
	return m_sock > 0;
}
