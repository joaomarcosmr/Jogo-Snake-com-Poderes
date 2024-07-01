#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
using namespace std;
using namespace chrono;

const int MAP_WIDTH = 30;
const int MAP_HEIGHT = 20;
const int COMPRIMENTO_INICIAL_COBRA = 5;
const int NUM_LEVELS = 3;
const int COMPRIMENTO_MAXIMO_COBRA = 100;

struct Ponto {
    int x, y;
};

struct Pontuacao {
    string nome;
    int pontuacao;
    string tempo;
    string data;
    string modo; // Novo
};

enum Direction { UP, DOWN, LEFT, RIGHT };

struct IngameDisplay {
    int pontuacao = 0;
    int numMacas = 0;
    int tempo = 0;
    int movimentos = 0;
};

vector<Pontuacao> lerPontuacoes(const string& filename) {
    vector<Pontuacao> pontuacoes;
    ifstream file(filename);
    if (file.is_open()) {
        Pontuacao p;
        while (file >> p.nome >> p.pontuacao >> p.tempo >> p.data >> p.modo) {
            if (file.fail()) {
                cout << "Erro ao ler o arquivo de pontua  es." << endl;
                break;
            }
            pontuacoes.push_back(p);
        }
        file.close();
    } else {
        cout << "Erro ao abrir o arquivo para leitura." << endl;
    }
    return pontuacoes;
}

void salvarPontuacoes(const vector<Pontuacao>& pontuacoes, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        for (const Pontuacao& p : pontuacoes) {
            file << p.nome << " " << p.pontuacao << " " << p.tempo << " " << p.data << " " << p.modo << endl;
            if (file.fail()) {
                cout << "Erro ao salvar pontua  es no arquivo." << endl;
                break;
            }
        }
        file.close();
    } else {
        cout << "Erro ao abrir o arquivo para escrita." << endl;
    }
}

void inserirPontuacaoRec(vector<Pontuacao>& pontuacoes, const Pontuacao& novaPontuacao, int indice) {
    if (indice >= pontuacoes.size() || pontuacoes[indice].pontuacao < novaPontuacao.pontuacao) {
        pontuacoes.insert(pontuacoes.begin() + indice, novaPontuacao);
        return;
    }
    inserirPontuacaoRec(pontuacoes, novaPontuacao, indice + 1);
}

void adicionarPontuacao(vector<Pontuacao>& pontuacoes, const string& nome, int pontuacao, const string& tempo, const string& modo) {
    auto now = system_clock::now();
    time_t now_c = system_clock::to_time_t(now);
    char buf[100];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now_c));
    string data(buf);

    Pontuacao novaPontuacao = {nome, pontuacao, tempo, data, modo};
    inserirPontuacaoRec(pontuacoes, novaPontuacao, 0);
}

void exibirRanking(const vector<Pontuacao>& pontuacoes) {
    if (pontuacoes.empty()) {
        cout << "O ranking est  vazio." << endl;
    } else {
        cout << "=== RANKING ===" << endl;
        for (size_t i = 0; i < pontuacoes.size(); ++i) {
            cout << i + 1 << ". " << pontuacoes[i].nome << " - " << pontuacoes[i].pontuacao << " pontos - Tempo: " << pontuacoes[i].tempo << " - Data: " << pontuacoes[i].data << " - Modo: " << pontuacoes[i].modo << endl;
        }
    }
}


void criaMapa(int m[][MAP_WIDTH], Ponto maca, Ponto portal, int level, bool portalActive) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (i == 0 || i == MAP_HEIGHT - 1 || j == 0 || j == MAP_WIDTH - 1) {
                m[i][j] = 1; // paredes
            } else {
                m[i][j] = 0; // espaço vazio
            }
        }
    }

    // Adiciona obstáculos ou layouts diferentes baseados no nível
    if (level == 2) {
        for (int i = 5; i < 18; i++) {
            m[i][10] = 1;
            m[i][20] = 1;
        }
    } else if (level == 3) {
        for (int i = 5; i < 18; i++) {
            m[5][i] = 1;
            m[17][i] = 1;
        }
    }

    if (maca.x >= 0 && maca.x < MAP_HEIGHT && maca.y >= 0 && maca.y < MAP_WIDTH) {
        m[maca.x][maca.y] = 3; // maçã
    }

    if (portalActive && portal.x >= 0 && portal.x < MAP_HEIGHT && portal.y >= 0 && portal.y < MAP_WIDTH) {
        m[portal.x][portal.y] = 5; // portal
    }
}

