#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

string receiveData(SOCKET clientSocket, char buf[4096])
{
    // Wait for client to send data
    string receivedWord ="";
    int bytesReceived = recv(clientSocket, buf, 4096, 0);
    for(int i=0;i < 5;i++)
        receivedWord += buf[i];
    if (bytesReceived == SOCKET_ERROR)
    {
        cerr << "Error in reciving data!" << endl;
        return "";
    }

    if (bytesReceived == 0)
    {
        cout << "Client disconnected " << endl;
        return "";
    }

    cout << string(buf, 0, bytesReceived) << endl;
    ZeroMemory(buf, 4096);
    return receivedWord;
}

class Word {
private:
	char word[5];

public:
	Word() {}

	Word(std::string stringword) {
		for (int i = 0; i < 5; i++)
			word[i] = stringword[i];
	}

	void setChar(int position, char c) {
		word[position] = c;
	}

	char getChar(int position) {
		return word[position];
	}

	void print() {
		for (int i = 0; i < 5; i++)
			std::cout << word[i];
	}

	bool isEqual(Word w) {
		for (int i = 0; i < 5; i++)
		{
			if (word[i] != w.getChar(i))
				return 0;
		}
		return 1;
	}
	bool isGreater(Word w) {
		for (int i = 0; i < 5; i++)
			if (word[i] < w.getChar(i))
				return 0;
			else
				if (word[i] > w.getChar(i))
					return 1;
		return 1;
	}
};

class WordDatabase {

public:
	std::vector<Word> words; // list of all words
	WordDatabase(const std::string& wordDatabaseFile) {
		std::ifstream f(wordDatabaseFile);
		std::string stringword;

		while (f >> stringword) {
			words.push_back(Word(stringword));
		}
		f.close();
	};
	std::vector<Word> GetCopyOfVector()
	{
		return words;
	}
	Word chooseRandomWord() {
		srand((unsigned int)time(0));
		Word word = words[(1LL * rand() * rand()) % words.size()];
		return word;
	};

};


int row = 0, col = 0;
char charact[4][10] = { {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'}, { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' },  { 'Z', 'X', 'C', 'V', 'B', 'N', 'M' } };
std::pair<int, int> char_to_pos[128];

