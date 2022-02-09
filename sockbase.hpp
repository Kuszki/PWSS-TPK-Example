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

/*! \brief Plik nagłówkowy klasy bazowej.
 *  \file
 *
 */

#ifndef SOCKBASE_HPP
#define SOCKBASE_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <netdb.h>

/*! \brief Klasa bazowa.
 *
 *  Klasa reprezentująca wspólne części serwera i klienta.
 *
 */
class SOCKBASE
{

	protected:

		int m_sock = 0; //!< Gniazdo główne.
		char m_buff[1024]; //!< Ogólny bufor na dane.

	public:

		explicit SOCKBASE(const SOCKBASE&) = delete; //!< Konstruktor kopiujący (usunięty)
		explicit SOCKBASE(SOCKBASE&&) = delete; //!< Konstruktor przenoszący (usunięty)

		explicit SOCKBASE(void); //!< Domyślny konstruktor.
		virtual ~SOCKBASE(void); //!< Wirtualny destruktor.

		void close(void); //!< Zamyka gniazdo.

		SOCKBASE& operator= (const SOCKBASE&) = delete; //!< Operator przypisania (kopia, usunięty)
		SOCKBASE& operator= (SOCKBASE&&) = delete; //!< Operator przypisania (przeniesienie, usunięty)

	protected:

		/*! \brief Wysyłanie danych.
		 *  \returns Powodzenie operacji.
		 *  \param [in] sock Deskryptor gniazda.
		 *  \param [in] data Dane do wysłania.
		 *  \param [in] size Liczba danych w bajtach.
		 *
		 *  Wysyła wskazane dane ponawiając próbę w przypadku niepełnego wysyłania.
		 *  Zwraca `true` w przypadku wysłania wszystkich danych lub `false` w przeciwnym razie.
		 *
		 */
		static bool send_all(int sock, const char* data, size_t size);

		/*! \brief Pobranie nazwy hosta.
		 *  \returns Łańcuch nazwy hosta.
		 *  \param [in] sock Deskryptor gniazda.
		 *
		 *  Pobiera informacje o hoście powiązanym z gniazdem i zwraca jego adres w postaci łańcucha.
		 *
		 */
		static char* get_name(int sock);

};

#endif // SOCKBASE_HPP
