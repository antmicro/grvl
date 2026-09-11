#include "grvl/platform/FbtermDrmLease.h"

#include <dbus/dbus.h>

static constexpr const char *FBTERM_BUS_NAME = "com.antmicro.fbterm";
static constexpr const char *FBTERM_OBJECT_PATH = "/com/antmicro/fbterm";
static constexpr const char *FBTERM_INTERFACE = "com.antmicro.fbterm.Display";
static constexpr const char *FBTERM_METHOD = "AcquireLease";

namespace grvl {
  int AcquireFbtermLease()
  {
      DBusError error;
      dbus_error_init(&error);

      DBusConnection *connection =
          dbus_bus_get(DBUS_BUS_SYSTEM, &error);

      if (!connection) {
          Log(ERROR,
                  "AcquireFbtermLease: dbus_bus_get failed: %s%s%s\n",
                  dbus_error_is_set(&error) ? error.name : "",
                  dbus_error_is_set(&error) ? ": " : "",
                  dbus_error_is_set(&error) ? error.message
                  : "unknown error");
          dbus_error_free(&error);
          return -1;
      }

      DBusMessage *message = dbus_message_new_method_call(
          FBTERM_BUS_NAME,
          FBTERM_OBJECT_PATH,
          FBTERM_INTERFACE,
          FBTERM_METHOD);

      if (!message) {
          Log(ERROR,
                  "AcquireFbtermLease: failed to allocate D-Bus method call\n");
          dbus_connection_unref(connection);
          return -1;
      }

      Log(INFO,
              "AcquireFbtermLease: calling %s.%s on %s%s\n",
              FBTERM_INTERFACE,
              FBTERM_METHOD,
              FBTERM_BUS_NAME,
              FBTERM_OBJECT_PATH);

      DBusMessage *reply =
          dbus_connection_send_with_reply_and_block(
              connection, message, -1, &error);

      dbus_message_unref(message);

      if (!reply) {
          Log(ERROR,
                  "AcquireFbtermLease: D-Bus call failed: %s%s%s\n",
                  dbus_error_is_set(&error) ? error.name : "",
                  dbus_error_is_set(&error) ? ": " : "",
                  dbus_error_is_set(&error) ? error.message
                  : "unknown error");
          dbus_error_free(&error);
          dbus_connection_unref(connection);
          return -1;
      }

      int lease_fd = -1;

      DBusMessageIter iter;
      if (!dbus_message_iter_init(reply, &iter) ||
          dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UNIX_FD) {
          Log(ERROR, "AcquireFbtermLease: reply contains no arguments\n");
          dbus_message_unref(reply);
          dbus_error_free(&error);
          dbus_connection_unref(connection);
          return -1;
      }

      dbus_message_iter_get_basic(&iter, &lease_fd);

      Log(INFO, "AcquireFbtermLease: received lease FD %d\n", lease_fd);

      dbus_message_unref(reply);
      dbus_error_free(&error);
      dbus_connection_unref(connection);

      return lease_fd;
  }
}
