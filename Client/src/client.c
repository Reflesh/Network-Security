// src/client.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "client_config.h"

#define MAX_PACKET_SIZE 2000

int tun_create(const char *if_name);

void tls_init(void);
void tls_cleanup(void);
SSL_CTX *create_client_ctx(const char *cert, const char *key, const char *ca);
int tls_read_all(SSL *ssl, void *buf, int len);
int tls_write_all(SSL *ssl, const void *buf, int len);

int main(void) {
    tls_init(); // 라이브러리 초기화

    // 클라이언트용 TLS 컨텍스트 지정
    SSL_CTX *ctx = create_client_ctx(
        CLIENT_CERT_PATH,
        CLIENT_KEY_PATH,
        CA_CERT_PATH
    );
    if (!ctx) {
        fprintf(stderr, "create_client_ctx failed\n");
        return 1;
    }

    // TUN 인터페이스 생성
    int tun_fd = tun_create(CLIENT_TUN_NAME);
    if (tun_fd < 0) {
        fprintf(stderr, "tun_create failed\n");
        SSL_CTX_free(ctx);
        return 1;
    }

    // 서버 TCP 연결
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        close(tun_fd);
        SSL_CTX_free(ctx);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        close(tun_fd);
        SSL_CTX_free(ctx);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        close(tun_fd);
        SSL_CTX_free(ctx);
        return 1;
    }
    printf("TCP connected to %s:%d\n", SERVER_IP, SERVER_PORT);

    // TLS 생성 및 바인딩
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    // handshake 수행
    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "SSL_connect failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock);
        close(tun_fd);
        SSL_CTX_free(ctx);
        return 1;
    }
    printf("TLS handshake complete (client)\n");

    uint8_t buf[MAX_PACKET_SIZE];
    int sock_fd = sock;

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(tun_fd, &rfds);
        FD_SET(sock_fd, &rfds);

        int maxfd = (tun_fd > sock_fd ? tun_fd : sock_fd) + 1;

        // 읽기 이벤트 대기
        int ret = select(maxfd, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        // TUN -> TLS
        if (FD_ISSET(tun_fd, &rfds)) {
            int n = read(tun_fd, buf, sizeof(buf));
            if (n <= 0) {
                perror("read(tun)");
                break;
            }

            uint16_t len = htons((uint16_t)n);
            if (tls_write_all(ssl, &len, sizeof(len)) < 0) {
                fprintf(stderr, "tls_write_all len failed\n");
                break;
            }
            // 실제 패킷 데이터 전송
            if (tls_write_all(ssl, buf, n) < 0) {
                fprintf(stderr, "tls_write_all data failed\n");
                break;
            }
        }

        // TLS -> TUN
        if (FD_ISSET(sock_fd, &rfds)) {
            uint16_t len_n;
            int r = tls_read_all(ssl, &len_n, sizeof(len_n));
            if (r <= 0) {
                fprintf(stderr, "tls_read_all len failed\n");
                break;
            }
            int plen = ntohs(len_n);
            if (plen <= 0 || plen > MAX_PACKET_SIZE) {
                fprintf(stderr, "invalid packet length: %d\n", plen);
                break;
            }

            // 패킷 본문 수신
            r = tls_read_all(ssl, buf, plen);
            if (r <= 0) {
                fprintf(stderr, "tls_read_all data failed\n");
                break;
            }

            // 수신한 패킷을 TUN 인터페이스로 쓰기
            int w = write(tun_fd, buf, plen);
            if (w < 0) {
                perror("write(tun)");
                break;
            }
        }
    }

    // 종료 처리
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    close(tun_fd);
    SSL_CTX_free(ctx);
    tls_cleanup();

    return 0;
}