#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

// TUN 인터페이스 생성, 성공 시 fd 반환
int tun_create(const char *if_name) {
    struct ifreq ifr;
    int fd = open("/dev/net/tun", O_RDWR); // O_RDWR : 읽기/쓰기 모두 가능하게 오픈
    if (fd < 0) {
        perror("open /dev/net/tun");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr)); // 구조체 초기화
    // IFF_TUN : IP 패킷 단위로 처리, IFF_NO_PI : read/write할 때 순수한 IP 패킷만 오고가도록 설정
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI; 
    if (if_name && *if_name)
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    // 커널에 TUN 인터페이스 생성 요청
    if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return -1;
    }

    printf("Created TUN interface: %s\n", ifr.ifr_name);
    return fd;
}