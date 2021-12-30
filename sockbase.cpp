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

/*! \brief Plik źródłowy klasy bazowej.
 *  \file
 *
 */

#include "sockbase.hpp"

SOCKBASE::SOCKBASE(void) {}

SOCKBASE::~SOCKBASE(void)
{
	close(); // Zamknij gniazdo
}

void SOCKBASE::close(void)
{
	// Zamknij gniazdo główne
	if (m_sock)
	{
		::close(m_sock);
		m_sock = 0;
	}
}

bool SOCKBASE::send_all(int sock, const char* data, size_t size)
{
	// Gdy są jeszcze dane do wysłania
	while (size > 0)
	{
		// Wyślij brakujące dane
		const int sc = ::send(sock, data, size, 0);

		// W przypadku błędu przerwij działanie
		if (sc <= 0) return false;
		else
		{
			data += sc; // Przesuń wskażnik na dane
			size -= sc; // Zmniejsz liczbę pozostałych danych
		}
	}

	return true;
}

char* SOCKBASE::get_name(int sock)
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);

	// Pobierz dane związane z gniazdem
	getpeername(sock, (sockaddr*) &addr, &len);

	// Konwertuj adres z liczby na łańcuch
	return inet_ntoa(addr.sin_addr);
}
