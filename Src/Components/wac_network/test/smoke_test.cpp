/*
** hallamp — wac_network Milestone 2 smoke test
**
** Proves the ported network stack *runs*, not just builds: drives the JNL
** WAC_Network_HTTPGet object directly through a live DNS resolve, TCP connect,
** and full HTTP + HTTPS GET. This exercises every runtime unknown surfaced by
** the platform-shim review (DNS worker thread, non-blocking sockets, OpenSSL
** RNG seeding after dropping CryptGenRandom, SIGPIPE on writes).
**
** Exit code 0 = all transactions succeeded; non-zero = at least one failed.
** Requires outbound network access; treat failures as informative, not as a
** build break, when running offline.
*/

#include <cstdio>
#include <cstring>
#include <ctime>
#include <unistd.h> // usleep

#include "wac_network_http_receiver.h"

using wa::Components::WAC_Network_HTTPGet;

static int fetch(const char *url) {
  printf("\n=== GET %s ===\n", url);

  WAC_Network_HTTPGet http; // API_DNS_AUTODNS: connection owns its own resolver
  http.addheader("User-Agent:hallamp-smoketest/0.1");
  http.addheader("Accept:*/*");
  http.connect(url); // ver defaults to HTTP/1.0 -> close-delimited body

  const time_t deadline = time(nullptr) + 20; // generous 20s ceiling
  size_t total_body = 0;
  int replycode = 0;
  char buf[4096];

  for (;;) {
    int r = http.run();
    if (r == -1) {
      printf("  [FAIL] error: %s\n", http.geterrorstr());
      return 1;
    }

    if (!replycode) {
      replycode = http.getreplycode();
      if (replycode)
        printf("  reply: \"%s\" (code %d)\n", http.getreply(), replycode);
    }

    // Drain whatever body bytes are buffered this tick.
    size_t avail;
    while ((avail = http.bytes_available()) > 0) {
      size_t want = avail < sizeof(buf) ? avail : sizeof(buf);
      size_t got = http.get_bytes(buf, want);
      if (!got)
        break;
      total_body += got;
    }

    if (r == 1) // connection closed cleanly -> response complete
      break;

    if (time(nullptr) > deadline) {
      printf("  [FAIL] timeout after 20s (status=%d)\n", http.get_status());
      return 1;
    }

    usleep(10 * 1000); // 10ms — yield instead of busy-spinning the socket
  }

  printf("  reply code: %d, body bytes: %zu\n", replycode, total_body);
  if (replycode >= 200 && replycode < 400 && total_body > 0) {
    printf("  [PASS]\n");
    return 0;
  }
  printf("  [FAIL] replycode=%d body=%zu\n", replycode, total_body);
  return 1;
}

int main() {
  int fails = 0;
  fails += fetch("http://example.com/");  // plain TCP path
  fails += fetch("https://example.com/"); // OpenSSL path (USE_SSL)

  if (fails == 0) {
    printf("\nALL SMOKE TESTS PASSED\n");
    return 0;
  }
  printf("\n%d SMOKE TEST(S) FAILED\n", fails);
  return 1;
}
