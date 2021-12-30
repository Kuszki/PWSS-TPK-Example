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

#include <stdlib.h>
#include <argp.h>

// Wersja programu dla argp
const char* argp_program_version = "TPK Client 1.0";

// Email zgłoszenia błędu dla argp
const char* argp_program_bug_address = "<lukasz.drozdz@polsl.pl>";

// Opis programu dla argp
static char doc[] = "Client program for TPK project";

// Opis parametrów dla argp
static char args_doc[] = "FILE [LOCALFILE]";

// Struktura parametrów dla argp
static struct argp_option options[] =
{
	{ "download",	'd',	0,		0, "Download selected file" },
	{ "upload",	'u',	0,		0, "Upload selected file" },
	{ "port",		'p',	"PORT",	0, "Select port number (default is 8080)" },
	{ "host",		'h',	"HOST",	0, "Select hostname (default is localhost)" },
	{ 0 }
};

/*! \brief Struktura opisujaca argumenty.
 *  \see parse_opt.
 *
 *  Przechowuje wartości wszystkich argumentów programu w wygodnej do użytku formie.
 *  Jest uzupełniana przez odpowiednią funkcję podczas parsowania argumentów.
 *
 */
struct arguments
{
	/*! \brief Enumeracja opcji.
	 *
	 *  Opisuje czynność, którą ma wykonać klient.
	 *
	 */
	enum modeset
	{
		unknown, //!< Nieznana czynność.
		download, //!< Pobierz plik.
		upload //!< Wyślij plik.
	};

	modeset mode; //!< Wybrana czynność.

	string local; //!< Lokalna ścieżka pliku.
	string file; //!< Nazwa pliku na serwerze.
	string host; //!< Adres serwera.

	uint16_t port; //!< Port serwera.
};

/*! \brief Funkcja przetwarzająca argumenty.
 *  \see arguments.
 *  \returns Kod błędu.
 *  \param [in] key Kod argumentu.
 *  \param [in] arg Wartość argumentu.
 *  \param [in] state Stan argp.
 *
 *  Przetwarza surowe argumenty i na ich podstawie uzupełnia pola struktury z danymi.
 *
 */
static error_t parse_opt(int key, char* arg, argp_state* state)
{
	struct arguments* args = (arguments*) state->input;

	switch (key)
	{
		case 'p':
			args->port = atoi(arg);
			if (!args->port) argp_usage(state);
		break;
		case 'h':
			args->host = arg;
			if (args->host.empty()) argp_usage(state);
		break;

		case 'u':
			if (args->mode != arguments::unknown) argp_usage(state);
			else args->mode = arguments::upload;
		break;
		case 'd':
			if (args->mode != arguments::unknown) argp_usage(state);
			else args->mode = arguments::download;
		break;

		case ARGP_KEY_ARG:
			switch (state->arg_num)
			{
				case 0:
					args->file = arg;
				break;
				case 1:
					args->local = arg;
				break;
				default:
					argp_usage(state);
			}
		break;

		case ARGP_KEY_END:
			if (state->arg_num < 1 || args->file.empty() ||
			    args->mode == arguments::unknown) argp_usage(state);
			else if (args->local.empty()) args->local = args->file;
		break;

		default: return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

// Struktura konfiguracji argp
static struct argp argp = { options, parse_opt, args_doc, doc };

/*! \brief Funkcja główna programu klienta.
 *  \returns Kod błędu.
 *  \param [in] argc Liczba argumentów.
 *  \param [in] argv Lista argumentów.
 *
 *  Przetwarza parametry przekazane do programu i uruchamia odpowiednie działanie.
 *
 */
int main(int argc, char* argv[])
{
	// Wartości domyślne parametrów
	struct arguments args =
	{
		.mode = arguments::unknown,
		.host = "localhost",
		.port = 8080
	};

	// Przetwórz argumenty
	argp_parse(&argp, argc, argv, 0, 0, &args);

	CLIENT cli; // Utwórz klienta

	// Nawiąż połączenie i wykonaj akcję
	if (cli.connect(args.host, args.port)) switch (args.mode)
	{
		case arguments::download:
			cli.download(args.file, args.local);
		break;
		case arguments::upload:
			cli.upload(args.file, args.local);
		break;
		default: return -2;
	}
	else return -1;

	return 0;
}
