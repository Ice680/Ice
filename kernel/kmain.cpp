#include <limine.h>
#include <stddef.h>
#include <stdint.h>

extern "C" {
static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

static void done(void) {
  for (;;) {
    asm("hlt");
  }
}

size_t strlen(const char* str) {
  size_t ret = 0;
  while (*str++) {
    ret++;
  }
  return ret;
}

void _start(void) {
  if (terminal_request.response == NULL ||
      terminal_request.response->terminal_count < 1) {
    done();
  }

  const char* hello_msg = "Hello World";

  struct limine_terminal* terminal = terminal_request.response->terminals[0];
  terminal_request.response->write(terminal, hello_msg, strlen(hello_msg));

  done();
}
}