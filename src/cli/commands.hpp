#include <systemd/sd-bus.h>

static const char* GSUS_NAME = "org.gsus";
static const char* GSUS_OBJECT = "/org/gsus/Manager";
static const char* GSUS_IFACE = "org.gsus.Manager";

static int call_echo(sd_bus* bus, const char* text)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;

    int status = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "Echo", &error, &reply, "s", text);
    if (status < 0)
    {
        fprintf(stderr, "Echo call failed: %s\n", error.message ? error.message : strerror(-status));
        sd_bus_error_free(&error);
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
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;

    int status = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "GetVersion", &error, &reply, nullptr);
    if (status < 0)
    {
        fprintf(stderr, "GetVersion failed: %s\n", error.message ? error.message : strerror(-status));
        sd_bus_error_free(&error);
        return 1;
    }

    const char* out;
    sd_bus_message_read(reply, "s", &out);
    printf("version: %s\n", out);
    sd_bus_message_unref(reply);
    return 0;
}

static int call_list(sd_bus* bus)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;

    int r = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "List", &error, &reply, nullptr);
    if (r < 0)
    {
        fprintf(stderr, "List failed: %s\n", error.message ? error.message : strerror(-r));
        sd_bus_error_free(&error);
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
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;

    int status = sd_bus_call_method(bus, GSUS_NAME, GSUS_OBJECT, GSUS_IFACE, "Add", &error, &reply, "s", item);
    if (status < 0)
    {
        fprintf(stderr, "Add failed: %s\n", error.message ? error.message : strerror(-status));
        sd_bus_error_free(&error);
        return 1;
    }
    int success = 0;
    sd_bus_message_read(reply, "b", &success);
    sd_bus_message_unref(reply);
    printf("added => %s\n", success ? "ok" : "failed");
    return success ? 0 : 1;
}