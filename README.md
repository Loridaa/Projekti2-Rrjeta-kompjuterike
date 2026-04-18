# Projekti2-Rrjeta-kompjuterike
UDP Client-Server Application in C

Ky projekt eshte realizuar ne kuader te lendes Rrjeta Kompjuterike dhe implementon nje sistem komunikimi Client-Server duke perdorur UDP dhe gjuhen C.

Serveri menaxhon kerkesat e klienteve, ruan mesazhet dhe ofron funksionalitete per menaxhim file-sh. Projekti perfshin edhe nje HTTP server te thjeshte per monitorim statistikor.

Teknologjite
- Gjuha programuese: C
- Protokolli: UDP
- Socket Programming
- HTTP Server basic per statistika

Struktura e file-ve
- server: logjika kryesore e UDP serverit, autentikim roli, menaxhim klientesh, delegim komandash.
- filemanage: komandat per file (list, read, search, info, upload, download, delete).
- httpserver: endpoint HTTP ne portin 8081 per statistika (GET /stats).
- client: logjika e klientit UDP, komunikim me serverin dhe komandat.
- menu_ui.c: renderimi i menus statike ne terminal.
- menu_ui.h: deklarimet e funksioneve te menus UI.
- Makefile: build dhe clean per server/client (i perdorur gjat fazes se zhvillimit).
- README: dokumentimi i projektit.

Projekti eshte kompiluar dhe testuar ne MSYS2 gjate fazes se zhvillimit.

1. ./server
2. ./client
3. HTTP test
- GET http://127.0.0.1:8081/stats

Funksionalitete kryesore
- Multi-client UDP server me role admin/user.
- Menaxhim file-sh ne folderin server_files.
- Kontroll i komandave admin per upload/download/delete.
- Monitorim me JSON stats nga HTTP endpoint.
