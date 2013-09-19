/*

	   Definitions for RTP packet manipulation routines

*/

struct rtcp_sdes_request_item {
    unsigned char r_item;
    char *r_text;
};

struct rtcp_sdes_request {
    int nitems; 		      /* Number of items requested */
    unsigned char ssrc[4];	      /* Source identifier */
    struct rtcp_sdes_request_item item[10]; /* Request items */
};

extern int rtp_make_sdes(char **, unsigned long, int);
extern int rtp_make_bye(unsigned char *, unsigned long, char *, int);
extern int parseSDES(unsigned char *, struct rtcp_sdes_request *);
extern void copySDESitem(char *, char *);
extern int isRTCPByepacket(unsigned char *, int);
extern int isRTCPSdespacket(unsigned char *, int);
