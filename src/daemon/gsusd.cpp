#include "commands.hpp"
#include "state.hpp"

#include <systemd/sd-bus.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

static const sd_bus_vtable gsus_vtable[] = {SD_BUS_VTABLE_START(0),
                                            SD_BUS_METHOD("Echo", "s", "s", method_Echo, SD_BUS_VTABLE_UNPRIVILEGED),
                                            SD_BUS_METHOD("GetVersion", "", "s", method_GetVersion, SD_BUS_VTABLE_UNPRIVILEGED),
                                            SD_BUS_METHOD("List", "", "as", method_List, SD_BUS_VTABLE_UNPRIVILEGED),
                                            SD_BUS_METHOD("Add", "s", "b", method_Add, SD_BUS_VTABLE_UNPRIVILEGED),
                                            SD_BUS_SIGNAL("Changed", "s", 0),
                                            SD_BUS_VTABLE_END};

int main(int, char**)
{
    int r;
    sd_bus* bus = nullptr;
    State state;
    state.items = {"file-a", "file-b"};

    r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        fprintf(stderr, "Failed to open system bus: %s\n", strerror(-r));
        return 1;
    }

    // register object
    r = sd_bus_add_object_vtable(bus, nullptr, GSUS_OBJECT, GSUS_IFACE, gsus_vtable, &state);
    if (r < 0)
    {
        fprintf(stderr, "Failed to add object vtable: %s\n", strerror(-r));
        sd_bus_unref(bus);
        return 1;
    }

    // request a name
    r = sd_bus_request_name(bus, GSUS_NAME, 0);
    if (r < 0)
    {
        fprintf(stderr, "Failed to request name '%s': %s\n", GSUS_NAME, strerror(-r));
        sd_bus_unref(bus);
        return 1;
    }

    puts("gsus daemon running (ctrl-c to exit).");
    // main loop
    for (;;)
    {
        r = sd_bus_process(bus, nullptr);
        if (r < 0)
        {
            fprintf(stderr, "sd_bus_process error: %s\n", strerror(-r));
            break;
        }
        if (r > 0)
        {
            continue;  // we processed a message, loop again
        }

        r = sd_bus_wait(bus, (uint64_t)-1);
        if (r < 0)
        {
            fprintf(stderr, "sd_bus_wait error: %s\n", strerror(-r));
            break;
        }
    }

    sd_bus_unref(bus);
    return 0;
}