void criaMapa(int m[][MAP_WIDTH], Ponto snake[], int comprimentoCobra, Ponto maca, Ponto portal, int level, bool portalActive) {
    criaMapa(m, maca, portal, level, portalActive);
    for (int i = 0; i < comprimentoCobra; i++) {
        if (i == 0) {
            m[snake[i].x][snake[i].y] = 4; // cabeça da cobra
        } else {
            m[snake[i].x][snake[i].y] = 2; // corpo da cobra
        }
    }
}

Ponto geraMaca(Ponto snake[], int comprimentoCobra) {
    Ponto maca;
    bool valid = false;
    int tentativas = 0; // Adicionamos um contador de tentativas
    while (!valid && tentativas < 100) { // Limite de tentativas para evitar loops infinitos
        valid = true;
        maca.x = rand() % (MAP_HEIGHT - 2) + 1;
        maca.y = rand() % (MAP_WIDTH - 2) + 1;
        for (int i = 0; i < comprimentoCobra; ++i) {
            if (snake[i].x == maca.x && snake[i].y == maca.y) {
                valid = false;
                break;
            }
        }
        tentativas++;
    }
    if (!valid) { // Se não foi possível encontrar uma posição válida, retorna uma posição fora do mapa
        maca.x = -1;
        maca.y = -1;
    }
    return maca;
}

