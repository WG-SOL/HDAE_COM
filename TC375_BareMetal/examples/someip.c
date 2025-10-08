#include "someip.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "GPIO.h"
#include "my_stdio.h"
#include "Configuration.h"
#include <string.h>
#include "etc.h"
#include "Ifx_Lwip.h"

#if LWIP_UDP

struct udp_pcb *g_SOMEIPSD_PCB;
struct udp_pcb *g_SOMEIPSERVICE_PCB;


void SOMEIPSD_Recv_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16 port);
void SOMEIP_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16 port);

//added
typedef struct {
    uint16_t svc;
    uint16_t inst;
    uint8_t  major;   // 0xFF
    uint32_t minor;   // 0
    uint8_t  proto;   // 0=UDP 1=TCP
    uint16_t port;    // 30509 (LED)
    const char *name; // 서비스 명
} svc_desc_t;

static const svc_desc_t g_services[] = {
    { 0x0100, 0x0001, 0xFF, 0, 0, PN_SERVICE_1, "LED" },
};

static inline void be16(uint8_t *p, uint16_t v){ p[0]=(v>>8)&0xFF; p[1]=v&0xFF; }
static inline void be32(uint8_t *p, uint32_t v){ p[0]=(v>>24)&0xFF; p[1]=(v>>16)&0xFF; p[2]=(v>>8)&0xFF; p[3]=v&0xFF; }


void SOMEIPSD_Init(void)
{
    /* SOME/IP-SD Init */
    g_SOMEIPSD_PCB = udp_new();
    if (g_SOMEIPSD_PCB)
    {
        /* bind pcb to the 30490 port */
        /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
        err_t err = udp_bind(g_SOMEIPSD_PCB, IP_ADDR_ANY, PN_SOMEIPSD);
        delay_ms(100);
        if (err == ERR_OK) {
            /* Set a receive callback for the pcb */
            udp_recv(g_SOMEIPSD_PCB, (void *)SOMEIPSD_Recv_Callback, NULL);
            my_printf("SOME/IP-SD PCB Initialized\n");
        } else {
            udp_remove(g_SOMEIPSD_PCB);
            my_printf("SOME/IP-SD PCB init failed\n");
        }
    }
}

void SOMEIP_Init(void)
{
    /* SOME/IP Service1 Init */
    g_SOMEIPSERVICE_PCB = udp_new();
    if (g_SOMEIPSERVICE_PCB)
    {
        /* bind pcb to the 5000 port */
        /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
        err_t err = udp_bind(g_SOMEIPSERVICE_PCB, IP_ADDR_ANY, PN_SERVICE_1);
        delay_ms(100);
        if (err == ERR_OK) {
            /* Set a receive callback for the pcb */

            /*ip4_addr_t maddr;
            IP4_ADDR(&maddr, 244, 244, 244, 245);

            extern struct netif *netif_default;
            if(netif_default){
                igmp_joingroup(netif_ip4_addr(netif_defualt), &maddr);
            }*/

            udp_recv(g_SOMEIPSERVICE_PCB, (void *)SOMEIP_Callback, NULL);
            my_printf("SOME/IP Service PCB Initialized!\n");
        } else {
            udp_remove(g_SOMEIPSERVICE_PCB);
            my_printf("SOME/IP Service PCB init failed!\n");
        }
    }
}

uint32 g_InteriorLightControlCount = 0;
void SrvWelcomeLightInteriorControl(uint8 LightOnOff)
{
	/* LED on if LightOnOff=1 otherwise, LED off */
    GPIO_SetLed(1, LightOnOff);
    my_printf("LED1 state Changed\n");
}

uint32 g_ExteriorLightControlCount = 0;
void SrvWelcomeLightExteriorControl(uint8 LightOnOff)
{
	/* LED on if LightOnOff=1 otherwise, LED off */
    GPIO_SetLed(2, LightOnOff);
    my_printf("LED SET 2\n");
	g_ExteriorLightControlCount++;
}


