#include "state.hpp"

#include <systemd/sd-bus.h>

static const char* GSUS_NAME = "org.gsus";
static const char* GSUS_OBJECT = "/org/gsus/Manager";
static const char* GSUS_IFACE = "org.gsus.Manager";

static int method_Echo(sd_bus_message* m, void*, sd_bus_error*)
{
    const char* in;
    int r = sd_bus_message_read(m, "s", &in);
    if (r < 0)
    {
        return r;
    }
    // reply with the same string
    return sd_bus_reply_method_return(m, "s", in);
}

static int method_GetVersion(sd_bus_message* m, void* userdata, sd_bus_error*)
{
    State* s = (State*)userdata;
    return sd_bus_reply_method_return(m, "s", s->version);
}

static int method_List(sd_bus_message* m, void* userdata, sd_bus_error*)
{
    State* s = (State*)userdata;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_message_new_method_return(m, &reply);
    if (r < 0)
    {
        return r;
    }

    r = sd_bus_message_open_container(reply, 'a', "s");  // array of strings
    if (r < 0)
    {
        return r;
    }

    for (auto& it : s->items)
    {
        r = sd_bus_message_append(reply, "s", it.c_str());
        if (r < 0)
        {
            sd_bus_message_unref(reply);
            return r;
        }
    }

    r = sd_bus_message_close_container(reply);
    if (r < 0)
    {
        sd_bus_message_unref(reply);
        return r;
    }

    r = sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);
    return r;
}

static int method_Add(sd_bus_message* m, void* userdata, sd_bus_error*)
{
    const char* item;
    int r = sd_bus_message_read(m, "s", &item);
    if (r < 0)
    {
        return r;
    }
    State* s = (State*)userdata;
    s->items.emplace_back(item);

    // emit a Changed signal with the added item
    sd_bus_emit_signal(sd_bus_message_get_bus(m), sd_bus_message_get_destination(m), /* destination: NULL to broadcast */
                       GSUS_OBJECT, GSUS_IFACE, "Changed", "s", item);

    return sd_bus_reply_method_return(m, "b", 1);  // success
}