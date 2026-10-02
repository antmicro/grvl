
#include <grvl/Misc.h>
#include <grvl/grvl.h>

#include "grvl/platform/XrandrDrmLease.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <xcb/randr.h>
#include <xcb/xcb.h>

namespace grvl {

    struct RandrOutput {
        xcb_window_t root;
        xcb_randr_output_t output;
        xcb_randr_crtc_t crtc;

        uint32_t connector_id;
        std::string name;

        const char* str() const
        {
            return name.c_str();
        }
    };

    static bool GetDrmDeviceNumber(int fd, dev_t& device)
    {
        struct stat info {};

        if(fstat(fd, &info) != 0) {
            Log(ERROR, "Failed to identify DRM device for file descriptor %d: %s", fd, strerror(errno));
            return false;
        }

        device = info.st_rdev;
        return true;
    }

    static int GetConnectorId(xcb_connection_t* connection, xcb_randr_output_t output, xcb_atom_t atom)
    {
        if(atom == XCB_ATOM_NONE) {
            return -1;
        }

        xcb_randr_get_output_property_reply_t* reply = xcb_randr_get_output_property_reply(
            connection,
            xcb_randr_get_output_property(connection, output, atom, XCB_ATOM_NONE, 0, 1, false, false),
            nullptr);

        if(!reply) {
            return -1;
        }

        int id = -1;

        if(reply->format == 32 && reply->num_items == 1 && xcb_randr_get_output_property_data_length(reply) >= 4) {
            memcpy(&id, xcb_randr_get_output_property_data(reply), sizeof(id));
        }

        free(reply);
        return id;
    }

    static xcb_atom_t GetAtom(xcb_connection_t* connection, const char* name)
    {
        xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(
            connection,
            xcb_intern_atom(connection, true, strlen(name), name),
            nullptr);

        if(!reply) {
            return XCB_ATOM_NONE;
        }

        xcb_atom_t atom = reply->atom;
        free(reply);
        return atom;
    }

    // implementation

