# Projekti2-Rrjeta-kompjuterike
UDP Client-Server Application in C

Ky projekt është realizuar në kuadër të lëndës Rrjeta Kompjuterike dhe implementon një sisitem komunikimi Client-Server duke përdorur protokollin UDP dhe gjuhen programuese C.

Serveri menaxhon kërkesat nga klientët, ruan mesazhet dhe ofron funksionalitete për menaxhimin e file-ve. Gjithashtu përfshin nje HTTP server të thjeshtë për monitorimin e aktivitett.

Teknologjitë
-Gjuha programuese: C
-Protokolli: UDP
-Socket Programming
-HTTP Server (basic)


Funksionalitetet:
-Serveri 
Pranon lidhje nga shumë klientë
Menaxhon kërkesat dhe mesazhet
Ruajn mesazhet për monitorim
Mbyll lidhjet pas një periudhe joaktive
Ofron qasje në file për klientët admin
HTTP server në prot tjetër për statistika

-Klienti
Lidhet me server përmes IP dhe portit
Dërgon dhe pranon mesazhe
Ekzekuton komanda në server