//função para o modo automático (cobra controlada pelo computador)
void jogarModoComputador(vector<Pontuacao> &pontuacoes, const string& nomeArquivo) {
    string nomeJogador = "Computador";
    system("cls");
    Direction dir = RIGHT;
    IngameDisplay ingameDisplay;
    auto ultimoTempo = chrono::steady_clock::now();
    auto inicioContadorInGame = chrono::steady_clock::now();

    int currentLevel = 1;
    int numMacasPegadas = 0;
    const auto marcadorTempoInGame = chrono::milliseconds(1000); // Marcador de tempo em jogo
    const auto velocidadeInicial = chrono::milliseconds(300); // Velocidade inicial da cobra
    auto velocidadeCobra = velocidadeInicial; // Velocidade da cobra
    bool portalActive = false;
    Ponto portal = {MAP_HEIGHT / 2, MAP_WIDTH / 2}; // Posição do portal

    Ponto snake[100]; // Comprimento máximo
    int comprimentoCobra = COMPRIMENTO_INICIAL_COBRA;
    for (int i = 0; i < comprimentoCobra; i++) {
        snake[i].x = MAP_HEIGHT / 2;
        snake[i].y = MAP_WIDTH / 2 - i;
    }
    Ponto maca = geraMaca(snake, comprimentoCobra);

    while (true) {
        auto tempoAtualContadorInGame = chrono::steady_clock::now();
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
        int m[MAP_HEIGHT][MAP_WIDTH];
        criaMapa(m, snake, comprimentoCobra, maca, portal, currentLevel, portalActive);

        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                switch (m[i][j]) {
                    case 0: cout << " "; break; // espaço vazio
                    case 1: cout << char(219); break; // paredes
                    case 2: cout << char(184); break; // corpo da cobra
                    case 4: cout << char(199); break; // cabeça da cobra
                    case 3: cout << char(208); break; // maçã
                    case 5: cout << char(206); break; // portal
                }
            }
            cout << "\n";
        }

        // Lógica de movimento do computador
        if (snake[0].x < maca.x) {
            dir = DOWN;
        } else if (snake[0].x > maca.x) {
            dir = UP;
        } else if (snake[0].y < maca.y) {
            dir = RIGHT;
        } else if (snake[0].y > maca.y) {
            dir = LEFT;
        }

        auto tempoAtual = chrono::steady_clock::now();
        if (tempoAtual - ultimoTempo >= velocidadeCobra) {
            ultimoTempo = tempoAtual;

            Ponto proxPos = snake[0]; // Próxima posição da cabeça da cobra
            switch (dir) {
                case UP: proxPos.x--; break;
                case DOWN: proxPos.x++; break;
                case LEFT: proxPos.y--; break;
                case RIGHT: proxPos.y++; break;
            }

            // Colisão com a parede
            if (proxPos.x <= 0 || proxPos.x >= MAP_HEIGHT - 1 || proxPos.y <= 0 || proxPos.y >= MAP_WIDTH - 1) {
                adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo) + "s", "Computador");
                salvarPontuacoes(pontuacoes, nomeArquivo);
                cout << "Game Over!" << endl;
                break;
            }

            // Colisão com o corpo da cobra
            for (int i = 1; i < comprimentoCobra; i++) {
                if (snake[i].x == proxPos.x && snake[i].y == proxPos.y) {
                    adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo) + "s", "Computador");
                    salvarPontuacoes(pontuacoes, nomeArquivo);
                    cout << "Game Over!" << endl;
                    return;
                }
            }

            // Se a cobra comer uma maçã
            if (proxPos.x == maca.x && proxPos.y == maca.y) {
                comprimentoCobra++;
                ingameDisplay.pontuacao += 10;
                ingameDisplay.numMacas++;
                numMacasPegadas++;
                if (numMacasPegadas % 2 == 0 && numMacasPegadas <= 10) {
                    velocidadeCobra -= chrono::milliseconds(25); // Acelera a cobra
                }
                maca = geraMaca(snake, comprimentoCobra);

                // Verifica se 100 maçãs foram coletadas para ativar o portal
                if (ingameDisplay.numMacas >= 100) {
                    portalActive = true;
                }
            }

            // Se a cobra chegar ao portal e o portal estiver ativo
            if (portalActive && proxPos.x == portal.x && proxPos.y == portal.y) {
                portalActive = false;
                currentLevel++;
                if (currentLevel <= NUM_LEVELS) {
                    comprimentoCobra = COMPRIMENTO_INICIAL_COBRA;
                    for (int i = 0; i < comprimentoCobra; i++) {
                        snake[i].x = MAP_HEIGHT / 2;
                        snake[i].y = MAP_WIDTH / 2 - i;
                    }
                    numMacasPegadas = 0;
                    velocidadeCobra = velocidadeInicial; // Reseta a velocidade da cobra
                    ingameDisplay.numMacas = 0;
                    ingameDisplay.pontuacao = ingameDisplay.pontuacao+1000;
                    ingameDisplay.movimentos = 0; // Reseta o contador de movimentos
                    maca = geraMaca(snake, comprimentoCobra);
                    system("cls");
                    continue; // Carrega o novo nível
                } else {
                    adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo) + "s", "Computador");
                    salvarPontuacoes(pontuacoes, nomeArquivo);
                    cout << "Você completou todos os níveis!" << endl;
                    break;
                }
            }

            // Move a cobra
            for (int i = comprimentoCobra - 1; i > 0; i--) {
                snake[i] = snake[i - 1];
            }
            snake[0] = proxPos;
            ingameDisplay.movimentos++; // Incrementa o contador de movimentos
        }

        ingameDisplay.tempo = chrono::duration_cast<chrono::seconds>(tempoAtualContadorInGame - inicioContadorInGame).count();
        cout << "Tempo: " << ingameDisplay.tempo << "s" << endl;
        cout << "Pontuação: " << ingameDisplay.pontuacao << endl;
        cout << "Maçãs: " << ingameDisplay.numMacas << endl;
        cout << "Movimentos: " << ingameDisplay.movimentos << endl;
        Sleep(50);
    }
}