void SOMEIPSD_SendOfferService(unsigned char ip_a, unsigned char ip_b, unsigned char ip_c, unsigned char ip_d)
{
	err_t err;
	uint8 MSG_OfferService[] = {
			0xFF, 0xFF, 0x81, 0x00,  //SOMEIP Header
			0x00, 0x00, 0x00, 0x30,  //SOMEIP Header Length
			0x00, 0x00, 0x00, 0x01,  //Request ID
			0x01, 0x01, 0x02, 0x00,  //SOMEIP version information
			0xC0, 0x00, 0x00, 0x00,  //SOMEIP-SD Flags
			0x00, 0x00, 0x00, 0x10,
			0x01, 0x00, 0x00, 0x10,
			0x01, 0x00, 0x00, 0x01,
			0x00, 0x00, 0x00, 0x0A,
			0x00, 0x00, 0x00, 0x01,
			0x00, 0x00, 0x00, 0x0C,  //Options Array
			0x00, 0x09, 0x04, 0x00,
			0xC0, 0xA8, 0x02, 0x14,  //IP
			0x00, 0x11, 0x77, 0x2D  //Port
	};

	// set ip address from lwip
	uint8 *ip = Ifx_Lwip_getIpAddrPtr();
	MSG_OfferService[48] = ip[0];
	MSG_OfferService[49] = ip[1];
	MSG_OfferService[50] = ip[2];
	MSG_OfferService[51] = ip[3];

	// set supported protocol for SOME/IP
	MSG_OfferService[53] = 0x11;  // UDP only

	MSG_OfferService[54] = (uint8)(PN_SERVICE_1 >> 8) & 0xff;
	MSG_OfferService[55] = (uint8)PN_SERVICE_1 & 0xff;

	struct pbuf *txbuf = pbuf_alloc(PBUF_TRANSPORT, sizeof(MSG_OfferService), PBUF_RAM);
	if (txbuf != NULL) {
		udp_connect(g_SOMEIPSD_PCB, IP_ADDR_BROADCAST, PN_SOMEIPSD);
		pbuf_take(txbuf, MSG_OfferService, sizeof(MSG_OfferService));

		ip_addr_t destination_ip;
		IP4_ADDR(&destination_ip, ip_a, ip_b, ip_c, ip_d);
		u16_t destination_port = PN_SOMEIPSD;

		err = udp_sendto(g_SOMEIPSD_PCB, txbuf, &destination_ip, destination_port);
		if (err == ERR_OK) {
			my_printf("Send Offer Service Success !! \n");
		} else {
			my_printf("Send Offer Service Failed !! \n");
		}
		udp_disconnect(g_SOMEIPSD_PCB);

		pbuf_free(txbuf);
	} else {
		my_printf("Failed to allocate memory for UDP packet buffer.\n");
	}
}

void SOMEIPSD_SendSubEvtGrpAck(unsigned char ip_a, unsigned char ip_b, unsigned char ip_c, unsigned char ip_d)
{
	err_t err;
	uint8 MSG_SubEvtGrpAck[] = {
			0xFF, 0xFF, 0x81, 0x00,  //SOMEIP Header
			0x00, 0x00, 0x00, 0x28,  //SOMEIP Header Length
			0x00, 0x00, 0x00, 0x01,  //Request ID
			0x01, 0x01, 0x02, 0x00,  //SOMEIP version information
			0xC0, 0x00, 0x00, 0x00,  //SOMEIP-SD Flags
			0x00, 0x00, 0x00, 0x10,
			0x07, 0x00, 0x00, 0x00,
			0x01, 0x00, 0x00, 0x01,
			0x00, 0x00, 0x00, 0x0A,
			0x00, 0x00, 0x00, 0x01
	};

	struct pbuf *txbuf = pbuf_alloc(PBUF_TRANSPORT, sizeof(MSG_SubEvtGrpAck), PBUF_RAM);
	if (txbuf != NULL) {
		udp_connect(g_SOMEIPSD_PCB, IP_ADDR_BROADCAST, PN_SERVICE_1);
		pbuf_take(txbuf, MSG_SubEvtGrpAck, sizeof(MSG_SubEvtGrpAck));

        ip_addr_t destination_ip;
        IP4_ADDR(&destination_ip, ip_a, ip_b, ip_c, ip_d);
		u16_t destination_port = PN_SOMEIPSD;

		err = udp_sendto(g_SOMEIPSD_PCB, txbuf, &destination_ip, destination_port);
		if (err == ERR_OK) {
			my_printf("Send SOMEIP Test Message !! \n");
		} else {
			my_printf("udp_sendto fail!!\n");
		}
		udp_disconnect(g_SOMEIPSD_PCB);

		pbuf_free(txbuf);
	} else {
		my_printf(
				"Failed to allocate memory for UDP packet buffer.\n");
	}
}



