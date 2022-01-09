# Projekt z Sieci Komputerowych 2
---
## Wisielec przez sieć

Gracz łączy się do serwera i wysyła swój nick (jeśli nick jest już zajęty,
serwer prosi o podanie innego nicku).

Gracz trafia do lobby, gdzie może wejść do istniejącego pokoju lub założyć nowy. Gracz może w każdej chwili wrócić z pokoju do lobby.

Gracz po dołączeniu widzi listę graczy w pokoju i czeka na rozpoczęcie gry. Jeśli gra już trwa, gracz po prostu dołącza do gry.

Grę może rozpocząć gracz, który najdłużej czeka w pokoju, pod warunkiem że w pokoju jest przynajmniej dwóch graczy. Rozpoczynający grę ustawia ilość rund.

Gracz który rozpoczął grę wysyła do serwera hasło wisielca. Nie ma rozróżnienia między małymi i wielkimi literami. Jeśli gracz zostanie rozłączony przed wysłaniem hasła, hasło wysyła kolejny gracz.

Po przekazaniu hasła do serwera, serwer wysyła do zgadujących puste miejsca na litery na hasła.

Zgadujący mogą wpisywać pojedyncze litery z odgadywanego hasła. Każda wpisana litera jest widziana przez wszystkich graczy. Jeśli litera jest w haśle, zgadujący gracz dostaje liczbę punktów równą liczbie wystąpień litery w haśle, a litera jest wpisywana we właściwe miejsce w haśle u wszystkich graczy. Jeśli litera nie jest w haśle, wisielec zgadującego gracza powiększa się.

Jeżeli kilku graczy wpisało tę samą literę jednocześnie, punkty dostaje gracz, którego wiadomość serwer otrzymał jako pierwszy.

Po odgadnięciu całego hasła lub minięciu określonego czasu następny gracz wysyła hasło do serwera.

Gra toczy się do momentu, gdy miną wszystkie rundy lub gdy zostanie mniej niż dwóch graczy. Po zakończeniu gry gracze widzą ranking z punktami, włączając graczy, który rozłączyli się z gry.

Gracze cały czas widzą wisielce swoje i innych graczy, a także bieżącą punktację.
---
Informatyka, semestr V
Dawid Plaskowski
Wojciech Piszczek
---
