#include "commands.hpp"

#include <systemd/sd-bus.h>

#include <cstdio>
#include <cstring>
#include <string>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: gsus <echo|version|list|add> [...]\n");
        return 1;
    }

    sd_bus* bus = nullptr;
    int r = sd_bus_open_system(&bus);
    if (r < 0)
    {
        fprintf(stderr, "Failed to open system bus: %s\n", strerror(-r));
        return 1;
    }

    std::string cmd = argv[1];
    int rc = 0;
    if (cmd == "echo")
    {
        if (argc < 3)
        {
            fprintf(stderr, "echo requires a string\n");
            rc = 1;
        }
        else
        {
            rc = call_echo(bus, argv[2]);
        }
    }
    else if (cmd == "version")
    {
        rc = call_version(bus);
    }
    else if (cmd == "list")
    {
        rc = call_list(bus);
    }
    else if (cmd == "add")
    {
        if (argc < 3)
        {
            fprintf(stderr, "add requires an item name\n");
            rc = 1;
        }
        else
        {
            rc = call_add(bus, argv[2]);
        }
    }
    else
    {
        fprintf(stderr, "unknown command: %s\n", argv[1]);
        rc = 1;
    }

    sd_bus_unref(bus);
    return rc;
}
