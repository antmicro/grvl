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
          dbus_error_free(&error);
          return -1;
      }

      DBusMessage *message = dbus_message_new_method_call(
          FBTERM_BUS_NAME,
          FBTERM_OBJECT_PATH,
          FBTERM_INTERFACE,
          FBTERM_METHOD);

      if (!message) {
          dbus_connection_unref(connection);
          return -1;
      }

      DBusMessage *reply =
          dbus_connection_send_with_reply_and_block(
              connection, message, -1, &error);

      dbus_message_unref(message);

      if (!reply) {
          dbus_error_free(&error);
          dbus_connection_unref(connection);
          return -1;
      }

      int lease_fd = -1;

      DBusMessageIter iter;
      if (!dbus_message_iter_init(reply, &iter) ||
          dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UNIX_FD) {
          dbus_message_unref(reply);
          dbus_error_free(&error);
          dbus_connection_unref(connection);
          return -1;
      }

      dbus_message_iter_get_basic(&iter, &lease_fd);

      dbus_message_unref(reply);
      dbus_error_free(&error);
      dbus_connection_unref(connection);

      return lease_fd;
  }
}
