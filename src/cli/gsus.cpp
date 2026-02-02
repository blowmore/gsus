#include <systemd/sd-bus.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

static const char* GSUS_NAME = "org.gsus";
static const char* GSUS_OBJECT = "/org/gsus/Manager";
static const char* GSUS_IFACE = "org.gsus.Manager";

static int call_echo(sd_bus* bus, const char* text)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "Echo", &err, &reply, "s", text);
    if (r < 0)
    {
        fprintf(stderr, "Echo call failed: %s\n", err.message ? err.message : strerror(-r));
        sd_bus_error_free(&err);
        return 1;
    }

    const char* out;
    sd_bus_message_read(reply, "s", &out);
    printf("%s\n", out);
    sd_bus_message_unref(reply);

    return 0;
}

static int call_version(sd_bus* bus)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "GetVersion", &err, &reply, nullptr);
    if (r < 0)
    {
        fprintf(stderr, "GetVersion failed: %s\n", err.message ? err.message : strerror(-r));
        sd_bus_error_free(&err);
        return 1;
    }
    const char* v;
    sd_bus_message_read(reply, "s", &v);
    printf("version: %s\n", v);
    sd_bus_message_unref(reply);
    return 0;
}

static int call_list(sd_bus* bus)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "List", &err, &reply, nullptr);
    if (r < 0)
    {
        fprintf(stderr, "List failed: %s\n", err.message ? err.message : strerror(-r));
        sd_bus_error_free(&err);
        return 1;
    }

    // read array of strings
    sd_bus_message_enter_container(reply, 'a', "s");
    const char* s;
    while (sd_bus_message_read(reply, "s", &s) > 0)
    {
        puts(s);
    }
    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);
    return 0;
}

static int call_add(sd_bus* bus, const char* item)
{
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "Add", &err, &reply, "s", item);
    if (r < 0)
    {
        fprintf(stderr, "Add failed: %s\n", err.message ? err.message : strerror(-r));
        sd_bus_error_free(&err);
        return 1;
    }
    int success = 0;
    sd_bus_message_read(reply, "b", &success);
    sd_bus_message_unref(reply);
    printf("added => %s\n", success ? "ok" : "failed");
    return success ? 0 : 1;
}

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