// Função para o modo de redução
void jogarModoReducao(vector<Pontuacao>& pontuacoes, const string& nomeArquivo) {
    const int TEMPO_LIMITE = 10; // Tempo limite em segundos
    srand(time(NULL));
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);

    int m[MAP_HEIGHT][MAP_WIDTH];

    Ponto snake[100];
    int comprimentoCobra = COMPRIMENTO_MAXIMO_COBRA;
    for (int i = 0; i < comprimentoCobra; i++) {
        snake[i].x = MAP_HEIGHT / 2;
        snake[i].y = MAP_WIDTH / 2 - i;
    }

    struct Maca {
        Ponto pos;
        bool aumenta; // True se a maçã aumenta a cobra, false se diminui
    };

    auto geraMaca = [&snake, &comprimentoCobra, &m]() {
        Maca maca;
        bool invalidPosition;
        do {
            invalidPosition = false;
            maca.pos.x = rand() % MAP_HEIGHT;
            maca.pos.y = rand() % MAP_WIDTH;

            // Verifica se a maçã está em cima da cobra
            for (int i = 0; i < comprimentoCobra; i++) {
                if (snake[i].x == maca.pos.x && snake[i].y == maca.pos.y) {
                    invalidPosition = true;
                    break;
                }
            }

            // Verifica se a maçã está nas paredes
            if (m[maca.pos.x][maca.pos.y] == 1) {
                invalidPosition = true;
            }
        } while (invalidPosition);

        maca.aumenta = rand() % 2 == 0; // 50% de chance de aumentar ou diminuir
        return maca;
    };

    Maca maca = geraMaca();

    string nomeJogador;
    cout << "Digite seu nome: " << endl;
    cin >> nomeJogador;
    system("cls");

    //remove espaços do nome
    replace(nomeJogador.begin(), nomeJogador.end(), ' ', '_');
    Direction dir = RIGHT;
    char tecla;
    IngameDisplay ingameDisplay;
    auto ultimoTempo = chrono::steady_clock::now();
    auto inicioContadorInGame = chrono::steady_clock::now();
    auto inicioJogo = chrono::steady_clock::now();


    int numMacasPegadas = 0;
    const auto marcadorTempoInGame = chrono::milliseconds(1000);
    const auto velocidadeInicial = chrono::milliseconds(300);
    auto velocidadeCobra = velocidadeInicial;

    while (true) {
        auto tempoAtualContadorInGame = chrono::steady_clock::now();
        auto tempoPassado = chrono::duration_cast<chrono::seconds>(tempoAtualContadorInGame - inicioJogo).count();
        if (tempoPassado >= TEMPO_LIMITE) {
            cout << "Tempo esgotado! Fim de jogo!" << endl;
            break;
        }
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
        criaMapa(m, snake, comprimentoCobra, maca.pos, { -1, -1 }, 1, false);

        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                switch (m[i][j]) {
                    case 0: cout << " "; break; // espaço vazio
                    case 1: cout << char(219); break; // paredes
                    case 2: cout << char(184); break; // corpo da cobra
                    case 4: cout << char(199); break; // cabeça da cobra
                    case 3: cout << (maca.aumenta ? char(208) : char(176)); break; // maçã (diferentes caracteres para aumentar ou diminuir)
                    case 5: cout << char(206); break; // portal
                }
            }
            cout << "\n";
        }

        cout << "Tempo para conseguir: " << tempoPassado << " de " << TEMPO_LIMITE << "segundos | " << "Pontuacao: " << ingameDisplay.pontuacao << " | " << "Movimentos: " << ingameDisplay.movimentos << " | " << "Macas consumidas: " << numMacasPegadas << endl;

        if (_kbhit()) {
            tecla = getch();
            switch (tecla) {
                case 72: case 'w':
                    if (dir != DOWN) dir = UP;
                    break;
                case 80: case 's':
                    if (dir != UP) dir = DOWN;
                    break;
                case 75: case 'a':
                    if (dir != RIGHT) dir = LEFT;
                    break;
                case 77: case 'd':
                    if (dir != LEFT) dir = RIGHT;
                    break;
            }
        }

        auto tempoAtual = chrono::steady_clock::now();
        if (tempoAtual - ultimoTempo >= velocidadeCobra) {
            ultimoTempo = tempoAtual;

            Ponto proxPos = snake[0]; // Próxima posição da cabeça da cobra
            switch (dir) {
                case UP: proxPos.x--; break;
                case DOWN: proxPos.x++; break;
                case LEFT: proxPos.y--; break;
                case RIGHT: proxPos.y++; break;
            }

            if (m[proxPos.x][proxPos.y] == 1 || m[proxPos.x][proxPos.y] == 2) {
                cout << "Game Over! Voce bateu na parede ou comeu a si mesmo!" << endl;
                break;
            }

            for (int i = comprimentoCobra - 1; i > 0; i--) {
                snake[i] = snake[i - 1];
            }
            snake[0] = proxPos;

            if (snake[0].x == maca.pos.x && snake[0].y == maca.pos.y) {
                if (maca.aumenta) {
                    if (comprimentoCobra < 100) {
                        comprimentoCobra++;
                    }
                    ingameDisplay.pontuacao -= 10; // Diminuir a pontuação se a maçã aumenta a cobra
                } else {
                    if (comprimentoCobra > 1) {
                        comprimentoCobra--;
                    }
                    ingameDisplay.pontuacao += 10; // Aumentar a pontuação se a maçã diminui a cobra
                }
                maca = geraMaca();
                numMacasPegadas++;
            }

            if (comprimentoCobra == 0) {
                cout << "Parabens! Voce conseguiu reduzir a cobra ao tamanho minimo!" << endl;
                break;
            }

            ingameDisplay.movimentos++;
            ultimoTempo = tempoAtualContadorInGame;
        }

        if (chrono::duration_cast<chrono::milliseconds>(tempoAtualContadorInGame - inicioContadorInGame) >= marcadorTempoInGame) {
            ingameDisplay.tempo++;
            inicioContadorInGame = tempoAtualContadorInGame;
        }

    }

    adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo), "Reducao");
    salvarPontuacoes(pontuacoes, nomeArquivo);
}