/*void SOMEIPSD_Recv_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16 port)
{
	volatile uint16 ServiceID = 0;
	volatile uint16 MethodID = 0;
	volatile uint8 SD_Type = 0;

	if (p != NULL)
	{
		ServiceID = *(uint16*)(p->payload);
		MethodID  = (*(((uint8*)p->payload) + 2) << 8) +
                    *(((uint8*)p->payload) + 3);

		unsigned char a = (unsigned char)(addr->addr);
		unsigned char b = (unsigned char)(addr->addr >> 8);
		unsigned char c = (unsigned char)(addr->addr >> 16);
        unsigned char d = (unsigned char)(addr->addr >> 24);

		 Received SOME/IP-SD
		if (ServiceID == 0xFFFFU && MethodID == 0x8100U)
		{
		    SD_Type = *(((uint8*)p->payload) + 24);
			if (SD_Type == 0x00) {
				SOMEIPSD_SendOfferService(a, b, c, d);
			} else if (SD_Type == 0x06) {
				SOMEIPSD_SendSubEvtGrpAck(a, b, c, d);
			}
		}
		pbuf_free(p);
	}
}*/

//added
static void send_service_list_unicast(const ip_addr_t *dst_ip, uint16_t dst_port) {
    const uint8_t ver = 1;
    const uint8_t cnt = (uint8_t)(sizeof(g_services)/sizeof(g_services[0]));

    // 총 길이: 헤더(8) + 각 엔트리(12 + 1 + name)
    uint16_t total = 8;
    for (uint8_t i=0; i<cnt; i++) {
        total += 12 + 1 + (uint8_t)strlen(g_services[i].name);
    }

    struct pbuf *pb = pbuf_alloc(PBUF_TRANSPORT, total, PBUF_RAM);
    if (!pb) return;

    uint8_t *w = (uint8_t*)pb->payload;
    // PSDD 헤더
    w[0]='P'; w[1]='S'; w[2]='D'; w[3]='D';
    w[4]=ver; w[5]=cnt; w[6]=0; w[7]=0; w += 8;

    // 엔트리들
    for (uint8_t i=0; i<cnt; i++) {
        const svc_desc_t *s = &g_services[i];
        be16(w+0,  s->svc);
        be16(w+2,  s->inst);
        w[4] = s->major;
        be32(w+5,  s->minor);
        w[9] = s->proto;
        be16(w+10, s->port);
        w += 12;
        uint8_t nl = (uint8_t)strlen(s->name);
        *w++ = nl;
        memcpy(w, s->name, nl);
        w += nl;
    }

    udp_sendto(g_SOMEIPSD_PCB, pb, dst_ip, dst_port);
    pbuf_free(pb);
}

