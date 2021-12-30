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

#ifndef SERVER_H
#define SERVER_H

#include "sockbase.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <map>

using namespace std;

/*! \brief Klasa serwera.
 *
 *  Klasa reprezentująca serwer TPK.
 *
 */
class SERVER : public SOCKBASE
{

	protected:

		/*! \brief Enumeracja maszyny stanów.
		 *
		 *  Określa aktualny stan połaczenia obsługiwanego przez serwer.
		 *
		 */
		enum class STATE
		{
			Waiting, //!< Oczekiwanie na nagłówek.
			Uploading, //!< Odbieranie danych z klienta.
			Downloading //!< Wysyłanie danych do klienta.
		};

		/*! \brief Struktura opisująca klienta.
		 *  \see STATE.
		 *
		 *  Przechowuje dane związane z obsługą klienta.
		 *
		 */
		struct CLIENT
		{
			STATE state = STATE::Waiting; //!< Status klienta.

			char* buff = nullptr; //!< Bufor na nagłówek.
			size_t size = 0; //!< Liczba zgromadzonych danych.
			size_t cap = 64; //!< Długość bufora.

			fstream file; //!< Plik powiązany z klientem.

			int sock = 0; //!< Gniazdo połączenia.

			CLIENT(int fd); //!< Konstruktor konwertujący.
			CLIENT(void); //!< Domyślny konstruktor.
			~CLIENT(void); //!< Destruktor klienta.

			CLIENT(const CLIENT&) = delete; //!< Konstruktor kopiujacy (usunięty).
			CLIENT(CLIENT&& c); //!< Konstruktor przenoszący.

			CLIENT& operator= (CLIENT&) = delete; //!< Operator przypisania (kopia, usunięty)
			CLIENT& operator= (CLIENT&& c); //!< Operator przypisania (przeniesienie)

			/*! \brief Zmiana rozmiaru bufora na nagłówek.
			 *  \see buff.
			 *  \returns Nowy wskaźnik na nagłówek.
			 *  \param [in] new_size Nowy rozmiar bufora.
			 *
			 *  Zmienia rozmiar bufora na zadany. W przypadku mniejszego rozmiaru
			 *  dane zostaną uciete. W przypadku niepowodzenia zwraca pusty wskaźnik.
			 *
			 */
			char* resize(size_t new_size);

			/*! \brief Czyści bufor na nagłówek.
			 *  \see buff, resize.
			 *
			 *  Zwalnia zasoby związane z nagłówkiem. Warto wywołać gdy nagłówek
			 *  zostanie w pełni skompletowany.
			 *
			 */
			void clean(void);
		};

		map<int, CLIENT> m_clients; //!< Mapa obsługiwanych klientów.
		vector<pollfd> m_sockets; //!< Wektor wszystkich monitorowanych gniazd.

		bool m_terminate = false; //!< Flaga zakończenia działania serwera.

	public:

		using iterator = decltype(m_sockets)::iterator; //!< Typ iteratora dla kontenera połączeń.

		explicit SERVER(void); //!< Konstruktor serwera.
		virtual ~SERVER(void) override; //!< Destruktor serwera.

		/*! \brief Inicjacja serwera.
		 *  \see stop.
		 *  \returns Powodzenie operacji.
		 *  \param [in] addr Adres do nasłuchiwania.
		 *  \param [in] port Port do nasłuchiwania.
		 *  \param [in] queue Liczba klientów do skolejkowania.
		 *
		 *  Rozpoczyna pracę serwera ustalając zadane parametry pracy. W przypadku niepowodzenia
		 *  zwraca wartość `false`. Podczas startu wyświetlane są komunikaty informujące o
		 *  aktualnie wykonywanej czynności.
		 *
		 */
		bool start(const string& addr = "0.0.0.0",
				 const uint16_t port = 8080,
				 const int queue = 10);

		/*! \brief Zatrzymuje serwer.
		 *  \see start.
		 *
		 *  Zwalnia zasoby związane z serwerem i zamyka wszystkie połączenia.
		 *
		 */
		void stop(void);

		/*! \brief Zatrzymuje serwer.
		 *  \see m_terminate.
		 *
		 *  Ustala flagę zakończenia pracy serwera.
		 *
		 */
		void end(int signal);

		/*! \brief Pętla serwera.
		 *  \see start.
		 *  \returns `false` gdy odebrano komunikat o zamknieciu serwera, `true` w przeciwnym razie.
		 *  \param [in] timeout Czas oczekiwania na poll.
		 *
		 *  Wykonuje obsługę połączeń oczekując określony czas na aktywność.
		 *
		 */
		bool loop(int timeout = -1);


		/*! \brief Test uruchomienia serwera.
		 *  \see start, stop.
		 *  \returns `true` gdy serwer jest uruchomiony, `false` w przeciwnym razie.
		 *
		 *  Sprawdza, czy serwer został uruchomiony i nasłuchuje połączeń.
		 *
		 */
		bool is_started(void) const;

	protected:

		/*! \brief Obsługa nowego połączenia.
		 *  \see loop.
		 *  \param [in] sock Deskryptor nowego połączenia.
		 *
		 *  Dodaje zaakceptowane połączenie do listy klientów.
		 *
		 */
		void on_accept(int sock);

		/*! \brief Obsługa nagłówka.
		 *  \see loop.
		 *  \returns Iterator kolejnego klienta.
		 *  \param [in] it Iterator obsługiwanego klienta.
		 *
		 *  Pobiera fragment nagłówka, przetwarza go i w razie potrzeby zmienia stan połaczenia.
		 *
		 */
		iterator on_header(iterator it);

		/*! \brief Obsługa wysyłania pliku.
		 *  \see loop.
		 *  \returns Iterator kolejnego klienta.
		 *  \param [in] it Iterator obsługiwanego klienta.
		 *
		 *  Pobiera fragment danych od klienta, zapisuje go i w razie potrzeby zmienia stan połaczenia.
		 *
		 */
		iterator on_upload(iterator it);

		/*! \brief Obsługa pobierania pliku.
		 *  \see loop.
		 *  \returns Iterator kolejnego klienta.
		 *  \param [in] it Iterator obsługiwanego klienta.
		 *
		 *  Wysyła fragment danych do klienta z odczytanego pliku i w razie potrzeby zmienia stan połaczenia.
		 *
		 */
		iterator on_download(iterator it);

		/*! \brief Obsługa rozłączenia klienta.
		 *  \see loop.
		 *  \returns Iterator kolejnego klienta.
		 *  \param [in] it Iterator obsługiwanego klienta.
		 *
		 *  Zamyka połaczenie i zwalnia związane z nim zasoby.
		 *
		 */
		iterator on_disconnect(iterator it);

};

#endif // SERVER_H
