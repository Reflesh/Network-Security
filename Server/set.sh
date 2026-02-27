#!/bin/bash
# === Mini VPN 서버측 네트워크 설정 스크립트 (최소 변경 버전) ===
# - tun_server 에 10.8.0.1/24 IP 부여 (이미 있으면 무시)
# - 인터페이스 UP
# - IP 포워딩 + NAT 설정

VPN_NET="10.8.0.0/24"
VPN_TUN="tun_server"
VPN_IP="10.8.0.1"
WAN_IF="enp0s3"

echo "[*] 서버 TUN 인터페이스 설정 시작..."

# 1) tun_server 에 IP 시도 (없으면 커널이 'Cannot find device' 출력만 하고, 스크립트는 계속 진행)
echo "    - ${VPN_TUN} 에 ${VPN_IP}/24 추가 시도"
sudo ip addr add ${VPN_IP}/24 dev ${VPN_TUN} 2>&1 || echo "      (경고) tun_server 에 IP 추가 실패 - 나중에 서버 실행 후 다시 sh 실행해도 됩니다."

# 2) 인터페이스 UP 시도
echo "    - ${VPN_TUN} UP 시도"
sudo ip link set ${VPN_TUN} up 2>&1 || echo "      (경고) tun_server 를 UP 하지 못했습니다."

echo
echo "[*] 현재 ${VPN_TUN} 상태 (없으면 아래에 아무 것도 안 나올 수 있습니다):"
ip addr show ${VPN_TUN} 2>/dev/null || echo "    (정보) ip addr show ${VPN_TUN} 실패 - 아직 생성 전일 수 있습니다."

echo
echo "[*] IPv4 포워딩 활성화..."
sudo sysctl -w net.ipv4.ip_forward=1

echo
echo "[*] NAT(MASQUERADE) 규칙 추가 시도..."
sudo iptables -t nat -C POSTROUTING -s ${VPN_NET} -o ${WAN_IF} -j MASQUERADE 2>/dev/null \
  || sudo iptables -t nat -A POSTROUTING -s ${VPN_NET} -o ${WAN_IF} -j MASQUERADE

sudo iptables -C FORWARD -i ${VPN_TUN} -o ${WAN_IF} -j ACCEPT 2>/dev/null \
  || sudo iptables -A FORWARD -i ${VPN_TUN} -o ${WAN_IF} -j ACCEPT

sudo iptables -C FORWARD -i ${WAN_IF} -o ${VPN_TUN} -m state --state ESTABLISHED,RELATED -j ACCEPT 2>/dev/null \
  || sudo iptables -A FORWARD -i ${WAN_IF} -o ${VPN_TUN} -m state --state ESTABLISHED,RELATED -j ACCEPT

echo
echo "[*] 서버 측 기본 설정 스크립트 종료."
echo "    - tun_server 가 이미 생성되어 있었다면 10.8.0.1/24 가 붙어 있어야 합니다."
echo "    - 안 붙었으면: 서버를 sudo 로 실행한 뒤, 이 스크립트를 한 번 더 실행해 보십시오."