void SOMEIPSD_Recv_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                            const ip_addr_t *addr, uint16 port)
{
    if (!p) return;

    uint8 *b = (uint8*)p->payload;

    // SOME/IP-SD 최소 길이 확인 (SD 타입 byte: 오프셋 24)
    if (p->len >= 25) {
        uint16_t ServiceID = (uint16_t)((b[0] << 8) | b[1]);
        uint16_t MethodID  = (uint16_t)((b[2] << 8) | b[3]);
        uint8_t  SD_Type   = b[24]; // 0x00 = FindService

        if (ServiceID == 0xFFFF && MethodID == 0x8100 && SD_Type == 0x00) {
            // 라즈베리파이 SD 제공
            send_service_list_unicast(addr, port);
            pbuf_free(p);
            return;
        }
    }
    pbuf_free(p);
}


//사용본
/*
void SOMEIPSD_Recv_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                            const ip_addr_t *addr, uint16 port)
{
    if (p == NULL) return;

    // Debug (src IP/port, dst port, 길이, 앞 8바이트)
    uint8 *b = (uint8*)p->payload;
    my_printf("[SD RX] from %u.%u.%u.%u:%u -> dst_port=%u len=%d  head=%02X %02X %02X %02X %02X %02X %02X %02X",
              (unsigned)(addr->addr & 0xFF),
              (unsigned)((addr->addr >> 8) & 0xFF),
              (unsigned)((addr->addr >> 16) & 0xFF),
              (unsigned)((addr->addr >> 24) & 0xFF),
              (unsigned)port,
              (unsigned)PN_SOMEIPSD,
              (int)p->len,
              (int)b[0], (int)b[1], (int)b[2], (int)b[3],
              (int)(p->len > 4 ? b[4] : 0),
              (int)(p->len > 5 ? b[5] : 0),
              (int)(p->len > 6 ? b[6] : 0),
              (int)(p->len > 7 ? b[7] : 0));

    // Length Guard
    if (p->len < 25) { // SD 타입(오프셋 24)까지 최소 25바이트 필요
        my_printf("[SD RX] too short (%d), drop", (int)p->len);
        pbuf_free(p);
        return;
    }

    // NW byte order Parsing
    uint16 ServiceID = (uint16)((b[0] << 8) | b[1]);
    uint16 MethodID  = (uint16)((b[2] << 8) | b[3]);
    uint8  SD_Type   = b[24];  // 0x00: FindService, 0x06: SubscribeEventgroup, 등

    // Debug 확인용
    my_printf("[SD RX] SID=0x%04X MID=0x%04X SD_Type=0x%02X", ServiceID, MethodID, SD_Type);

    // SOME/IP-SD 프레임 처리
    if (ServiceID == 0xFFFFU && MethodID == 0x8100U) {
         //송신자 IP 4바이트 분해 (기존 코드 유지)
        unsigned char a = (unsigned char)(addr->addr);
        unsigned char b1 = (unsigned char)(addr->addr >> 8);
        unsigned char c  = (unsigned char)(addr->addr >> 16);
        unsigned char d  = (unsigned char)(addr->addr >> 24);

        if (SD_Type == 0x00) {
            my_printf("[SD RX] FindService -> Send OfferService");
            SOMEIPSD_SendOfferService(a, b1, c, d);
        } else if (SD_Type == 0x06) {
            my_printf("[SD RX] SubscribeEventgroup -> Send ACK");
            SOMEIPSD_SendSubEvtGrpAck(a, b1, c, d);
        } else {
            my_printf("[SD RX] SD_Type 0x%02X not handled", SD_Type);
        }
    } else {
        my_printf("[SD RX] Not SD (SID/MID mismatch) -> ignore");
    }

    pbuf_free(p);
}
*/


