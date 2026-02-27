#!/bin/bash
# === Mini VPN 클라이언트측 네트워크 설정 스크립트 (최소 변경 버전) ===
# - tun_client 에 10.8.0.2/24 IP 부여 (이미 있으면 무시)
# - 인터페이스 UP
# - 서버와의 Host-Only 경로 유지
# - VPN 내부 네트워크(10.8.0.0/24) 를 tun_client 로 라우팅
#   (기본 게이트웨이는 바꾸지 않음)

VPN_TUN="tun_client"
VPN_IP="10.8.0.2"
VPN_NET="10.8.0.0/24"
VPN_GW="10.8.0.1"

SERVER_NET="192.168.56.0/24"
SERVER_IF="eth1"

echo "[*] 클라이언트 TUN 인터페이스 설정 시작..."

# 1) tun_client 에 IP 시도
echo "    - ${VPN_TUN} 에 ${VPN_IP}/24 추가 시도"
sudo ip addr add ${VPN_IP}/24 dev ${VPN_TUN} 2>&1 || echo "      (경고) tun_client 에 IP 추가 실패 - 나중에 클라이언트 실행 후 다시 sh 실행해도 됩니다."

# 2) 인터페이스 UP 시도
echo "    - ${VPN_TUN} UP 시도"
sudo ip link set ${VPN_TUN} up 2>&1 || echo "      (경고) tun_client 를 UP 하지 못했습니다."

echo
echo "[*] 현재 ${VPN_TUN} 상태 (없으면 아래에 아무 것도 안 나올 수 있습니다):"
ip addr show ${VPN_TUN} 2>/dev/null || echo "    (정보) ip addr show ${VPN_TUN} 실패 - 아직 생성 전일 수 있습니다."

echo
echo "[*] 서버 Host-Only 네트워크(192.168.56.0/24) 경로 유지 설정..."
sudo ip route add ${SERVER_NET} dev ${SERVER_IF} 2>/dev/null || true

echo
echo "[*] VPN 내부 네트워크(10.8.0.0/24) 라우트 추가..."
sudo ip route add ${VPN_NET} dev ${VPN_TUN} 2>/dev/null || true

echo
echo "[*] 현재 관련 라우팅 테이블:"
ip route | egrep '10.8.0.0|192.168.56.0' || echo "    (정보) 해당 라우트가 아직 없을 수 있습니다."

echo
echo "[!] 지금은 터널만 확인하기 위해 최소 설정만 적용했습니다."
echo "    - 먼저 'ping 10.8.0.1' 이 되는지만 확인하십시오."
echo "    - 기본 게이트웨이를 VPN으로 바꾸는 설정은 나중에 별도로 적용하는 것이 안전합니다."
