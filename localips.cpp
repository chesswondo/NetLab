// Локальные адреса

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include "socklib.h"

#pragma comment(lib,"ws2_32.lib")


int main(int argc, const char * argv[])
{
    char chInfo[64];
    if (!gethostname(chInfo, sizeof(chInfo)))
    {
        struct hostent *sh;
        sh=gethostbyname((char*)&chInfo);
        if (sh!=NULL)
        {
            int nAdapter = 0;
            while (sh->h_addr_list[nAdapter])
            {
                struct sockaddr_in adr;
                memcpy(&adr.sin_addr, sh->h_addr_list[nAdapter], sh->h_length);
                printf("%s\n", inet_ntoa(adr.sin_addr));
                nAdapter++;
            }
        }
    }

    Socket s;
}


