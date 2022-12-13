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

/*! \brief Plik main serwera.
 *  \file
 *
 *  Zawiera przykładową implementacje wykorzystania serwera TPK.
 *
 */

#include "server.hpp"

static SERVER* srv; //!< Obiekt serwera.

/*! \brief Funkcja obsługująca sygnały.
 *  \param [in] signal Kod sygnału.
 *
 *  Odbiera sygnał z systemu operacyjnego i zamyka serwer.
 *
 */
void handler(int);

/*! \brief Funkcja główna programu serwera.
 *  \returns Kod błędu.
 *  \param [in] argc Liczba argumentów.
 *  \param [in] argv Lista argumentów.
 *
 *  Rejestruje obsługę zdarzeń i uruchamia serwer.
 *
 */
int main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	// Utworzenie serwera
	srv = new SERVER();

	// Rejestracja obsługi sygnałów przez funkcję `handler`
	signal(SIGABRT, handler); // Błąd krytyczny (np. libc)
	signal(SIGINT, handler); // Kombinacja CTRL+C w terminalu
	signal(SIGTERM, handler); // Proces zakończony (np. kill)

	if (!srv->start()) cout << "FAIL\n";
	else while (srv->loop());

	delete srv;

	return 0;
}

void handler(int signal)
{
	srv->end(signal); // Zakończ pętlę główną
}