// função para o modo contra o tempo
void jogarModoContraTempo(vector<Pontuacao>& pontuacoes, const string& nomeArquivo) {
    const int TEMPO_LIMITE = 10; // Tempo limite em segundos
    srand(time(NULL));
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);

    int m[MAP_HEIGHT][MAP_WIDTH];

    Ponto snake[100];
    int comprimentoCobra = COMPRIMENTO_INICIAL_COBRA;
    for (int i = 0; i < comprimentoCobra; i++) {
        snake[i].x = MAP_HEIGHT / 2;
        snake[i].y = MAP_WIDTH / 2 - i;
    }
    Ponto maca = geraMaca(snake, comprimentoCobra);

    string nomeJogador;
    cout << "Digite seu nome: " << endl;
    cin >> nomeJogador;
    replace(nomeJogador.begin(), nomeJogador.end(), ' ', '_');
    system("cls");
    Direction dir = RIGHT;
    char tecla;
    IngameDisplay ingameDisplay;
    auto ultimoTempo = chrono::steady_clock::now();
    auto inicioContadorInGame = chrono::steady_clock::now();
    auto inicioJogo = chrono::steady_clock::now();

    int numMacasPegadas = 0;
    const auto marcadorTempoInGame = chrono::milliseconds(1000);
    auto velocidadeInicial = chrono::milliseconds(100);
    auto velocidadeCobra = velocidadeInicial;

    while (true) {
        auto tempoAtualContadorInGame = chrono::steady_clock::now();
        auto tempoPassado = chrono::duration_cast<chrono::seconds>(tempoAtualContadorInGame - inicioJogo).count();
        if (tempoPassado >= TEMPO_LIMITE) {
            cout << "Tempo esgotado! Fim de jogo!" << endl;
            break;
        }

        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
        criaMapa(m, snake, comprimentoCobra, maca, {-1, -1}, 1, false);

        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                switch (m[i][j]) {
                    case 0: cout << " "; break; // Espaço vazio
                    case 1: cout << char(219); break; // Paredes
                    case 2: cout << char(184); break; // corpo da cobra
                    case 4: cout << char(199); break; // cabeça da cobra
                    case 3: cout << char(208); break; // Maçã
                    case 5: cout << char(206); break; // Portal
                }
            }
            cout << "\n";
        }

        cout << "Pontuacao: " << ingameDisplay.pontuacao << " | " << "Movimentos: " << ingameDisplay.movimentos << " | " << "Macas consumidas: " << numMacasPegadas << endl;

        if (_kbhit()) {
            tecla = _getch();
            switch (tecla) {
                case 72: case 'w':
                    if (dir != DOWN) dir = UP;
                    break;
                case 80: case 's':
                    if (dir != UP) dir = DOWN;
                    break;
                case 75: case 'a':
                    if (dir != RIGHT) dir = LEFT;
                    break;
                case 77: case 'd':
                    if (dir != LEFT) dir = RIGHT;
                    break;
            }
        }

        auto tempoAtual = chrono::steady_clock::now();
        if (tempoAtual - ultimoTempo >= velocidadeCobra) {
            ultimoTempo = tempoAtual;

            Ponto proxPos = snake[0]; // Próxima posição da cabeça da cobra
            switch (dir) {
                case UP: proxPos.x--; break;
                case DOWN: proxPos.x++; break;
                case LEFT: proxPos.y--; break;
                case RIGHT: proxPos.y++; break;
            }

            // Verificação de colisão
            if (proxPos.x < 0 || proxPos.x >= MAP_HEIGHT || proxPos.y < 0 || proxPos.y >= MAP_WIDTH || m[proxPos.x][proxPos.y] == 1 || m[proxPos.x][proxPos.y] == 2) {
                cout << "Game Over! Voce bateu na parede ou comeu a si mesmo!" << endl;
                adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo), "ContraTempo");
                salvarPontuacoes(pontuacoes, nomeArquivo);
                break;
            }

            for (int i = comprimentoCobra - 1; i > 0; i--) {
                snake[i] = snake[i - 1];
            }
            snake[0] = proxPos;

            // Cobra pega a maçã
            if (proxPos.x == maca.x && proxPos.y == maca.y) {
                if (comprimentoCobra < 100) { // Evitar ultrapassar o tamanho do vetor
                    snake[comprimentoCobra] = maca; // Adicionar a maçã ao corpo da cobra
                    comprimentoCobra++;
                    ingameDisplay.pontuacao += 10;
                    ingameDisplay.numMacas++;
                    numMacasPegadas++;
                    maca = geraMaca(snake, comprimentoCobra);
                } else {
                    cout << "Game Over! Cobra muito grande!" << endl;
                    break;
                }
            }

            ingameDisplay.movimentos++;
            ultimoTempo = tempoAtualContadorInGame;
        }

        if (chrono::duration_cast<chrono::milliseconds>(tempoAtualContadorInGame - inicioContadorInGame) >= marcadorTempoInGame) {
            ingameDisplay.tempo++;
            inicioContadorInGame = tempoAtualContadorInGame;
        }

        Sleep(50);
    }

    adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo), "ContraTempo");
    salvarPontuacoes(pontuacoes, nomeArquivo);
}

