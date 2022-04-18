## IPC - kolejki komunikatów


## Zadanie 1. Prosty chat - System V (50%)
Napisz prosty program typu klient-serwer, w którym komunikacja zrealizowana jest za pomoc¹ kolejek komunikatów.
Serwer po uruchomieniu tworzy now¹ kolejkê komunikatów systemu V. Za pomoc¹ tej kolejki klienci bêd¹ wysy³aæ komunikaty do serwera. Wysy³ane zlecenia maj¹ zawieraæ rodzaj zlecenia jako rodzaj komunikatu oraz informacjê od którego klienta zosta³y wys³ane (ID klienta), w odpowiedzi rodzajem komunikatu ma byæ informacja identyfikuj¹ca czekaj¹cego na ni¹ klienta.
Klient bezpoœrednio po uruchomieniu tworzy kolejkê z unikalnym kluczem IPC i wysy³a jej klucz komunikatem do serwera (komunikat INIT). Po otrzymaniu takiego komunikatu, serwer otwiera kolejkê klienta, przydziela klientowi identyfikator (np. numer w kolejnoœci zg³oszeñ) i odsy³a ten identyfikator do klienta (komunikacja w kierunku serwer->klient odbywa siê za pomoc¹ kolejki klienta). Po otrzymaniu identyfikatora, klient mo¿e wys³aæ zlecenie do serwera(zlecenia s¹ czytane ze standardowego wyjœcia w postaci typ_komunikatu).


Rodzaje zleceñ
* LIST:
Zlecenie wypisania listy wszystkich aktywnych klientów.
* 2ALL string:
Zlecenie wys³ania komunikatu do wszystkich pozosta³ych klientów. Klient wysy³a ci¹g znaków. Serwer wysy³a ten ci¹g wraz z identyfikatorem klienta-nadawcy oraz aktualn¹ dat¹ do wszystkich pozosta³ych klientów.
* 2ONE id_klienta string:
Zlecenie wys³ania komunikatu do konkretnego klienta. Klient wysy³a ci¹g znaków podaj¹c jako adresata konkretnego klienta o identyfikatorze z listy aktywnych klientów. Serwer wysy³a ten ci¹g wraz z identyfikatorem klienta-nadawcy oraz aktualn¹ dat¹ do wskazanego klienta.
* STOP:
Zg³oszenie zakoñczenia pracy klienta.  Klient wysy³a ten komunikat, kiedy koñczy pracê, aby serwer móg³ usun¹æ z listy jego kolejkê. Nastêpnie koñczy pracê, usuwaj¹c swoj¹ kolejkê. Komunikat ten wysy³any jest równie¿, gdy po stronie klienta zostanie wys³any sygna³ SIGINT.
Serwer powinien zapisywaæ do pliku czas otrzymania zlecenia, identyfikator klienta i treœæ komunikatu.

Zlecenia powinny byæ obs³ugiwane zgodnie z priorytetami,najwy¿szy priorytet ma STOP, potem LIST i reszta. Mo¿na tego dokonaæ poprzez sterowanie parametrem MTYPE w funkcji msgsnd.
Poszczególne rodzaje komunikatów nale¿y identyfikowaæ za pomoc¹ typów komunikatów systemu V. Klucze dla kolejek maj¹ byæ generowane na podstawie œcie¿ki $HOME. Ma³e liczby do wygenerowania kluczy oraz rodzaje komunikatów maj¹ byæ zdefiniowane we wspólnym pliku nag³ówkowym. Dla uproszczenia mo¿na za³o¿yæ, ¿e d³ugoœæ komunikatu jest ograniczona pewn¹ sta³¹ (jej definicja powinna znaleŸæ siê w pliku nag³ówkowym).
Klient i serwer nale¿y napisaæ w postaci osobnych programów (nie korzystamy z funkcji fork). Serwer musi byæ w stanie pracowaæ z wieloma klientami naraz. Przed zakoñczeniem pracy ka¿dy proces powinien usun¹æ kolejkê któr¹ utworzy³ (patrz IPC_RMID oraz funkcja atexit). Dla uproszczenia mo¿na przyj¹æ, ¿e serwer przechowuje informacje o klientach w statycznej tablicy (rozmiar tablicy ogranicza liczbê klientów, którzy mog¹ siê zg³osiæ do serwera).
Serwer mo¿e wys³aæ do klientów komunikaty:
* inicjuj¹cy pracê klienta
* wysy³aj¹cy odpowiedzi do klientów (kolejki klientów)
* informuj¹cy klientów o zakoñczeniu pracy serwera - po wys³aniu takiego sygna³u i odebraniu wiadomoœci STOP od wszystkich klientów serwer usuwa swoj¹ kolejkê i koñczy pracê. (kolejki klientów)
Nale¿y obs³u¿yæ przerwanie dzia³ania serwera lub klienta za pomoc¹ CTRL+C. Po stronie klienta obs³uga tego sygna³u jest równowa¿na z wys³aniem komunikatu STOP.

## Zadanie 2. Prosty chat - POSIX (50%)
Zrealizuj zadanie analogiczne do Zadania 1, wykorzystuj¹c kolejki komunikatów POSIX. Kolejka klienta powinna mieæ losow¹ nazwê zgodn¹ z wymaganiami stawianymi przez POSIX. Na typ komunikatu mo¿na zarezerwowaæ pierwszy bajt jego treœci. Obs³uga zamykania kolejek analogiczna jak w zadaniu 1, z tym, ¿e aby mo¿na by³o usun¹æ kolejkê, wszystkie procesy powinny najpierw j¹ zamkn¹æ. Przed zakoñczeniem pracy klient wysy³a do serwera komunikat informuj¹cy, ¿e serwer powinien zamkn¹æ po swojej stronie kolejkê klienta. Nastêpnie klient zamyka i usuwa swoj¹ kolejkê. Serwer przed zakoñczeniem pracy zamyka wszystkie otwarte kolejki, informuje klientów, aby usunêli swoje kolejki oraz zamknêli kolejkê serwera i usuwa kolejkê, któr¹ utworzy³.