#include <stdio.h>
#include <stdlib.h>
#include "torrify.h"

Req *request(struct sockaddr_in *sock2) {
    Req *req;

    req = (Req *)malloc(reqsize);

    req->vn = 4;
    req->cd = 1;
    req->dstport = sock2->sin_port;
    req->dstip = sock2->sin_addr.s_addr;
    strncpy(req->userid, USERNAME, 8);

    return req;
}

int connect(int s2, const struct sockaddr *sock2, socklen_t addrlen) {

    int s; // Socket file descriptor
    Req *req;
    Res *res;
    char buf[ressize];
    int success;
    char tmp[512];

    int (*connect_og)(int, const struct sockaddr *, socklen_t);

    struct sockaddr_in sock;


    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0) {
        perror("socket");
        return -2;
    }

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    connect_og = dlsym(RTLD_NEXT, "connect");

    if (connect_og(s, (struct sockaddr *)&sock, sizeof(sock)) != 0) {
        perror("connect");
        return -3;
    }

    printf("Connected to proxy server\n");
    req = request((struct sockaddr_in*)sock2);

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

    printf("Successfully connected through the proxy");

    dup2(s, s2);
    free(req);

    return 0;
}
