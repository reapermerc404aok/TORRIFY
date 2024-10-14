#include <stdio.h>
#include <stdlib.h>
#include "torrify.h"

Req *request(const char *dstip, const int dstport) {
    Req *req;

    req = (Req *)malloc(reqsize);

    req->vn = 4;
    req->cd = 1;
    req->dstport = htons(dstport);
    req->dstip = inet_addr(dstip);
    strncpy(req->userid, USERNAME, 8);

    return req;
}

int main(int argc, char **argv) {
    char *host;
    int port;
    int s; // Socket file descriptor
    Req *req;
    Res *res;
    char buf[ressize];
    int success;
    char tmp[512];

    struct sockaddr_in sock;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return -1;
    }

    host = argv[1];
    port = atoi(argv[2]);

    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0) {
        perror("socket");
        return -2;
    }

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    if (connect(s, (struct sockaddr *)&sock, sizeof(sock)) != 0) {
        perror("connect");
        return -3;
    }

    printf("Connected to proxy server\n");
    req = request(host, port);

    write(s, req, reqsize);
    memset(buf, 0, ressize);
    if (read(s, buf, ressize) < 1) {
        perror("read");
        free(req);
        close(s);
        return -4;
    }

    res = (Res *)buf;
    success = (res->cd == 90);

    printf("response code: %d\n", res->cd);

    if (!success) {
        fprintf(stderr, "Unable to traverse the proxy. Error code: %d\n",
                res->cd);
        close(s);
        free(req);
        return -5;
    }

    printf("Successfully connected through the proxy through %s:%d\n",
           host, port);

    memset(tmp, 0, 512);
    snprintf(tmp, 511,
             "HEAD / HTTP/1.0\r\n"
             "Host: thankyoudrbirch\r\n"
             "\r\n");
    write(s, tmp, strlen(tmp));
    memset(tmp, 0, 512);
    read(s, tmp, 511);
    printf("'%s'\n", tmp);


    close(s);
    free(req);

    return 0;
}
