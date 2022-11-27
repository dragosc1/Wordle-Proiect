#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <WS2tcpip.h>
#include <map>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

/// lista liniara pentru a retine perechile (cuvant, entropie)
char buf[4096];
SOCKET sock;
string  chosen_word;


vector<pair<string, double>> listcuvinte, lista_redusa;

string getUserInput(SOCKET sock, char buf[4096], string word)
{
    string userInput = "";
    for (int i = 0; i < 5; i++)
        userInput += word[i];
    // Send the text
    int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        
    ZeroMemory(buf, 4096);
    return userInput;
}
string receiveData(SOCKET clientSocket, char buf[4096])
{
    // Wait for server to send data
    string receivedWord = "";
    int bytesReceived = recv(clientSocket, buf, 4096, 0);

    for (int i = 0; i < 5; i++)
        receivedWord += buf[i];

    ZeroMemory(buf, 4096);
    return receivedWord;
}
void init() {
    ifstream g("Entropie descrescator");
    string s;
    double entr;
    while (g >> s) {
        g >> entr;
        listcuvinte.push_back({ s, entr });
    }
    g.close();
}

string cuv_rand, rezult_comp;

char stoc[100][6];
int nr = 0;
int l = 0;

map<string, int> tablou;

int lTablou = 243, nr_total_cuv = 11454;
void my_f_lower(string& s) {
    for (int i = 0; i < s.size(); i++)
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] += 32;
}
void comp(string& s, string& cuvR, string& outp) {
    if (s.size() != 5)
        s.resize(5);
    if (cuvR.size() != 5)
        cuvR.resize(5);
    if (outp.size() != 5)
        outp.resize(5);
    for (int i = 0; i < 5; i++)
        outp[i] = 'y';
    for (int i = 0; i < 5; i++)
        if (s[i] == cuvR[i])
            outp[i] = 'v';
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5 && outp[i] == 'y'; j++) {
            if (i == j)
                continue;
            if (s[i] == cuvR[j])
                outp[i] = 'g';
        }
}

void update_list(string& cuv_ales, string& outp_cuv_ales_cuv_rand) {
    string outp;
    for (int i = lista_redusa.size() - 1; i >= 0; i--) {
        comp(cuv_ales, lista_redusa[i].first, outp);
        if (outp != outp_cuv_ales_cuv_rand)
            lista_redusa.erase(lista_redusa.begin() + i);
    }
}

void setare_probab(string& cuv_ales, int lTablou) {
    tablou.clear();
    string output;
    for (int i = 0; i < lista_redusa.size(); i++) {
        comp(cuv_ales, lista_redusa[i].first, output);
        tablou[output]++;
    }
}

double calc_entropie(string& cuv_ales) {

    setare_probab(cuv_ales, 243);
    double e = 0;
    for (auto it = tablou.begin(); it != tablou.end(); it++)
        if (it->second != 0) {
            double prob = it->second * 1.0 / lista_redusa.size();
            e += prob * (-log2(prob));
        }
    return e;
}

void update_entropie() {
    for (int i = 0; i < listcuvinte.size(); i++)
        listcuvinte[i].second = calc_entropie(listcuvinte[i].first);
}

pair<string, double> entr_maxi() {
    int Maxi = 0;
    for (int i = 1; i < listcuvinte.size(); i++)
        if (listcuvinte[i].second > listcuvinte[Maxi].second)
            Maxi = i;
    pair<string, double> rez = listcuvinte[Maxi];
    listcuvinte.erase(listcuvinte.begin() + Maxi);
    return rez;
}

ofstream p;

void bot(int n, char stoc[100][6], int& nr) {
    pair<string, double> cuv_ales;
    string outp_wordle;
    string chosen_word;
    while (outp_wordle != "vvvvv" && lista_redusa.size() > 0) {
        cuv_ales = entr_maxi();
        if (lista_redusa.size() == 1)
            cuv_ales = lista_redusa[0];
        p << cuv_ales.first << ' ';
        chosen_word = getUserInput(sock, buf, cuv_ales.first);
        outp_wordle = receiveData(sock, buf);
        update_list(cuv_ales.first, outp_wordle);
        update_entropie();
    }
    p << '\n';
}

void main()
{
    int timp_start = time(NULL);
    string ipAddress = "127.0.0.1";			// IP Address of the server
    int port = 54000;						// Listening port on the server
    char buf[4096];
    char wordvar[6] = "unyut";
    ifstream f("cuvinte.in");
    int nr_max_incercari = 0, s_incercari = 0;
    double nr_mediu_incercari = 0;
    p.open("rezultat.out");
    

    // Initialize WinSock
    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0)
    {
        cerr << "Can't start Winsock!" << wsResult << endl;
        return;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cerr << "Can't create socket!" << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        cerr << "Can't connect to server!" << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Do-while loop to send and receive data
    do
    {
        //Wordle Bot
        int nr_max_incercari = 0, s_incercari = 0;
        double nr_mediu_incercari = 0;

        while (f >> cuv_rand) {
            listcuvinte.clear();
            init();
            my_f_lower(cuv_rand);
            p << cuv_rand << "-> ";
            lista_redusa = listcuvinte;
            bot(nr_total_cuv, stoc, nr);
            if (nr > nr_max_incercari)
                nr_max_incercari = nr;
            s_incercari += nr;
        }
        int timp_final = time(NULL);
        cout << "nr max incercari= " << nr_max_incercari << "\nnrmediu_incercari= " << s_incercari * 1.0 / nr_total_cuv << "\ntimp de executie= " << timp_final - timp_start;
        p << "nr max incercari= " << nr_max_incercari << "\nnrmediu_incercari= " << s_incercari * 1.0 / nr_total_cuv << "\ntimp de executie= " << timp_final - timp_start;;
        p.close();
        f.close();


    } while (chosen_word.size() > 0);

    // Close down everything
    closesocket(sock);
    WSACleanup();
}