int main() {
    int menu;
    int currentLevel = 1; // Iniciar no nível 1
    bool portalActive = false;
    Ponto portal = {MAP_HEIGHT / 2, MAP_WIDTH / 2}; // Posição do portal

    do {
        srand(time(NULL));
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(out, &cursorInfo);

        int m[MAP_HEIGHT][MAP_WIDTH];

        Ponto snake[100]; // Comprimento máximo
        int comprimentoCobra = COMPRIMENTO_INICIAL_COBRA;
        for (int i = 0; i < comprimentoCobra; i++) {
            snake[i].x = MAP_HEIGHT / 2;
            snake[i].y = MAP_WIDTH / 2 - i;
        }
        Ponto maca = geraMaca(snake, comprimentoCobra);

        cout << "1. Modo clássico" << endl;
        cout << "2. Modo contra o tempo" <<endl;
        cout << "3. Modo de redução" <<endl;
        cout << "4. Modo autoplay" << endl;
        cout << "5. Desenvolvimento" << endl;
        cout << "6. Ranking" << endl;
        cout << "7. Sair" << endl;

        cin >> menu;
        string nomeArquivo = "rankJogador.txt";
        vector<Pontuacao> pontuacoes = lerPontuacoes(nomeArquivo);

        switch (menu) {
            case 1: {
                string nomeJogador;
                cout << "Digite seu nome: " << endl;
                cin >> nomeJogador;
                replace(nomeJogador.begin(), nomeJogador.end(), ' ', '_');
                system("cls");
                Direction dir = RIGHT;
                char tecla;
                IngameDisplay ingameDisplay;
                auto ultimoTempo = chrono::steady_clock::now();
                auto inicioContadorInGame = chrono::steady_clock::now();

                int numMacasPegadas = 0;
                const auto marcadorTempoInGame = chrono::milliseconds(1000); // Marcador de tempo em jogo
                const auto velocidadeInicial = chrono::milliseconds(300); // Velocidade inicial da cobra
                auto velocidadeCobra = velocidadeInicial; // Velocidade da cobra

                while (true) {
                    auto tempoAtualContadorInGame = steady_clock::now();
                    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
                    criaMapa(m, snake, comprimentoCobra, maca, portal, currentLevel, portalActive);

                    for (int i = 0; i < MAP_HEIGHT; i++) {
                        for (int j = 0; j < MAP_WIDTH; j++) {
                            switch (m[i][j]) {
                                case 0: cout << " "; break; // espaço vazio
                                case 1: cout << char(219); break; // paredes
                                case 2: cout << char(184); break; // corpo da cobra
                                case 4: cout << char(199); break; // cabeça da cobra
                                case 3: cout << char(208); break; // maçã
                                case 5: cout << char(206); break; // portal
                            }
                        }
                        cout << "\n";
                    }

                    if (_kbhit()) {
                        tecla = getch();
                        switch (tecla) {
                            case 72: case 'w':
                                if (dir != DOWN) dir = UP;
                                break;
                            case 80: case 's':
                                if (dir != UP) dir = DOWN;
                                break;
                            case 75: case 'a':
                                if (dir != RIGHT) dir = LEFT;
                                break;
                            case 77: case 'd':
                                if (dir != LEFT) dir = RIGHT;
                                break;
                        }
                    }

                    auto tempoAtual = chrono::steady_clock::now();
                    if (tempoAtual - ultimoTempo >= velocidadeCobra) {
                        ultimoTempo = tempoAtual;

                        Ponto proxPos = snake[0]; // Próxima posição da cabeça da cobra
                        switch (dir) {
                            case UP: proxPos.x--; break;
                            case DOWN: proxPos.x++; break;
                            case LEFT: proxPos.y--; break;
                            case RIGHT: proxPos.y++; break;
                        }

                        // Colisão com a parede
                        if (proxPos.x <= 0 || proxPos.x >= MAP_HEIGHT - 1 || proxPos.y <= 0 || proxPos.y >= MAP_WIDTH - 1) {
                             adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo) + "s", "Classico");
                            salvarPontuacoes(pontuacoes, nomeArquivo);
                            cout << "Game Over!" << endl;
                            break;
                        }

                        // Colisão com o corpo da cobra
                        for (int i = 1; i < comprimentoCobra; i++) {
                            if (snake[i].x == proxPos.x && snake[i].y == proxPos.y) {
                                 adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo) + "s", "Classico");
                                salvarPontuacoes(pontuacoes, nomeArquivo);
                                cout << "Game Over!" << endl;
                                return 0;
                            }
                        }

                        // Se a cobra comer uma maçã
                        if (proxPos.x == maca.x && proxPos.y == maca.y) {
                            comprimentoCobra++;
                            ingameDisplay.pontuacao += 10;
                            ingameDisplay.numMacas++;
                            numMacasPegadas++;
                            if (numMacasPegadas % 2 == 0 && numMacasPegadas <= 10) {
                                velocidadeCobra -= chrono::milliseconds(25); // Acelera a cobra
                            }
                            maca = geraMaca(snake, comprimentoCobra);

                            // Verifica se 100 maçãs foram coletadas para ativar o portal
                            if (ingameDisplay.numMacas >= 3) {
                                portalActive = true;
                            }
                        }

                        // Se a cobra chegar ao portal e o portal estiver ativo
                        if (portalActive && proxPos.x == portal.x && proxPos.y == portal.y) {
                            portalActive = false;
                            currentLevel++;
                            if (currentLevel <= NUM_LEVELS) {
                                comprimentoCobra = COMPRIMENTO_INICIAL_COBRA;
                                for (int i = 0; i < comprimentoCobra; i++) {
                                    snake[i].x = MAP_HEIGHT / 2;
                                    snake[i].y = MAP_WIDTH / 2 - i;
                                }
                                numMacasPegadas = 0;
                                velocidadeCobra = velocidadeInicial; // Reseta a velocidade da cobra
                                ingameDisplay.numMacas = 0;
                                ingameDisplay.pontuacao = ingameDisplay.pontuacao+1000;
                                ingameDisplay.movimentos = 0; // Reseta o contador de movimentos
                                maca = geraMaca(snake, comprimentoCobra);
                                system("cls");
                                continue; // Carrega o novo nível
                            } else {
                                 adicionarPontuacao(pontuacoes, nomeJogador, ingameDisplay.pontuacao, to_string(ingameDisplay.tempo) + "s", "Classico");
                                salvarPontuacoes(pontuacoes, nomeArquivo);
                                cout << "Você completou todos os níveis!" << endl;
                                break;
                            }
                        }

                        // Move a cobra
                        for (int i = comprimentoCobra - 1; i > 0; i--) {
                            snake[i] = snake[i - 1];
                        }
                        snake[0] = proxPos;
                        ingameDisplay.movimentos++; // Incrementa o contador de movimentos
                    }

                    ingameDisplay.tempo = chrono::duration_cast<chrono::seconds>(tempoAtualContadorInGame - inicioContadorInGame).count();
                    cout << "Tempo: " << ingameDisplay.tempo << "s" << endl;
                    cout << "Pontuação: " << ingameDisplay.pontuacao << endl;
                    cout << "Maçãs: " << ingameDisplay.numMacas << endl;
                    cout << "Movimentos: " << ingameDisplay.movimentos << endl;
                    Sleep(50);
                }

                break;
            }
            case 2: {
                jogarModoContraTempo(pontuacoes, nomeArquivo);
                break;
            }
            case 3: {
                 jogarModoReducao(pontuacoes, nomeArquivo);
                 break;
            }
            case 4: {
                jogarModoComputador(pontuacoes, nomeArquivo);
                break;
            }

            case 5:
                cout << "Alunos: João, Matheus, Miguel e Nicolas" << endl;
                cout << "Professor: Alex Rese" << endl;
                cout << "Matéria: Algoritmos II" << endl;
                break;
            case 6:
                exibirRanking(pontuacoes);
                break;
            case 7:
                cout << "Saindo do jogo!" << endl;
                break;
            default:
                cout << "Opção inválida!" << endl;
                break;
        }
    } while (menu != 7);

    return 0;
}
