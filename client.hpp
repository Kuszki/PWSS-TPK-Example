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

/*! \brief Plik nagłówkowy klienta.
 *  \file
 *
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "sockbase.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <map>

using namespace std;

/*! \brief Klasa klienta.
 *
 *  Klasa reprezentująca klienta TPK.
 *
 */
class CLIENT : public SOCKBASE
{

	public:

		explicit CLIENT(void); //!< Konstruktor domyślny
		virtual ~CLIENT(void) override; //!< Wirtualny destruktor

		/*! \brief Inicjacja połączenia.
		 *  \see disconnect.
		 *  \returns Powodzenie operacji.
		 *  \param [in] addr Adres serwera.
		 *  \param [in] port Port serwera.
		 *
		 *  Nawiązuje połączenie z wybranym serwerem. Zwraca `true` w przypadku
		 *  powodzenia lub `false` w przypadku błędu.
		 *
		 */
		bool connect(const string& addr,
				   const uint16_t port);

		/*! \brief Zamyka połączenie.
		 *  \see start.
		 *
		 *  Zwalnia zasoby związane z klientem i zamyka połączenie z serwerem.
		 *
		 */
		void disconnect(void);

		/*! \brief Pobieranie pliku.
		 *  \see connect.
		 *  \returns Liczba odebranych bajtów.
		 *  \param [in] path Nazwa pliku na serwerze.
		 *  \param [in] dest Lokalna ścieżka pliku.
		 *
		 *  Pobiera plik z połączonego serwera i zapisuje we wskazane miejsce w
		 *  systemie plików. Ze wskazanej ścieżki do pliku źródłowego na serwerze
		 *  pozyskiwana jest jedynie nazwa pliku.
		 *
		 */
		int download(const string& path,
				   const string& dest);

		/*! \brief Wysyłanie pliku.
		 *  \see connect.
		 *  \returns Liczba wysłanych bajtów.
		 *  \param [in] path Nazwa pliku na serwerze.
		 *  \param [in] src Lokalna ścieżka pliku.
		 *
		 *  Wysyła plik do połączonego serwera odczytany z wskazanego miejsca w
		 *  systemie plików. Ze wskazanej ścieżki do pliku docelowego na serwerze
		 *  pozyskiwana jest jedynie nazwa pliku.
		 *
		 */
		int upload(const string& path,
				 const string& src);

		/*! \brief Test nawiązania połączenia.
		 *  \see connect, disconnect.
		 *  \returns `true` gdy połączenie jest aktywne, `false` w przeciwnym razie.
		 *  \warning W przypadku gdy serwer zamknie połączenie status zmieni się dopiero po pierwszej operacji.
		 *
		 *  Sprawdza, czy zostało nawiązane połączenie z serwerem.
		 *
		 */
		bool is_connected(void) const;

};

#endif // CLIENT_H
