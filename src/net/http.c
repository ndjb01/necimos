#include "net.h"
#include "../kernel.h"
#include "../vga.h"

static int started = 0, done = 0;

void http_start(uint32_t ip, const char *host, const char *path){
    if (tcp_connect(ip, 80) != 0){
        kprintf("[http] connect failed\n");
        done = 1;
        return;
    }
    char req[512];
    req[0] = 0;
    strcat(req, "GET ");
    strcat(req, path);
    strcat(req, " HTTP/1.1\r\nHost: ");
    strcat(req, host);
    strcat(req, "\r\nUser-Agent: necimos/0.1\r\nConnection: close\r\n\r\n");
    tcp_send((uint8_t*)req, (uint16_t)strlen(req));
    char ipb[16]; ip_to_str(ip, ipb);
    kprintf("[http] GET %s -> %s\n", path, ipb);
    started = 1;
}

int http_poll(void){
    if (done) return 1;
    if (!started) return 0;
    uint8_t tmp[512];
    int n;
    while ((n = tcp_receive(tmp, sizeof(tmp))) > 0)
        for (int i = 0; i < n; i++) vga_putc((char)tmp[i]);
    if (tcp_closed()){
        done = 1;
        kprintf("\n[http] done\n");
        return 1;
    }
    return 0;
}