void main()
{
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsStatus = WSAStartup(ver, &wsData);
	if (wsStatus != 0)
	{
		cerr << "Can't Initialize winsock!" << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket!" << endl;
		return;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Service the client is connect on

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected on port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " <<
			ntohs(client.sin_port) << endl;
	}

	// Close listening socket
	closesocket(listening);

	// While loop: accept
	char buf[4096] = "";
    bool isDone = 0;
	while (true && !isDone)
	{
		ZeroMemory(buf, 4096);

		//Wordle program

        WordDatabase wordDatabase("cuvinte.in");

        Word chosenWord = wordDatabase.chooseRandomWord();
        chosenWord.print();
        cout << "\n";
        std::vector<Word> wordscpy = wordDatabase.GetCopyOfVector();
        string testWord = "";
        Word wordTable[6];

        sf::Event ev;
        sf::RenderWindow window(sf::VideoMode(1024, 860), "Wordle", sf::Style::Default);
        while (window.isOpen()) {
            if (isDone) {
                // you can only close the window
                if (window.pollEvent(ev))
                    switch (ev.type) {
                    case sf::Event::EventType::Closed:
                        window.close();
                        break;
                    case sf::Event::KeyPressed:
                        if (ev.key.code == sf::Keyboard::Escape)
                            window.close();
                    }
                continue;
            }
            testWord = receiveData(clientSocket, buf);
            
            // Update
            for (int inx = 0; inx < 6; inx++)
            {
                if (inx == 5)
                {
                    bool ok = 1;
                    for (int i = 0; i < 5 && ok; i++)
                        if (chosenWord.getChar(i) != wordTable[row].getChar(i))
                            ok = 0;
                    if (ok)
                        isDone = 1;
                    row++, col = 0;
                    // Cicle if you entered more than 6 lines
                    if (row == 6 && !isDone) {
                        for (int i = 0; i < 5; i++)
                            for (int j = 0; j < 5; j++)
                                wordTable[i].setChar(j, wordTable[i + 1].getChar(j));
                        for (int i = 0; i < 5; i++)
                            wordTable[5].setChar(i, 0);
                        row = 5, col = 0;
                    }
                    continue;
                }
                char letter_pressed = testWord[inx] - 'a' + 'A';
                wordTable[row].setChar(col, letter_pressed);
                col++;
                    
            }
            
            // Render
            window.clear(sf::Color::Black);
        // Draw Wordle

            // Title
            sf::Font font;
            if (!font.loadFromFile("karnak-small-normal-400.ttf")) {
                std::cout << "Font not loaded!";
            }
            sf::Text title;
            title.setFont(font);
            title.setString("Wordle");
            title.setCharacterSize(50);
            title.setFillColor(sf::Color::White);
            title.setStyle(sf::Text::Bold);
            sf::FloatRect titleRect = title.getLocalBounds();
            title.setOrigin(titleRect.left + titleRect.width / 2.0f, 0.f);
            title.setPosition(window.getView().getCenter().x, 10.f);
            window.draw(title);

            // Create table(6, 5)
            const float Y_DIF = window.getView().getSize().y / 10;
            sf::RectangleShape cell(sf::Vector2f(70.f, 70.f));
            cell.setFillColor(sf::Color(255, 255, 255, 85));
            cell.setPosition(window.getView().getSize().x / 3.25, Y_DIF);
            std::vector<std::vector<sf::RectangleShape>> table(6, std::vector<sf::RectangleShape>(5));
            for (int i = 0; i < table.size(); i++) {
                table[i][0] = cell;
                table[i][0].setPosition(table[i][0].getPosition().x, i * table[i][0].getSize().y + 10.f * i + table[i][0].getPosition().y);
                for (int j = 1; j < table[i].size(); j++) {
                    table[i][j] = table[i][j - 1];
                    table[i][j].setPosition(table[i][j - 1].getPosition().x + table[i][j - 1].getSize().x + 10.f, table[i][j - 1].getPosition().y);
                }
            }

            // Keyboard
            const float Y_DIF2 = window.getView().getSize().y / 1.40;
            sf::RectangleShape cell2(sf::Vector2f(70.f, 70.f));
            if (!font.loadFromFile("ClearSans-Medium.ttf")) {
                std::cout << "Font not loaded!";
            }
            sf::Text letter;
            letter.setFont(font);
            letter.setString("Wordle");
            letter.setCharacterSize(35);
            letter.setFillColor(sf::Color::White);
            letter.setStyle(sf::Text::Bold);
            letter.setOutlineColor(sf::Color::White);
            letter.setOutlineThickness(0.5f);
            letter.setPosition(window.getView().getSize().x / 8, Y_DIF2 + 8.5f);
            cell2.setFillColor(sf::Color(255, 255, 255, 110));
            cell2.setPosition(window.getView().getSize().x / 9, Y_DIF2);
            cell2.setOutlineColor(sf::Color(255, 255, 255, 190));
            cell2.setOutlineThickness(1.f);

            std::vector<std::vector<sf::RectangleShape>> keyb(3, std::vector<sf::RectangleShape>(10));
            std::vector<std::vector<sf::Text>> letters(3, std::vector<sf::Text>(10));

            for (int i = 0; i < 10; i++) {
                if (i == 0) {
                    keyb[0][i] = cell2;
                    letters[0][i] = letter;
                }
                else {
                    keyb[0][i] = keyb[0][i - 1];
                    keyb[0][i].setPosition(keyb[0][i - 1].getPosition().x + keyb[0][i - 1].getSize().x + 10.f, keyb[0][i - 1].getPosition().y);
                    letters[0][i] = letters[0][i - 1];
                    letters[0][i].setPosition(letters[0][i - 1].getPosition().x + 81.3f, letters[0][i - 1].getPosition().y);
                }
                letters[0][i].setString(charact[0][i]);
            }
            for (int i = 0; i < 9; i++) {
                if (i == 0) {
                    keyb[1][i] = cell2;
                    keyb[1][i].setPosition(cell2.getPosition().x + (cell2.getSize().x + 10.f) / 2, cell2.getPosition().y + cell2.getSize().y + 11.f);
                    letters[1][i] = letter;
                    letters[1][i].setPosition(letter.getPosition().x + (81.3f + 10.f) / 2, letter.getPosition().y + 72.f + 11.f);
                }
                else {
                    keyb[1][i] = keyb[1][i - 1];
                    keyb[1][i].setPosition(keyb[1][i - 1].getPosition().x + keyb[1][i - 1].getSize().x + 10.f, keyb[1][i - 1].getPosition().y);
                    letters[1][i] = letters[1][i - 1];
                    letters[1][i].setPosition(letters[1][i - 1].getPosition().x + 81.3f, letters[1][i - 1].getPosition().y);
                }
                letters[1][i].setString(charact[1][i]);
            }
            for (int i = 0; i < 9; i++) {
                if (i == 0) {
                    keyb[2][i] = cell2;
                    keyb[2][i].setPosition(cell2.getPosition().x, cell2.getPosition().y + 2 * (cell2.getSize().y + 11.f));
                    letters[2][i] = letter;

                    letters[2][i].setPosition(letter.getPosition().x, letter.getPosition().y + 170.f);
                    letters[2][i].setString("ENTER");
                    letters[2][i].setCharacterSize(27);
                }
                else {
                    keyb[2][i] = keyb[2][i - 1];
                    keyb[2][i].setPosition(keyb[2][i - 1].getPosition().x + keyb[2][i - 1].getSize().x + 10.f, keyb[2][i - 1].getPosition().y);
                    letters[2][i] = letters[2][i - 1];
                    letters[2][i].setPosition(letters[2][i - 1].getPosition().x + letters[2][i - 1].getString().getSize() * 11.3 + 69.2, letters[2][i - 1].getPosition().y);
                }
                if (i > 0 && i < 8) {
                    letters[2][i].setString(charact[2][i - 1]);
                    letters[2][i].setCharacterSize(letter.getCharacterSize());
                }
                else if (i == 8)
                    letters[2][i].setString(L" BS");
                if (i == 0 || i == 8)
                    keyb[2][i].setSize(sf::Vector2f(keyb[2][i].getSize().x * 3 / 2 + 5.0f, keyb[2][i].getSize().y));
                else if (i == 1) {
                    letters[2][i].setPosition(letters[2][i].getPosition().x, letters[2][i].getPosition().y - 5.f);
                    keyb[2][i].setSize(sf::Vector2f((keyb[2][i].getSize().x - 5.0f) * 2 / 3, keyb[2][i].getSize().y));
                }
            }

            // create char to pos
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < (i == 0 ? 10 : 9); j++)
                    char_to_pos[charact[i][j]] = { i, j + (i == 2 ? 1 : 0) };

            // check if the matching of the word on the previous rows
            char chos[6] = "";
            chos[5] = 0;
            
            for (int i = 0; i < row; i++)
                for (int j = 0; j < 5; j++)
                    if (chosenWord.getChar(j) == wordTable[i].getChar(j)) {
                        table[i][j].setFillColor(sf::Color(0, 170, 0));
                        std::pair<int, int> x = char_to_pos[wordTable[i].getChar(j)];
                        keyb[x.first][x.second].setFillColor(sf::Color(0, 170, 0));
                        keyb[x.first][x.second].setOutlineThickness(0.f);
                        if (col == 0)
                            chos[j] = 'v';

                    }
                    else {
                        bool ok = 1;
                        for (int k = 0; k < 5 && ok; k++)
                            if (wordTable[i].getChar(j) == chosenWord.getChar(k))
                                ok = 0;
                        if (ok) {
                            std::pair<int, int> x = char_to_pos[wordTable[i].getChar(j)];
                            keyb[x.first][x.second].setFillColor(sf::Color(255, 255, 255, 75));
                            keyb[x.first][x.second].setOutlineThickness(0.f);
                            chos[j] = 'y';
                        }
                        else {
                                std::pair<int, int> x = char_to_pos[wordTable[i].getChar(j)];
                                keyb[x.first][x.second].setFillColor(sf::Color(255, 255, 0, 175));
                                keyb[x.first][x.second].setOutlineThickness(0.f);
                                table[i][j].setFillColor(sf::Color(255, 255, 0, 175));
                                chos[j] = 'g';
                        }
                    }

            send(clientSocket, chos, 4096, 0);
            ZeroMemory(buf, 4096);
            // Draw table
            for (int i = 0; i < table.size(); i++)
                for (int j = 0; j < table[i].size(); j++)
                    window.draw(table[i][j]);

            // Draw the letters
            sf::Text let;
            if (!font.loadFromFile("arial.ttf"))
                std::cout << "Font not loaded!";
            let.setFont(font);
            let.setCharacterSize(40);
            let.setFillColor(sf::Color::White);
            let.setStyle(sf::Text::Bold);
            let.setOutlineColor(sf::Color::White);
            let.setOutlineThickness(1.5f);
            for (int i = 0; i <= row; i++)
                for (int j = 0; j < (i == row ? col : 5); j++) {
                    // if we don't have a letter (ERROR from BS) ignore
                    if (wordTable[i].getChar(j) == 0)
                        continue;
                    // set the letter in the right square
                    let.setString(wordTable[i].getChar(j));
                    let.setPosition(j == 0 ? window.getView().getSize().x / 3.25 + 20 : window.getView().getSize().x / 3.25 + 20 + 80 * j, (i == 0 ? Y_DIF + 8 : Y_DIF + 80 * i + 8));
                    window.draw(let);
                }

            // Draw the keyboard
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < (i == 0 ? 10 : 9); j++)
                    window.draw(keyb[i][j]);

            // Draw the letters
            // special character enter
            letters[2][0].setPosition(letters[2][0].getPosition().x - 7, letters[2][0].getPosition().y);
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < (i == 0 ? 10 : 9); j++)
                    window.draw(letters[i][j]);

            // Display changes
            window.display();

        }
	}
    
	// Close the socket
	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();

	system("pause");
}
