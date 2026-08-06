
#include <grvl/grvl.h>
#include <grvl/Misc.h>

#include <cstring>
#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <memory>
#include "grvl/platform/DrmLease.h"

namespace grvl {

    struct RandrOutput {
        xcb_window_t root;
        xcb_randr_output_t output;
        xcb_randr_crtc_t crtc;

        c_unique_ptr<xcb_randr_get_output_info_reply_t> info;
        uint32_t connector_id;
        std::string name;
        int score;

        const char* str() const {
            return name.c_str();
        }
    };

    static int GetUseCount(const std::unordered_map<xcb_randr_crtc_t, int>& counts, xcb_randr_crtc_t crtc) {
        auto it = counts.find(crtc);

        if (it == counts.end()) {
            return 0;
        }

        return it->second;
    }

    static xcb_randr_crtc_t PickCrtc(xcb_connection_t* connection, const std::unordered_map<xcb_randr_crtc_t, int>& counts, const xcb_randr_get_output_info_reply_t* info) {
        if (info->crtc != XCB_NONE) {
            return (GetUseCount(counts, info->crtc) == 1) ? info->crtc : XCB_NONE;
        }

        xcb_randr_crtc_t* crtcs = xcb_randr_get_output_info_crtcs(info);

        for (int i = 0; i < info->num_crtcs; i++) {
            if (GetUseCount(counts, crtcs[i]) == 0) return crtcs[i];
        }

        return XCB_NONE;
    }


    static int GetConnectorId(xcb_connection_t* connection, xcb_randr_output_t output, xcb_atom_t atom) {
        if (atom == XCB_ATOM_NONE) {
            return -1;
        }

        xcb_randr_get_output_property_reply_t* reply = xcb_randr_get_output_property_reply(
            connection,
            xcb_randr_get_output_property(connection, output, atom, XCB_ATOM_NONE, 0, 1, false, false),
            nullptr
        );

        if (!reply) {
            return -1;
        }

        int id = -1;

        if (reply->format == 32 && reply->num_items == 1 && xcb_randr_get_output_property_data_length(reply) >= 4) {
            memcpy(&id, xcb_randr_get_output_property_data(reply), sizeof(id));
        }

        free(reply);
        return id;
    }

    static xcb_atom_t GetAtom(xcb_connection_t* connection, const char* name) {
        xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(
            connection,
            xcb_intern_atom(connection, true, strlen(name), name),
            nullptr
        );

        if (!reply) {
            return XCB_ATOM_NONE;
        }

        xcb_atom_t atom = reply->atom;
        free(reply);
        return atom;
    }

    static std::vector<RandrOutput> GetOutputs(xcb_connection_t* connection, const std::function<int(const RandrOutput&)>& judge) {
        xcb_atom_t atom = GetAtom(connection, "CONNECTOR_ID");

        std::unordered_map<xcb_randr_crtc_t, int> counts;
        std::vector<RandrOutput> results;
        xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(connection));

        while (it.rem) {

            xcb_window_t root = it.data->root;

            auto* resources = xcb_randr_get_screen_resources_current_reply(
                connection,
                xcb_randr_get_screen_resources_current(connection, root),
                nullptr
            );

            if (!resources) {
                xcb_screen_next(&it);
                continue;
            }

            xcb_randr_output_t* outputs = xcb_randr_get_screen_resources_current_outputs(resources);

            for (int i = 0; i < resources->num_outputs; i ++) {

                xcb_randr_output_t output = outputs[i];

                auto* info = xcb_randr_get_output_info_reply(
                    connection,
                    xcb_randr_get_output_info(connection, output, resources->config_timestamp),
                    nullptr
                );

                if (!info) {
                    continue;
                }

                if (info->connection != XCB_RANDR_CONNECTION_CONNECTED) {
                    free(info);
                    continue;
                }

                int name_len = xcb_randr_get_output_info_name_length(info);
                const char* buffer = (const char *) xcb_randr_get_output_info_name(info);

                char name[name_len + 1];
                memcpy(name, buffer, name_len);
                name[name_len] = 0;

                RandrOutput result {};
                result.root = root;
                result.name = name;
                result.output = output;
                result.connector_id = GetConnectorId(connection, output, atom);
                result.info = c_unique_ptr<xcb_randr_get_output_info_reply_t> {info};
                result.score = judge(result);

                counts[info->crtc] ++;

                results.push_back(std::move(result));
            }

            // second pass to pick CRTCs
            for (RandrOutput& result : results) {
                result.crtc = PickCrtc(connection, counts, result.info.get());
            }

            free(resources);
            xcb_screen_next(&it);
        }

        std::sort(results.begin(), results.end(), [] (const RandrOutput& l, const RandrOutput& r) {
            return l.score > r.score;
        });

        return results;
    }

    // implementation

    int LeaseDriver(int driver_fd, uint32_t preferred_connector_id) {
        int default_screen = 0;
        xcb_connection_t* connection = xcb_connect(nullptr, &default_screen);

        if (!connection || xcb_connection_has_error(connection)) {
            Log(ERROR, "Can't connect to X11");
            return -1;
        }

        const xcb_query_extension_reply_t* extension = xcb_get_extension_data(connection, &xcb_randr_id);
        if (!extension || !extension->present) {
            Log(ERROR, "RandR extension not supported");
            return -1;
        }

        xcb_randr_query_version_reply_t* version = xcb_randr_query_version_reply(
            connection,
            xcb_randr_query_version(connection, 1, 6),
            nullptr
        );

        if (!version) {
            Log(ERROR, "Failed to query XRandR version");
            return -1;
        }

        if ((version->major_version < 1) || (version->major_version == 1 && version->minor_version < 6)) {
            Log(ERROR, "XRandR version >= 1.6 required, but not supported");
            free(version);
            return -1;
        }

        free(version);

        // Consider all outputs but first try the ones matching the given preferred_connector_id, by sorting them
        // in descending order by the score returned by the given scoring function
        std::vector<RandrOutput> outputs = GetOutputs(connection, [preferred = preferred_connector_id] (const RandrOutput& output) {
            return output.connector_id == preferred ? 100 : 0;
        });

        for (int i = 0; i < outputs.size(); i ++) {
            const RandrOutput& output = outputs.at(i);
            xcb_randr_lease_t id = xcb_generate_id(connection);

            xcb_randr_create_lease_reply_t* reply = xcb_randr_create_lease_reply(
                    connection,
                    xcb_randr_create_lease(connection, output.root, id, 1, 1, &output.crtc, &output.output),
                    nullptr
            );

            if (!reply) {
                Log(ERROR, "Failed to lease output #%d '%s' (DRM connector: %d): XRandR failed to create a lease", i, output.str(), output.connector_id);
                continue;
            }

            int lease_fd = -1;
            int* fds = xcb_randr_create_lease_reply_fds(connection, reply);

            if (fds) {
                if (reply->nfd >= 1) lease_fd = fds[0];
                for (int i = 1; i < reply->nfd; i++) close(fds[i]);
            }
            free(reply);

            if (lease_fd < 0) {
                Log(ERROR, "Failed to lease output #%d '%s' (DRM connector: %d): No file descriptor returned", i, output.str(), output.connector_id);
                continue;
            }

            xcb_disconnect(connection);
            Log(INFO, "Leased XRandR output #%d '%s' (DRM connector: %d)", i, output.str(), output.connector_id);
            return lease_fd;
        }

        if (!outputs.empty()) {
            Log(ERROR, "XRandR outputs found, but none could be leased!");
        } else {
            Log(ERROR, "No leasable XRandR outputs found!");
        }

        return -1;
    }

}
