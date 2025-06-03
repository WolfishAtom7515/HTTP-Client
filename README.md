# Bobei Bogdan Dumitru - 324CC

### Client web. Comunicaţie cu REST API.

## Descriere

Acest proiect reprezintă un client HTTP scris în C++ care comunică cu un server REST API. Clientul permite efectuarea de operații precum autentificare, gestionare utilizatori, trimitere și primire de date în format JSON, folosind protocoalele HTTP/HTTPS. 

Fișierele `helper.cpp`, `helper.hpp`, `requests.cpp` si `requests.hpp` a fost realizat pe baza cadrului oferit la laboratorul 9.

## Structura proiectului

- `client.cpp` – Punctul de intrare al aplicației, gestionează interacțiunea cu utilizatorul și apelează funcțiile principale.
- `requests.cpp` / `requests.hpp` – Implementarea și definirea funcțiilor pentru trimiterea cererilor HTTP (GET, POST, DELETE etc.).
- `helper.cpp` / `helper.hpp` – Funcții auxiliare pentru procesarea datelor, parsarea răspunsurilor și gestionarea erorilor.
- `json.hpp` – Bibliotecă pentru manipularea datelor JSON.
- `Makefile` – Script pentru compilarea proiectului.

## Resurse

- Compilator C++ (recomandat: g++ >= 9)
- [nlohmann/json](https://github.com/nlohmann/json) (inclus local ca `json.hpp`)

## Compilare

In directorul curent, ruleaza comanda:

```sh
make
```

Se va genera un executabil numit `client`.

## Utilizare

Execută clientul din terminal:

```sh
./client
```

Urmează instrucțiunile din linia de comandă pentru a interacționa cu serverul (autentificare, creare utilizator, trimitere cereri etc.).

## Utilizarea parserului JSON

Pentru manipularea datelor în format JSON, proiectul folosește biblioteca [nlohmann/json](https://github.com/nlohmann/json), inclusă local ca `json.hpp`. Aceasta permite conversia rapidă și ușoară între structuri C++ și obiecte JSON.

În `client.cpp`, parserul JSON este folosit pentru:
- **Parsarea răspunsurilor de la server:** Răspunsurile HTTP primite de la server sunt extrase și convertite în obiecte JSON, permițând accesul facil la câmpuri precum `id`, `title`, `owner`, `movies` etc.
- **Construirea cererilor către server:** Pentru operații precum autentificare, adăugare de utilizatori sau filme, datele sunt organizate în obiecte JSON și serializate înainte de a fi trimise către server.
- **Manipularea listelor și obiectelor:** Listele de utilizatori, filme sau colecții sunt parcurse și procesate direct ca vectori de obiecte JSON.

Exemplu de utilizare atat de construire cat si de parsare:
```cpp
// Parsarea unui răspuns JSON
std::string ext_response = basic_extract_json_response((char *)response.c_str());
nlohmann::json body_response = nlohmann::json::parse(ext_response);
std::string title = body_response["title"];

// Construirea unui obiect JSON pentru trimitere
nlohmann::ordered_json credentials;
credentials["username"] = username;
credentials["password"] = password;
std::string credentials_dump = credentials.dump();
```
## Exemple de comenzi suportate

- `login_admin` – Autentificare ca administrator
- `add_user` – Adăugare utilizator nou (admin)
- `get_users` – Listare utilizatori (admin)
- `delete_user` – Ștergere utilizator (admin)
- `logout_admin` – Deconectare administrator (admin)
- `login` – Autentificare ca utilizator
- `get_access` – Obținere token de acces (JWT)
- `get_movies` – Listare filme
- `get_movie` – Afișare detalii film după ID
- `add_movie` – Adăugare film nou
- `delete_movie` – Ștergere film după ID
- `update_movie` – Actualizare detalii film
- `get_collections` – Listare colecții
- `get_collection` – Afișare detalii colecție după ID
- `add_collection` – Adăugare colecție nouă
- `delete_collection` – Ștergere colecție după ID
- `add_movie_to_collection` – Adăugare film într-o colecție
- `delete_movie_from_collection` – Ștergere film dintr-o colecție
- `logout` – Deconectare utilizator
- `exit` – Ieșire din aplicație

