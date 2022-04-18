## IPC - kolejki komunikat�w


## Zadanie 1. Prosty chat - System V (50%)
Napisz prosty program typu klient-serwer, w kt�rym komunikacja zrealizowana jest za pomoc� kolejek komunikat�w.
Serwer po uruchomieniu tworzy now� kolejk� komunikat�w systemu V. Za pomoc� tej kolejki klienci b�d� wysy�a� komunikaty do serwera. Wysy�ane zlecenia maj� zawiera� rodzaj zlecenia jako rodzaj komunikatu oraz informacj� od kt�rego klienta zosta�y wys�ane (ID klienta), w odpowiedzi rodzajem komunikatu ma by� informacja identyfikuj�ca czekaj�cego na ni� klienta.
Klient bezpo�rednio po uruchomieniu tworzy kolejk� z unikalnym kluczem IPC i wysy�a jej klucz komunikatem do serwera (komunikat INIT). Po otrzymaniu takiego komunikatu, serwer otwiera kolejk� klienta, przydziela klientowi identyfikator (np. numer w kolejno�ci zg�osze�) i odsy�a ten identyfikator do klienta (komunikacja w kierunku serwer->klient odbywa si� za pomoc� kolejki klienta). Po otrzymaniu identyfikatora, klient mo�e wys�a� zlecenie do serwera(zlecenia s� czytane ze standardowego wyj�cia w postaci typ_komunikatu).


Rodzaje zlece�
* LIST:
Zlecenie wypisania listy wszystkich aktywnych klient�w.
* 2ALL string:
Zlecenie wys�ania komunikatu do wszystkich pozosta�ych klient�w. Klient wysy�a ci�g znak�w. Serwer wysy�a ten ci�g wraz z identyfikatorem klienta-nadawcy oraz aktualn� dat� do wszystkich pozosta�ych klient�w.
* 2ONE id_klienta string:
Zlecenie wys�ania komunikatu do konkretnego klienta. Klient wysy�a ci�g znak�w podaj�c jako adresata konkretnego klienta o identyfikatorze z listy aktywnych klient�w. Serwer wysy�a ten ci�g wraz z identyfikatorem klienta-nadawcy oraz aktualn� dat� do wskazanego klienta.
* STOP:
Zg�oszenie zako�czenia pracy klienta.  Klient wysy�a ten komunikat, kiedy ko�czy prac�, aby serwer m�g� usun�� z listy jego kolejk�. Nast�pnie ko�czy prac�, usuwaj�c swoj� kolejk�. Komunikat ten wysy�any jest r�wnie�, gdy po stronie klienta zostanie wys�any sygna� SIGINT.
Serwer powinien zapisywa� do pliku czas otrzymania zlecenia, identyfikator klienta i tre�� komunikatu.

Zlecenia powinny by� obs�ugiwane zgodnie z priorytetami,najwy�szy priorytet ma STOP, potem LIST i reszta. Mo�na tego dokona� poprzez sterowanie parametrem MTYPE w funkcji msgsnd.
Poszczeg�lne rodzaje komunikat�w nale�y identyfikowa� za pomoc� typ�w komunikat�w systemu V. Klucze dla kolejek maj� by� generowane na podstawie �cie�ki $HOME. Ma�e liczby do wygenerowania kluczy oraz rodzaje komunikat�w maj� by� zdefiniowane we wsp�lnym pliku nag��wkowym. Dla uproszczenia mo�na za�o�y�, �e d�ugo�� komunikatu jest ograniczona pewn� sta�� (jej definicja powinna znale�� si� w pliku nag��wkowym).
Klient i serwer nale�y napisa� w postaci osobnych program�w (nie korzystamy z funkcji fork). Serwer musi by� w stanie pracowa� z wieloma klientami naraz. Przed zako�czeniem pracy ka�dy proces powinien usun�� kolejk� kt�r� utworzy� (patrz IPC_RMID oraz funkcja atexit). Dla uproszczenia mo�na przyj��, �e serwer przechowuje informacje o klientach w statycznej tablicy (rozmiar tablicy ogranicza liczb� klient�w, kt�rzy mog� si� zg�osi� do serwera).
Serwer mo�e wys�a� do klient�w komunikaty:
* inicjuj�cy prac� klienta
* wysy�aj�cy odpowiedzi do klient�w (kolejki klient�w)
* informuj�cy klient�w o zako�czeniu pracy serwera - po wys�aniu takiego sygna�u i odebraniu wiadomo�ci STOP od wszystkich klient�w serwer usuwa swoj� kolejk� i ko�czy prac�. (kolejki klient�w)
Nale�y obs�u�y� przerwanie dzia�ania serwera lub klienta za pomoc� CTRL+C. Po stronie klienta obs�uga tego sygna�u jest r�wnowa�na z wys�aniem komunikatu STOP.

## Zadanie 2. Prosty chat - POSIX (50%)
Zrealizuj zadanie analogiczne do Zadania 1, wykorzystuj�c kolejki komunikat�w POSIX. Kolejka klienta powinna mie� losow� nazw� zgodn� z wymaganiami stawianymi przez POSIX. Na typ komunikatu mo�na zarezerwowa� pierwszy bajt jego tre�ci. Obs�uga zamykania kolejek analogiczna jak w zadaniu 1, z tym, �e aby mo�na by�o usun�� kolejk�, wszystkie procesy powinny najpierw j� zamkn��. Przed zako�czeniem pracy klient wysy�a do serwera komunikat informuj�cy, �e serwer powinien zamkn�� po swojej stronie kolejk� klienta. Nast�pnie klient zamyka i usuwa swoj� kolejk�. Serwer przed zako�czeniem pracy zamyka wszystkie otwarte kolejki, informuje klient�w, aby usun�li swoje kolejki oraz zamkn�li kolejk� serwera i usuwa kolejk�, kt�r� utworzy�.