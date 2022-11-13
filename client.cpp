#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <format>
#include "socklib.h"
#include <windows.h>

using namespace std;

bool verbose = false;

class Client: public StrSocket
{
public:
    Client()
    {
        soRcvTimeout(60);
        soSndTimeout(60);
    }

    void puts(const string& ss)
    {
        StrSocket::puts(ss);
        if (verbose)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "<< " << ss << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
        }
    }

    string gets()
    {
        string ss = StrSocket::gets();
        if (verbose)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE|FOREGROUND_RED);
            cout << ">> " << ss << endl;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
        }
        return ss;
    }

    Client& helo()
    {
        if (gets() != "HELO")
            throw runtime_error("Wrong HELO answer, quit");
        return *this;
    }

    void getFile()
    {
        string file;
        for(;;)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_GREEN);
            cout << "Enter filename, pls: ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
            getline(cin,file);
            in.open(file);
            if (!in.is_open())
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_RED);
                cout << "Can not open file " << file << endl;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
            }
            else break;
        }
        file = "FILE "s + file;
        puts(file);
        if (gets() != "OK")
            throw runtime_error(format("Server can not find file {}",file.substr(5)));
    }

    Client& readFile()
    {
        getFile();
        for(string ss; getline(in,ss); text.push_back(ss));
        in.close();
        cout << "File succesfully readed\n";

        puts("COUNT "s + to_string(text.size()));
        for(string ans = gets(); ans != "GET"; ans = gets());
        return *this;
    }
    Client& sendFile()
    {
        for(int i = 0; i < text.size(); ++i)
        {
            puts(format("{} {}",i+1,text[i]));
            gets();
        }
        return *this;
    }


private:
    ifstream in;
    vector<string> text;
};

int main(int argc, char * argv[])
{
    string server = "localhost";
    int port = 7890;
    for(int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-' && argv[i][0] != '/')
        {
            server = argv[i];
            if (size_t pos = server.find(':'); pos != string::npos)
            {
                port = atoi(server.c_str() + pos + 1);
                server[pos] = 0;
            }
        }
        else if (toupper(argv[i][1]) == 'V') verbose = true;
    }
    try
    {
        Client me;
        me.call(server,port);
        me.helo().readFile().sendFile();

        string lst, equ, qua;
        while((lst = me.gets()).starts_with("LIST "))
        {
            if ((lst[5] == 'E') && lst.size() > 7) equ = lst.substr(7);
            if ((lst[5] == 'Q') && lst.size() > 7) qua = lst.substr(7);
        }

        if (!equ.empty()) cout << "Numbers of equal stings: " << equ << "\n";
        if (!qua.empty()) cout << "Numbers of quasy stings: " << qua << "\n";

        if (lst == "BYE")
        {
            cout << "Session ended\n";
        }
    } catch(exception&e)
    {
        cout << "Error: " << e.what() << "\n";
    }

}