    int AcquireXrandrLease(int driver_fd)
    {
        Log(INFO, "XRandR lease: begin acquisition for DRM fd=%d", driver_fd);

        dev_t driver_device;
        if(!GetDrmDeviceNumber(driver_fd, driver_device)) {
            return -1;
        }

        int default_screen = 0;
        xcb_connection_t* connection = xcb_connect(nullptr, &default_screen);

        if(!connection || xcb_connection_has_error(connection)) {
            Log(ERROR, "Can't connect to X11");
            return -1;
        }

        const xcb_query_extension_reply_t* extension = xcb_get_extension_data(connection, &xcb_randr_id);
        if(!extension || !extension->present) {
            Log(ERROR, "RandR extension not supported");
            xcb_disconnect(connection);
            return -1;
        }

        xcb_randr_query_version_reply_t* version = xcb_randr_query_version_reply(
            connection,
            xcb_randr_query_version(connection, 1, 6),
            nullptr);

        if(!version) {
            Log(ERROR, "Failed to query XRandR version");
            xcb_disconnect(connection);
            return -1;
        }

        if((version->major_version < 1) || (version->major_version == 1 && version->minor_version < 6)) {
            Log(ERROR, "XRandR version >= 1.6 required, but not supported");
            free(version);
            xcb_disconnect(connection);
            return -1;
        }

        free(version);

        const xcb_atom_t connector_id_atom = GetAtom(connection, "CONNECTOR_ID");
        xcb_screen_iterator_t screen = xcb_setup_roots_iterator(xcb_get_setup(connection));
        bool found_outputs = false;

        while(screen.rem) {
            const xcb_window_t root = screen.data->root;
            Log(INFO, "XRandR lease: inspect root=%u", static_cast<unsigned>(root));

            auto* resources = xcb_randr_get_screen_resources_current_reply(
                connection,
                xcb_randr_get_screen_resources_current(connection, root),
                nullptr);

            if(!resources) {
                Log(WARN, "XRandR lease: failed to get resources for root=%u", static_cast<unsigned>(root));
                xcb_screen_next(&screen);
                continue;
            }

            Log(INFO, "XRandR lease: root=%u resources outputs=%d CRTCs=%d",
                static_cast<unsigned>(root), resources->num_outputs, resources->num_crtcs);

            std::vector<RandrOutput> outputs;
            std::vector<xcb_randr_output_t> lease_outputs;
            std::vector<xcb_randr_crtc_t> lease_crtcs;

            xcb_randr_output_t* resource_outputs = xcb_randr_get_screen_resources_current_outputs(resources);
            for(int i = 0; i < resources->num_outputs; i++) {
                const xcb_randr_output_t output = resource_outputs[i];
                auto* info = xcb_randr_get_output_info_reply(
                    connection,
                    xcb_randr_get_output_info(connection, output, resources->config_timestamp),
                    nullptr);

                if(!info) {
                    Log(WARN, "XRandR lease: cannot query RandR output id=%u", static_cast<unsigned>(output));
                    continue;
                }

                found_outputs = true;
                const char* connection_name = "unknown";
                switch(info->connection) {
                    case XCB_RANDR_CONNECTION_CONNECTED:
                        connection_name = "connected";
                        break;
                    case XCB_RANDR_CONNECTION_DISCONNECTED:
                        connection_name = "disconnected";
                        break;
                    case XCB_RANDR_CONNECTION_UNKNOWN:
                        connection_name = "unknown";
                        break;
                }

                const int name_len = xcb_randr_get_output_info_name_length(info);
                const char* buffer = reinterpret_cast<const char*>(xcb_randr_get_output_info_name(info));

                RandrOutput result {};
                result.root = root;
                result.output = output;
                result.crtc = info->crtc;
                result.connector_id = GetConnectorId(connection, output, connector_id_atom);
                result.name.assign(buffer, name_len);

                Log(INFO,
                    "XRandR lease: output #%d name='%s' randr_id=%u DRM_connector=%d connection=%s(%u) current_CRTC=%u modes=%u",
                    i, result.str(), static_cast<unsigned>(result.output), static_cast<int>(result.connector_id),
                    connection_name, static_cast<unsigned>(info->connection), static_cast<unsigned>(result.crtc),
                    static_cast<unsigned>(info->num_modes));

                outputs.push_back(result);
                lease_outputs.push_back(output);

                if(info->crtc != XCB_NONE &&
                    std::find(lease_crtcs.begin(), lease_crtcs.end(), info->crtc) == lease_crtcs.end()) {
                    Log(INFO, "XRandR lease: add active CRTC=%u for output '%s'",
                        static_cast<unsigned>(info->crtc), result.str());
                    lease_crtcs.push_back(info->crtc);
                } else if(info->crtc == XCB_NONE) {
                    Log(INFO, "XRandR lease: output '%s' has no active CRTC; connector will still be leased",
                        result.str());
                } else {
                    Log(INFO, "XRandR lease: CRTC=%u for output '%s' already present in lease set",
                        static_cast<unsigned>(info->crtc), result.str());
                }

                free(info);
            }

            free(resources);

            if(lease_outputs.empty()) {
                xcb_screen_next(&screen);
                continue;
            }

            Log(INFO, "XRandR lease: collected existing outputs=%zu active_CRTCs=%zu on root=%u",
                lease_outputs.size(), lease_crtcs.size(), static_cast<unsigned>(root));

            if(lease_crtcs.empty()) {
                Log(WARN, "XRandR screen has outputs but no CRTCs");
                xcb_screen_next(&screen);
                continue;
            }

            xcb_randr_lease_t id = xcb_generate_id(connection);
            Log(INFO, "XRandR lease: CreateLease id=%u root=%u outputs=%zu CRTCs=%zu",
                static_cast<unsigned>(id), static_cast<unsigned>(root), lease_outputs.size(), lease_crtcs.size());

            for(size_t i = 0; i < outputs.size(); i++) {
                Log(INFO, "XRandR lease:   output[%zu] name='%s' randr_id=%u DRM_connector=%d CRTC=%u",
                    i, outputs[i].str(), static_cast<unsigned>(outputs[i].output),
                    static_cast<int>(outputs[i].connector_id), static_cast<unsigned>(outputs[i].crtc));
            }
            for(size_t i = 0; i < lease_crtcs.size(); i++) {
                Log(INFO, "XRandR lease:   CRTC[%zu]=%u", i, static_cast<unsigned>(lease_crtcs[i]));
            }

            xcb_generic_error_t* error = nullptr;
            xcb_randr_create_lease_reply_t* reply = xcb_randr_create_lease_reply(
                connection,
                xcb_randr_create_lease(
                    connection,
                    root,
                    id,
                    static_cast<uint16_t>(lease_crtcs.size()),
                    static_cast<uint16_t>(lease_outputs.size()),
                    lease_crtcs.data(),
                    lease_outputs.data()),
                &error);

            if(!reply) {
                if(error) {
                    Log(ERROR, "Failed to lease all XRandR outputs on screen: X11 error: code=%d major=%d minor=%d",
                        error->error_code,
                        error->major_code,
                        error->minor_code);
                    free(error);
                } else {
                    Log(ERROR, "Failed to lease all XRandR outputs on screen: XRandR failed without an X11 error");
                }

                xcb_screen_next(&screen);
                continue;
            }

            Log(INFO, "XRandR lease: CreateLease reply nfd=%u", static_cast<unsigned>(reply->nfd));

            int lease_fd = -1;
            int* fds = xcb_randr_create_lease_reply_fds(connection, reply);

            if(fds) {
                if(reply->nfd >= 1)
                    lease_fd = fds[0];
                for(int fd_index = 1; fd_index < reply->nfd; fd_index++)
                    close(fds[fd_index]);
            }
            free(reply);

            if(lease_fd < 0) {
                Log(ERROR, "Failed to lease all XRandR outputs on screen: no file descriptor returned");
                xcb_screen_next(&screen);
                continue;
            }

            dev_t lease_device;
            if(!GetDrmDeviceNumber(lease_fd, lease_device)) {
                close(lease_fd);
                xcb_screen_next(&screen);
                continue;
            }

            if(lease_device != driver_device) {
                Log(ERROR,
                    "XRandR lease: created lease fd=%d belongs to DRM device=%llu, requested DRM device=%llu; ignoring lease",
                    lease_fd,
                    static_cast<unsigned long long>(lease_device),
                    static_cast<unsigned long long>(driver_device));
                close(lease_fd);
                xcb_screen_next(&screen);
                continue;
            }

            xcb_disconnect(connection);
            Log(INFO, "XRandR lease: SUCCESS fd=%d root=%u outputs=%zu CRTCs=%zu",
                lease_fd, static_cast<unsigned>(root), lease_outputs.size(), lease_crtcs.size());
            for(size_t i = 0; i < outputs.size(); i++) {
                Log(INFO, "XRandR lease:   leased output #%zu '%s' randr_id=%u DRM_connector=%d CRTC=%u",
                    i, outputs[i].str(), static_cast<unsigned>(outputs[i].output),
                    static_cast<int>(outputs[i].connector_id), static_cast<unsigned>(outputs[i].crtc));
            }
            return lease_fd;
        }

        xcb_disconnect(connection);

        if(found_outputs) {
            Log(ERROR, "XRandR outputs found, but none could be leased for this DRM device!");
        } else {
            Log(ERROR, "No XRandR outputs found!");
        }

        return -1;
    }


}
