[![CMake Build](https://github.com/Kuszki/PWSS-TPK-Example/actions/workflows/cmake.yml/badge.svg)](https://github.com/Kuszki/PWSS-TPK-Example/actions/workflows/cmake.yml)

# [TPK-Example](https://kuszki.github.io/tpkexample)

Przykładowy projekt prostej wymiany plików. Umożliwia przesyłanie plików
pomiędzy klientem i serwerem w dwie strony. Jeden transfer pliku
realizowany jest w jednej sesji. Po zakończeniu transferu połączenie
zostaje zamknięte.

W celu wymiany danych należy wysłać do serwera nagłówek z informacją o
działaniu `UPLOAD|DOWNLOAD nazwa_pliku` a następnie oczekiwać na dane
(`DOWNLOAD`) lub wysyłać dane pliku (`UPLOAD`).

Projekt jest stworzony w celach dydaktycznych jako prezentacja
wykorzystania gniazd sieciowych oraz biblioteki standardowej C++.

## Biblioteka TPK

Biblioteka implementuje działanie klienta i serwera dostarczając dwa
obiekty umożliwiające wykorzystanie funkcji projektu.

Serwer umożliwia obsługę wielu połączeń jednocześnie wykorzystując
funkcję `poll`. Istnieje możliwość zastosowania serwera w wątku
realizującym dodatkowe zadania poprzez wywoływanie metody `loop`
pomiędzy pozostałymi zadaniami.

Klient umożliwia połączenie z serwerem i transfer pliku. Przed każdym
transferem należy połączyć się z serwerem. Na końcu transferu połączenie
jest automatycznie zamykane.

## Program TPK_serwer

Przykładowe wykorzystanie serwera. W przykładzie pokazano jak obsłużyć
poprawnie zdarzenie zamknięcia programu (SIGTERM, SIGHUP itd.). Serwer
działa w nieskończonej pętli do czasu otrzymania zdarzenia zamykającego
program. Po otrzymaniu zdarzenia serwer dostaje informacje by nie
obsługiwać więcej połączeń i zakończyć działanie.

## Program TPK_klient

Przykładowe wykorzystanie klienta. W przykładzie pokazano jak wygodnie
parsować argumenty programu przy użyciu `argp`. Po uruchomieniu program
analizuje przekazane parametry i podejmuje wybrane działanie.

Użycie: `TPK_klient [OPCJE...] PLIK [PLIK_LOKALNY]`.

W celu wyświetlenia komunikatu pomocy należy uruchomić program z
parametrem `--help` lub `-?`.

## Dokumentacja

Do projektu dołączono plik `Doxyfile` służący do wygenerowania
dokumentacji programem `Doxygen`. Wszystkie pliki projektu zawierają
odpowiednio przygotowane komentarze na podstawie których generowana
jest dokumentacja w formacie `HTML` oraz `PDF`. Link do dokumentacji
znajduje się [tutaj](https://kuszki.github.io/tpkexample).

## Budowanie

W celu zbudowania projektu należy skonfigurować narzędzia: `CMake`,
`git` oraz dowolny kompilator C++ (`GCC`, `clang`), a następnie:

- sklonować projekt: `git clone https://github.com/Kuszki/TPK-Example`,
- utworzyć katalog budowania: `mkdir build-TPK-Example`,
- przejść do katalogu budowania: `cd build-TPK-Example`,
- skonfigurować projekt: `cmake ../TPK-Example`,
- zbudować projekt: `cmake --build .`.