/*void SOMEIP_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16 port)
{
	uint16 ServiceID = 0;
	uint16 MethodID = 0;
	uint8 MessageType = 0;

	if (p != NULL)
	{
		uint8 rxBuf[p->len];
		memcpy(rxBuf, p->payload, p->len);
		ServiceID = (rxBuf[0] << 8) + rxBuf[1];
		MethodID = (rxBuf[2] << 8) + rxBuf[3];

		if (ServiceID != 0x0100U)
		{
			my_printf("Requested Unknown Service ID\n");
		}
		else
		{
			 Message Type: Request
			MessageType = rxBuf[14];
			if (MessageType == 0x00)
			{
				 Check Service ID & Method ID
				if (ServiceID == 0x0100U)
				{
					if (MethodID == 0x0101U || MethodID == 0x0102U)
					{
						if (MethodID == 0x0101U) { SrvWelcomeLightInteriorControl(rxBuf[16]); }
						else { SrvWelcomeLightExteriorControl(rxBuf[16]); }

						 Send Response Message
						err_t err;
						rxBuf[14] = 0x80;
						struct pbuf *txbuf = pbuf_alloc(PBUF_TRANSPORT, sizeof(rxBuf), PBUF_RAM);
						if (txbuf != NULL) {
							udp_connect(upcb, addr, port);
							pbuf_take(txbuf, rxBuf, sizeof(rxBuf));
					        ip_addr_t destination_ip;
					        unsigned char a = (unsigned char)(addr->addr);
					        unsigned char b = (unsigned char)(addr->addr >> 8);
					        unsigned char c = (unsigned char)(addr->addr >> 16);
					        unsigned char d = (unsigned char)(addr->addr >> 24);

					        IP4_ADDR(&destination_ip, a, b, c, d);
							u16_t destination_port = PN_SERVICE_1;
							err = udp_sendto(upcb, txbuf, &destination_ip, destination_port);
							if (err == ERR_OK) {
								my_printf("Send SOME/IP Service Response!! \n");
							} else {
								my_printf("Send SOME/IP Service Response Failed!! \n");
							}
							udp_disconnect(upcb);
							pbuf_free(txbuf);
						} else {
							my_printf("Failed to allocate memory for UDP packet buffer.\n");
						}
					}
				}
			}
		}
		pbuf_free(p);
	}
}*/

void SOMEIP_Callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, uint16 port)
{
    if (p == NULL) return;

    if (p->len < 17) { // SOME/IP header(16) + 1 byte payload 최소 guard
        pbuf_free(p);
        return;
    }

    uint8 rxBuf[p->len];
    memcpy(rxBuf, p->payload, p->len);

    uint16 ServiceID = (rxBuf[0] << 8) | rxBuf[1];
    uint16 MethodID  = (rxBuf[2] << 8) | rxBuf[3];
    uint8  MessageType = rxBuf[14];

    if (ServiceID != 0x0100U) {
        my_printf("Requested Unknown Service ID\n");
        pbuf_free(p);
        return;
    }

    if (MessageType == 0x00) { // Request
        if (MethodID == 0x0101U || MethodID == 0x0102U) {
            uint8 payload = (p->len > 16) ? rxBuf[16] : 0x00;
            if (MethodID == 0x0101U) { SrvWelcomeLightInteriorControl(payload); }
            else                      { SrvWelcomeLightExteriorControl(payload); }

            // Response : MessageType만 0x80으로 바꿔 회신
            rxBuf[14] = 0x80; // RESPONSE

            struct pbuf *txbuf = pbuf_alloc(PBUF_TRANSPORT, sizeof(rxBuf), PBUF_RAM);
            if (txbuf != NULL) {
                pbuf_take(txbuf, rxBuf, sizeof(rxBuf));

                // ★요청을 보낸 그 소스로 (addr, port) 그대로 회신
                err_t err = udp_sendto(upcb, txbuf, addr, port);
                if (err == ERR_OK) my_printf("Send SOME/IP Service Response!! \n");
                else               my_printf("Send SOME/IP Service Response Failed!! \n");

                pbuf_free(txbuf);
            } else {
                my_printf("Failed to allocate memory for UDP packet buffer.\n");
            }
        }
    }

    pbuf_free(p);
}


#endif /* LWIP_UDP */
