#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>

#include "err.h"

#define BSIZE         256
#define TTL_VALUE     4
#define REPEAT_COUNT  1
#define SLEEP_TIME    5

#define PORT 10001

int main(int argc, char *argv[]) {
  /* argumenty wywoĹania programu */
  char *remote_dotted_address;
  in_port_t remote_port;

  /* zmienne i struktury opisujÄce gniazda */
  int sock, optval, count;
//  struct sockaddr_in local_address;
  struct sockaddr_in remote_address, local_address;
  unsigned int remote_len;

  /* zmienne obsĹugujÄce komunikacjÄ */
  char buffer[BSIZE];
  size_t length;
  int i;

  /* parsowanie argumentĂłw programu */
  if (argc != 4)
    fatal("Usage: %s remote_address remote_port\n", argv[0]);
  remote_dotted_address = argv[1];
  remote_port = (in_port_t) atoi(argv[2]);
  count = atoi(argv[3]);
  /* otworzenie gniazda */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    syserr("socket");

  /* uaktywnienie rozgĹaszania (ang. broadcast) */
  optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval) < 0)
    syserr("setsockopt broadcast");

  /* ustawienie TTL dla datagramĂłw rozsyĹanych do grupy */
  optval = TTL_VALUE;
  if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval) < 0)
    syserr("setsockopt multicast ttl");

  /* zablokowanie rozsyĹania grupowego do siebie */
  /*
  optval = 0;
  if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, (void*)&optval, sizeof optval) < 0)
      syserr("setsockopt loop");
  */

  /* podpiÄcie siÄ pod lokalny adres i port */
  local_address.sin_family = AF_INET;
  local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  local_address.sin_port = htons(0);
  if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0)
    syserr("bind");

  /* ustawienie adresu i portu odbiorcy */
  remote_address.sin_family = AF_INET;
  remote_address.sin_port = htons(remote_port);
  if (inet_aton(remote_dotted_address, &remote_address.sin_addr) == 0)
    syserr("inet_aton");

  /* ustawienie timeoutu */
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval));

  /* radosne rozgĹaszanie czasu */
  ssize_t rcv_len;
  for (i = 0; i < REPEAT_COUNT; ++i) {
    printf("Sending request to %d server\n", count);
    bzero(buffer, BSIZE);
    strncpy(buffer, "GET_TIME", BSIZE);
    length = strnlen(buffer, BSIZE);
    if (sendto(sock, buffer, length, 0, (struct sockaddr *) &remote_address, sizeof(remote_address)) != length)
      syserr("write");

    for (int i = 0; i < count; ++i) {
      printf("Waiting for response...\n");
      rcv_len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &remote_address, &remote_len);
      if (rcv_len < 0) {
        if (i == 2) {
          printf("Closing.\n");
          close(sock);
          exit(1);
        }
        printf("Didn't get any response. Repeating request.\n");
      } else {
        printf("Time: %.*s\n", (int) rcv_len, buffer);
      }
    }
  }

  /* koniec */
  close(sock);
  exit(EXIT_SUCCESS);
}