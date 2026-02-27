#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>

// OpenSSL 에러 메시지 출력
static void print_errors(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    ERR_print_errors_fp(stderr);
}

// OpenSSL 초기화
void tls_init(void) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

// OpenSSL 정리
void tls_cleanup(void) {
    EVP_cleanup();
    ERR_free_strings();
}

// 서버용 SSL_CTX 생성 및 기본 설정
SSL_CTX *create_server_ctx(const char *cert, const char *key, const char *ca) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        print_errors("SSL_CTX_new(server) failed");
        return NULL;
    }

    // 서버 인증서 로드
    if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
        print_errors("SSL_CTX_use_certificate_file(server) failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    // 서버 개인키 로드
    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
        print_errors("SSL_CTX_use_PrivateKey_file(server) failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    // CA 인증서 로드
    if (!SSL_CTX_load_verify_locations(ctx, ca, NULL)) {
        print_errors("SSL_CTX_load_verify_locations(server) failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL); // 클라이언트 인증 강제
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5"); // 암호 스위트 설정

    return ctx;
}

// 클라이언트용 SSL_CTX 생성 및 기본 설정
SSL_CTX *create_client_ctx(const char *cert, const char *key, const char *ca) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        print_errors("SSL_CTX_new(client) failed");
        return NULL;
    }

    // 클라이언트 인증서 로드
    if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
        print_errors("SSL_CTX_use_certificate_file(client) failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    // 클라이언트 개인키 로드
    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
        print_errors("SSL_CTX_use_PrivateKey_file(client) failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    // CA 인증서 로드
    if (!SSL_CTX_load_verify_locations(ctx, ca, NULL)) {
        print_errors("SSL_CTX_load_verify_locations(client) failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL); // 서버 인증서 검증만 수행
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5"); // 암호 스위트 설정

    return ctx;
}

// TLS로 len바이트 크기만큼 읽기
int tls_read_all(SSL *ssl, void *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = SSL_read(ssl, (char *)buf + total, len - total);
        if (n <= 0) {
            return -1;
        }
        total += n;
    }
    return total;
}

// TLS로 len바이트 크기만큼 쓰기
int tls_write_all(SSL *ssl, const void *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = SSL_write(ssl, (const char *)buf + total, len - total);
        if (n <= 0) {
            return -1;
        }
        total += n;
    }
    return total;
}