// daemon/main.cpp
#include <systemd/sd-bus.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

static const char* GSUS_NAME = "org.gsus";
static const char* GSUS_OBJECT = "/org/gsus/Manager";
static const char* GSUS_IFACE = "org.gsus.Manager";

struct State {
    std::vector<std::string> items;
    const char* version = "0.1";
};

static int method_Echo(sd_bus_message *m, void *userdata, sd_bus_error *ret) {
    const char *in;
    int r = sd_bus_message_read(m, "s", &in);
    if (r < 0) return r;
    // reply with the same string
    return sd_bus_reply_method_return(m, "s", in);
}

static int method_GetVersion(sd_bus_message *m, void *userdata, sd_bus_error *ret) {
    State *s = (State*)userdata;
    return sd_bus_reply_method_return(m, "s", s->version);
}

static int method_List(sd_bus_message *m, void *userdata, sd_bus_error *ret) {
    State *s = (State*)userdata;
    sd_bus_message *reply = nullptr;
    int r = sd_bus_message_new_method_return(m, &reply);
    if (r < 0) return r;

    r = sd_bus_message_open_container(reply, 'a', "s"); // array of strings
    if (r < 0) return r;

    for (auto &it : s->items) {
        r = sd_bus_message_append(reply, "s", it.c_str());
        if (r < 0) {
            sd_bus_message_unref(reply);
            return r;
        }
    }

    r = sd_bus_message_close_container(reply);
    if (r < 0) {
        sd_bus_message_unref(reply);
        return r;
    }

    r = sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);
    return r;
}

static int method_Add(sd_bus_message *m, void *userdata, sd_bus_error *ret) {
    const char *item;
    int r = sd_bus_message_read(m, "s", &item);
    if (r < 0) return r;
    State *s = (State*)userdata;
    s->items.emplace_back(item);

    // emit a Changed signal with the added item
    sd_bus_emit_signal(sd_bus_message_get_bus(m),
                       sd_bus_message_get_destination(m), /* destination: NULL to broadcast */
                       GSUS_OBJECT,
                       GSUS_IFACE,
                       "Changed",
                       "s",
                       item);

    return sd_bus_reply_method_return(m, "b", 1); // success
}

static const sd_bus_vtable gsus_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Echo", "s", "s", method_Echo, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetVersion", "", "s", method_GetVersion, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("List", "", "as", method_List, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Add", "s", "b", method_Add, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL("Changed", "s", 0),
    SD_BUS_VTABLE_END
};

int main(int argc, char **argv) {
    int r;
    sd_bus *bus = nullptr;
    State state;
    state.items = { "task-a", "task-b" };

    bool use_system = false;
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i], "--system") == 0) use_system = true;
    }

    if (use_system) {
        r = sd_bus_open_system(&bus);
        if (r < 0) {
            fprintf(stderr, "Failed to open system bus: %s\n", strerror(-r));
            return 1;
        }
    } else {
        r = sd_bus_open_user(&bus); // session/user bus
        if (r < 0) {
            fprintf(stderr, "Failed to open user/session bus: %s\n", strerror(-r));
            return 1;
        }
    }

    // register object
    r = sd_bus_add_object_vtable(bus,
                                 nullptr,
                                 GSUS_OBJECT,
                                 GSUS_IFACE,
                                 gsus_vtable,
                                 &state);
    if (r < 0) {
        fprintf(stderr, "Failed to add object vtable: %s\n", strerror(-r));
        sd_bus_unref(bus);
        return 1;
    }

    // request a name
    r = sd_bus_request_name(bus, GSUS_NAME, 0);
    if (r < 0) {
        fprintf(stderr, "Failed to request name '%s': %s\n", GSUS_NAME, strerror(-r));
        sd_bus_unref(bus);
        return 1;
    }

    puts("gsus daemon running (ctrl-c to exit).");
    // main loop
    for (;;) {
        r = sd_bus_process(bus, nullptr);
        if (r < 0) {
            fprintf(stderr, "sd_bus_process error: %s\n", strerror(-r));
            break;
        }
        if (r > 0) continue; // we processed a message, loop again

        r = sd_bus_wait(bus, (uint64_t) -1);
        if (r < 0) {
            fprintf(stderr, "sd_bus_wait error: %s\n", strerror(-r));
            break;
        }
    }

    sd_bus_unref(bus);
    return 0;
}
