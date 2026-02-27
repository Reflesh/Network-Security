본 프로젝트는 OpenSSL 기반 상호인증을 사용하여 TUN 인터페이스 위에 암호화된 패킷 터널을 구성하는 경량 VPN 시스템입니다.
Linux환경에서 C언어로 구현하였으며 libssl, libcrypto 등의 API를 직접 호출하고 TLS 기반 상호인증, 암호화 스트림 생성, 에러 처리 과정을 코드로 작성하여 암호화 채널 구축하는 과정에 중점을 두었습니다.

해당 파일 내부에는 CA, Server, Client 각각의 개인키와 인증서를 보유하고 있으며, make를 통해 파일 빌드 후 아래의 순서대로 실행하면 VPN의 동작 과정을 확인할 수 있습니다.

1. sudo ./server
2. sudo ./client
3. sudo ./set.sh(client)
4. sudo ./set.sh(server)
5. sudo ip route del default, sudo ip route add default via 10.8.0.1 dev tun_client (Client에서 실행)

<img width="1232" height="153" alt="image" src="https://github.com/user-attachments/assets/ffff4e27-b4ef-49b1-abce-c960c30e0a76" />
handshake가 완료될 경우 다음과 같은 화면을 확인할 수 있습니다. 

프로그램을 실행하기 전 결과에서 다음과 같은 변화를 확인할 수 있습니다. 
<img width="893" height="312" alt="image" src="https://github.com/user-attachments/assets/5e13d9d4-c69a-40e7-9214-8ce9a090e222" />
traceroute 실행 결과 : 프로그램 실행 전
<img width="809" height="88" alt="image" src="https://github.com/user-attachments/assets/e8718c7f-befe-4370-ab8f-d9b9f0a9016a" />
tcpdump(tun_server) 결과 : 프로그램 실행 전

<img width="1405" height="370" alt="image" src="https://github.com/user-attachments/assets/5a54c454-e49e-464d-94f8-c839977051ae" />
tcpdump(tun_server) 결과 : 프로그램 실행 및 환경설정 후
<img width="1410" height="499" alt="image" src="https://github.com/user-attachments/assets/951712c3-01a7-4450-a6ac-74b74c78a269" />
tcpdump(enp0s3) 결과 : 프로그램 실행 및 환경설정 후
<img width="512" height="187" alt="image" src="https://github.com/user-attachments/assets/e14fb55f-2754-4c91-add3-f37886eb5f67" />
traceroute 결과 : 프로그램 실행 및 환경설정 후

<참고자료>
- https://docs.openssl.org/master/man3/ : OpenSSL에 대한 각종 라이브러리 사용 방법을 참고하였습니다.(SSL_free, SSL_CTX_free, SSL_shutdown, SSL_CTX_new 등)
- https://man7.org/linux/man-pages/man2/ : socket, connect와 같은 함수 사용 방법을 참고하였습니다.
- https://fm4dd.com/openssl/sslconnect.shtm/ : tls_init, tls_cleanup 함수의 초기화 및 정리 패턴을 참고하였습니다
- https://aticleworld.com/ssl-server-client-using-openssl-in-c/ : SSL/TLS 예제 코드를 참고하여 tls_read_all(), tls_write_all()을 작성하였습니다.
