#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <thread>
#include <format>
#include <fstream>
#include <sstream>
#include "socklib.h"
#include <windows.h>

using namespace std;

class Server: public StrSocket
{
public:
    Server(StrSocket&& s):StrSocket(move(s))
    {
        soRcvTimeout(60);
        soSndTimeout(60);
    }

    void puts(const string& ss)
    {
        StrSocket::puts(ss);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        cout << "<< " << ss << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
    }

    string gets()
    {
        string ss = StrSocket::gets();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE|FOREGROUND_RED);
        cout << ">> " << ss << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
        return ss;
    }

    Server& helo()
    {
        puts("HELO");
        return *this;
    }

    vector<string> readFile()
    {
        vector<string> text;
        string file = gets();
        if (!file.starts_with("FILE "))
            throw runtime_error(format("Unrecognized command {}",file));
        file = file.substr(5);

        ifstream in(file);
        if (!in.is_open())
        {
            puts("FAIL");
            throw runtime_error(format("Cann't open {}",file));
        }
        for(string ss; getline(in,ss); text.push_back(ss));
        in.close();
        cout << "File " << file << " succesfully readed\n";
        puts("OK");
        return text;
    }

    int count(int local_size)
    {
        string ss = gets();
        if (!ss.starts_with("COUNT "))
            throw runtime_error(format("Unrecognized command {}",ss));
        int cnt = stoi(ss.substr(6));
        puts(format("INFO client file: {} lines, server file: {} lines",cnt,local_size));
        puts(format("INFO {} lines will be checked",min(cnt,local_size)));

        puts("GET");
        return min(cnt,local_size);
    }
};

bool quasi(const string& a, const string& b)
{
    istringstream f(a), s(b);
    for(string w1,w2;;)
    {
        f >> w1;
        s >> w2;
        if (!f || !s) break;
        if (w1.size() != w2.size()) return false;
        for(int i = 0; i < w1.size(); ++i)
            if (toupper(w1[i]) != toupper(w2[i])) return false;
    }
    if (f || s) return false;
    return true;
}

void ServerWork(Server&& T)
{
    Server S(move(T));
    ifstream in;
    try {

        vector<string> text = S.helo().readFile();

        int count = S.count(text.size());


        vector<int> equ, quas;
        for(int i = 0; i < count; ++i)
        {
            string s = S.gets();
            //cout << "CLNT: " << s << "\n";
            //cout << "SRVR: " << i+1 << " " << text[i] << "\n";
            if (s == to_string(i+1) + " " + text[i])
            {
                S.puts(format("EQ Lines # {} are equal",i+1));
                equ.push_back(i+1);
            }
            else if (quasi(s,to_string(i+1) + " " + text[i]))
            {
                S.puts(format("QE Lines # {} are quasi-equal",i+1));
                quas.push_back(i+1);
            }
            else S.puts(format("NE Lines # {} are not equal",i+1));
        }

        string ans = "LIST E ";
        for(auto i: equ) ans += to_string(i) + " ";
        S.puts(ans);

        ans = "LIST Q ";
        for(auto i: quas) ans += to_string(i) + " ";
        S.puts(ans);

        S.puts("BYE");

    } catch(exception&e)
    {
        cout << "Thread error: " << e.what() << endl;
    }
}

int main(int argc, char*argv[])
{
    unsigned short int port = (argc > 1) ? atoi(argv[1]) : 7890; // listen on port 7890 by default
    try {
        StrSocket srv(0,port);
        cout << "Server listen port " << port << endl;
        for(;;)
        {
            try {
                auto r = srv.answer(5);
                cout << "Call from " << Socket::addr(r.peer()) << endl;
                thread T(ServerWork,move(r));
                T.detach();
            } catch(exception&e)
            {
                cout << "!!Error: " << e.what() << "\n";
            }
        }
    } catch(exception&e)
    {
        cout << "!!Error: " << e.what() << "\n";
    }
}

