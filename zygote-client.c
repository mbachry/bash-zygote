#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <varlink.h>

static long spawn_callback(VarlinkConnection *conn, const char *error,
                           VarlinkObject *call_args, uint64_t flags,
                           void *userdata) {
  bool *done = userdata;
  *done = true;
  return 0;
}

static void fallback(void) {
  if (execl("/bin/bash", "bash", "--login", NULL) < 0) {
    fprintf(stderr, "execl: /bin/bash: %m\n");
    exit(1);
  }
}

int main(void) {
  int res;

  pid_t pid = getpid();

  uid_t uid = getuid();
  char socket_addr[4096];
  snprintf(socket_addr, sizeof(socket_addr), "unix:@bash-zygote-%u.socket",
           uid);

  char tty_name[256];
  if (ttyname_r(STDIN_FILENO, tty_name, sizeof(tty_name)) < 0) {
    fprintf(stderr, "ttyname: %m\n");
    return 1;
  }

  char current_dir[4096];
  if (getcwd(current_dir, sizeof(current_dir)) < 0) {
    fprintf(stderr, "getcwd: %m\n");
    strncpy(current_dir, "/", sizeof(current_dir));
  }

  VarlinkObject *call_args;
  assert(varlink_object_new(&call_args) == 0);
  assert(varlink_object_set_int(call_args, "pid", pid) == 0);
  assert(varlink_object_set_string(call_args, "tty_name", tty_name) == 0);
  assert(varlink_object_set_string(call_args, "current_dir", current_dir) == 0);

  VarlinkConnection *conn;
  res = varlink_connection_new(&conn, socket_addr);
  if (res < 0) {
    fprintf(stderr, "varlink_connection_new: %s (%m)\n",
            varlink_error_string(-res));
    fallback();
  }

  bool done = false;
  res = varlink_connection_call(conn, "org.bash.zygote.Spawn", call_args, 0,
                                spawn_callback, &done);
  if (res < 0) {
    fprintf(stderr, "varlink_connection_call: %s (%m)\n",
            varlink_error_string(-res));
    fallback();
  }

  int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  assert(epoll_fd > 0);

  struct epoll_event event = {.events = EPOLLIN};
  assert(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, varlink_connection_get_fd(conn),
                   &event) == 0);

  while (true) {
    struct epoll_event events[1];
    res = epoll_wait(epoll_fd, events, 1, 1000);
    if (res == 0) {
      fprintf(stderr, "timed out waiting for bash zygote\n");
      fallback();
    }
    if (res < 0) {
      fprintf(stderr, "epoll_wait: %m\n");
      fallback();
    }

    res = varlink_connection_process_events(conn, events[0].events);
    if (res < 0) {
      fprintf(stderr, "varlink_connection_process_events: %s (%m)\n",
              varlink_error_string(-res));
      fallback();
    }

    if (done)
      break;
  }

  close(epoll_fd);
  varlink_object_unref(call_args);
  varlink_connection_free(conn);

  return 0;
}
