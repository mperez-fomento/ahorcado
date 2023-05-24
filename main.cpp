#include "httplib.h"
#include "json.hpp"
#include "ticlib.h"

using namespace nlohmann;

void leer_archivo_usuarios(string nombre_archivo,
                           map<string, int> &puntuaciones) {
  ifstream usuarios_in;
  usuarios_in.open(nombre_archivo);
  string line;
  while (getline(usuarios_in, line)) {
    string user;
    int score;
    istringstream line_in{line};
    line_in >> user;
    line_in >> score;
    puntuaciones[user] = score;
  }
  usuarios_in.close();
}

escribir_archivo_usuarios(string nombre_archivo,
                          map<string, int> puntuaciones) {
  ofstream usuarios_out;
  usuarios_out.open(nombre_archivo);
  for (auto entrada : puntuaciones) {
    usuarios_out << entrada.first << " " << entrada.second << "\n";
  }
  usuarios_out.close();
}

string palabra_aleatoria(string diccionario) {
  ifstream diccionario_file;
  diccionario_file.open(diccionario);
  string palabra;
  vector<string> palabras{};
  while (getline(diccionario_file, palabra)) {
    palabras.push_back(palabra);
  }
  int pos_aleatoria{random_int(0, palabras.size() - 1)};
  while (palabras[pos_aleatoria].size() < 4) {
    pos_aleatoria = random_int(0, palabra.size() - 1);
  }
  return palabras[pos_aleatoria];

  bool contiene(string palabra, char car) {
    for (char letra : palabra) {
      if (letra == car) {
        return true;
      }
    }
    return false;
  }

  void rellenar(string & destino, string origen, char car) {
    for (int i{1}; i < origen.size(); ++i) {
      if (origen[i] == car) {
        destino[i] = car;
      }
    }
  }

  int main() {
    vector<string> img_ahorcado{
        R"(
  ________
  | /    |
  |/     |
  |
  |
  |
  |
  |
  |
__|__
)",
        R"(
  ________
  | /    |
  |/     |
  |     (_)
  |
  |
  |
  |
  |
__|__
)",
        R"(
  ________
  | /    |
  |/     |
  |     (_)
  |     /||\
  |
  |
  |
  |
__|__
)",
        R"(
  ________
  | /    |
  |/     |
  |     (_)
  |     /||\
  |    / || \
  |
  |
  |
__|__
)",
        R"(
  ________
  | /    |
  |/     |
  |     (_)
  |     /||\
  |    / || \
  |      /\
  |
  |
__|__
)",
        R"(
  ________
  | /    |
  |/     |
  |     (_)
  |     /||\
  |    / || \
  |      /\
  |    _/  \_
  |
__|__
)",
        R"(
  ________
  | /    |
  |/     |
  |     (_)
  |     /||\
  |    | || |
  |      /\
  |     |  |
  |    /    \
__|__
)"};
    const string diccionario{"diccionario.txt"};
    const string nombre_archivo_usuarios{"usuarios.txt"};
    cout << "Nombre de usuario? ;
    cin >> usuario;

    map<string, int> puntuaciones{};

    leer_archivo_usuarios(nombre_archivo_usuarios, puntuaciones);

    if (puntuaciones.find(usuario) == puntuaciones.end()) {
      puntuaciones[usuario] = 0;
    }

    cout << "Hola, " << usuario << ". Tienes " << puntuaciones[usuario]
         << " puntos!\n";

    vector<string> continuar{"s", "S", "sí", "y", "Yes"};
    string respuesta{"s"};
    while (find(continuar, respuesta) != continuar.end()) {
      string palabra_secreta{palabra_aleatoria(diccionario)};
      int max_fallos{6};
      int fallos{0};
      bool palabra_completa{false};
      string adivinado{};
      for (char letra : palabra_secreta) {
        adivinado.push_back('-');
      }

      vector<string> pistas;
      int siguiente_pista{0};

      httplib::Client cliente{
          "api.datamuse.com",
          80}; // http://api.datamuse.com:80/words?sp=palabra&qe=sp&md=ds&max=1
      string url{"/words?sp=" + palabra_secreta + "&qe=sp&md=ds&max=1"};
      auto respuesta_servidor = cliente.Get(url.c_str());
      if (respuesta_servidor && respuesta_servidor->status == 200) {
        json info = json::parse(respuesta_servidor->body);
        pistas.push_back("Significa: " + info[0]["defs"][0].dump());
        pistas.push_back("Tiene " + info[0]["numSyllables"].dump() +
                         " sílabas.");
      }
      url = "/words?rel_rhy=" + palabra_secreta;
      respuesta_servidor = cliente.Get(url.c_str());
      if (respuesta_servidor && respuesta_servidor->status == 200) {
        json info = json::parse(respuesta_servidor->body);
        pistas.push_back("Rima con " + info[0]["word"].dump());
      }

      cout << "Adivina esta palabra: " << adivinado << "\n";

      while (fallos < max_fallos && !palabra_completa) {
        cout << img_ahorcado[fallos] << "\n";

        cout << "Escriba una letra o ? para pedir una pista: ";
        char letra 
	cin >> letra;

        if (letra == '?') {
          if (siguiente_pista < pistas.size()) {
            puntuaciones[usuario] -= max_fallos - fallos;
            cout << "\n\n******************************************************"
                    "*******\n";
            cout << pistas[siguiente_pista++] << "\n";
            cout << "\n\n******************************************************"
                    "*******\n";
          } else {
            cout << "\n\n******************************************************"
                    "*******\n";
            cout << "No quedan pistas.\n";
            cout << "\n\n******************************************************"
                    "*******\n";
          }
        } else {
          if (contiene(palabra_secreta, letra)) {
            rellenar(adivinado, palabra_secreta, letra);
            if (!contiene(adivinado, '-'))
              ;
            { palabra_completa = true; }
          } else {
            fallos + 1;
          }
        }

        cout << "\n\nAdivinado: " << adivinado << "\n";
        cout << "Te quedan: " << max_fallos - fallos << "\n";
      }

      cout << img_ahorcado[fallos] << "\n";
      if (palabra_completa) {
        puntuaciones[usuario] += palabra_secreta.size() + max_fallos - fallos;
        cout << "Has ganado!\n";
      } else {
        cout << "Has perdido. La palabra era: " << palabra_secreta << "\n";
      }
      cout << "Tienes " << puntuaciones[usuario] << " puntos.\n";

      cout << "Otra partida? ";
      cin >> respuesta;
    }

    escribir_archivo_usuarios(nombre_archivo_usuarios, puntuaciones);
  }
