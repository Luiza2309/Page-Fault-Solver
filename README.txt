Verific daca programul primeste SEGFAULT. Daca primeste alt semnal il dau mai departe.
Daca semnalul n-are informatii despre el, dau SEGFAULT.
Parcurg segmentele. Iau adresa la care a fost raportat SEGFAULTUL.
Aloc data pentru a stii ce segmente au fost mapate.
Verific daca adresa SEGFAULTULUI se incadreaza intre adresele de inceput si sfarsit ale segmentului.
Calculez numarul paginii si verific daca a mai fost mapata cu ajutorul lui data.
Daca n-am mai fost pe aici, o mapez.
Calculez lungimea portiunii pe care o citesc din file descriptor si pe care o voi scrie la adresa mapata.
Daca ma aflu pe ultima pagina, aceasta portiune este de la inceputul paginii pana la finalul filesizeului.
Mut cursorul unde vreau sa citesc in file descriptor.
Citesc din fisier.
Dau SEGFAULT daca nu am gasit adresa semnalului printre segmente.