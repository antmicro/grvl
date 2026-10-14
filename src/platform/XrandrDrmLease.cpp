
#include <grvl/Misc.h>
#include <grvl/grvl.h>

#include "grvl/platform/XrandrDrmLease.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <string>
#include <unordered_set>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <xcb/dpms.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

namespace grvl {

    struct RandrOutput {
        xcb_window_t root;
        xcb_randr_output_t output;
        xcb_randr_crtc_t crtc;

        std::string name;

        const char* str() const
        {
            return name.c_str();
        }
    };

    struct DrmDevice {
        xcb_randr_provider_t provider = XCB_NONE;
        std::vector<RandrOutput> outputs;
        std::vector<xcb_randr_output_t> lease_outputs;
        std::vector<xcb_randr_crtc_t> lease_crtcs;
        size_t matching_connectors = 0;
    };

    struct SavedDpmsState {
        bool valid = false;
        bool enabled = false;
        uint16_t power_level = XCB_DPMS_DPMS_MODE_ON;
    };

    static SavedDpmsState saved_dpms_state;

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

    static bool CheckDpmsRequest( xcb_connection_t *conn, xcb_void_cookie_t cookie, const char *action)
    {
        xcb_generic_error_t *error = xcb_request_check(conn, cookie);

        if (!error)
            return true;

        Log(ERROR,
                "%s: code=%u resource_id=0x%x sequence=%u major=%u minor=%u",
                action,
                error->error_code,
                error->resource_id,
                error->sequence,
                error->major_code,
                error->minor_code);

        free(error);
        return false;
    }

    static bool SaveDpmsState(xcb_connection_t *conn)
    {
        xcb_dpms_capable_cookie_t capable_cookie = xcb_dpms_capable(conn);
        xcb_dpms_capable_reply_t *capable = xcb_dpms_capable_reply(conn, capable_cookie, nullptr);

        if (!capable) {
            Log(ERROR, "Failed to query DPMS capability");
            return false;
        }

        bool is_capable = capable->capable;
        free(capable);

        if (!is_capable) {
            Log(ERROR, "X server does not support DPMS");
            return false;
        }

        xcb_dpms_info_cookie_t info_cookie = xcb_dpms_info(conn);
        xcb_dpms_info_reply_t *info = xcb_dpms_info_reply(conn, info_cookie, nullptr);

        if (!info) {
            Log(ERROR, "Failed to query current DPMS state");
            return false;
        }

        saved_dpms_state.enabled = info->state;
        saved_dpms_state.power_level = info->power_level;
        saved_dpms_state.valid = true;

        Log(INFO,
                "Saved DPMS state: enabled=%d power_level=0x%x",
                saved_dpms_state.enabled,
                saved_dpms_state.power_level);

        free(info);
        return true;
    }

    static bool RestoreDpmsState(xcb_connection_t *conn)
    {
        if (!saved_dpms_state.valid) {
            Log(ERROR, "No saved DPMS state to restore");
            return false;
        }

        if (saved_dpms_state.enabled) {
            if (!CheckDpmsRequest(
                        conn,
                        xcb_dpms_enable_checked(conn),
                        "Failed to enable DPMS during restoration"))
                return false;

            if (!CheckDpmsRequest(
                        conn,
                        xcb_dpms_force_level_checked(
                            conn, saved_dpms_state.power_level),
                        "Failed to restore DPMS power level"))
                return false;
        } else {
            if (!CheckDpmsRequest(
                        conn,
                        xcb_dpms_disable_checked(conn),
                        "Failed to restore disabled DPMS state"))
                return false;
        }

        xcb_flush(conn);

        Log(INFO,
                "Restored DPMS state: enabled=%d power_level=0x%x",
                saved_dpms_state.enabled,
                saved_dpms_state.power_level);

        saved_dpms_state.valid = false;
        return true;
    }

    static int SetDpms(xcb_connection_t *conn, uint16_t level)
    {
        if (!CheckDpmsRequest(
                    conn,
                    xcb_dpms_enable_checked(conn),
                    "Failed to enable DPMS"))
            return -1;

        if (!CheckDpmsRequest(
                    conn,
                    xcb_dpms_force_level_checked(conn, level),
                    "Failed to force DPMS power level"))
            return -1;

        xcb_dpms_info_cookie_t info_cookie = xcb_dpms_info(conn);
        xcb_dpms_info_reply_t *info =
            xcb_dpms_info_reply(conn, info_cookie, NULL);

        if (info) {
            Log(INFO, "DPMS: power_level=0x%x state=0x%x", info->power_level, info->state);
            free(info);
        }

        xcb_flush(conn);
        return 0;
    }

    // implementation
    static int CreateXrandrLease( xcb_connection_t* connection, xcb_window_t root, xcb_randr_lease_t lease, const DrmDevice& device, dev_t driver_device)
    {
        xcb_generic_error_t* error = nullptr;
        xcb_randr_create_lease_reply_t* reply = xcb_randr_create_lease_reply(
                connection,
                xcb_randr_create_lease(
                    connection,
                    root,
                    lease,
                    static_cast<uint16_t>(device.lease_crtcs.size()),
                    static_cast<uint16_t>(device.lease_outputs.size()),
                    device.lease_crtcs.data(),
                    device.lease_outputs.data()),
                &error);

        if(!reply) {
            if(error) {
                Log(ERROR,
                        "Failed to create XRandR lease: "
                        "code=%u resource_id=0x%x sequence=%u major=%u minor=%u",
                        error->error_code,
                        error->resource_id,
                        error->sequence,
                        error->major_code,
                        error->minor_code);
                free(error);
            } else {
                Log(ERROR, "Failed to create XRandR lease: no X11 error");
            }
            return -1;
        }

        Log(INFO, "XRandR lease: CreateLease reply nfd=%u",
                static_cast<unsigned>(reply->nfd));

        int lease_fd = -1;
        int* fds = xcb_randr_create_lease_reply_fds(connection, reply);

        if(fds) {
            if(reply->nfd >= 1)
                lease_fd = fds[0];
            for(int fd_index = 1; fd_index < reply->nfd; ++fd_index)
                close(fds[fd_index]);
        }
        free(reply);

        if(lease_fd < 0) {
            Log(ERROR, "Failed to create XRandR lease: no file descriptor returned");
            return -1;
        }

        dev_t lease_device;
        if(!GetDrmDeviceNumber(lease_fd, lease_device)) {
            close(lease_fd);
            return -1;
        }

        if(lease_device != driver_device) {
            Log(ERROR, "XRandR lease: created lease fd=%d belongs to DRM device=%llu, requested DRM device=%llu; ignoring lease",
                    lease_fd,
                    static_cast<unsigned long long>(lease_device),
                    static_cast<unsigned long long>(driver_device));
            close(lease_fd);
            return -1;
        }

        Log(INFO, "XRandR lease: SUCCESS fd=%d root=%u outputs=%zu CRTCs=%zu",
                lease_fd,
                static_cast<unsigned>(root),
                device.lease_outputs.size(),
                device.lease_crtcs.size());

        for(size_t i = 0; i < device.outputs.size(); ++i) {
            Log(INFO,
                    "XRandR lease:   leased output #%zu '%s' randr_id=%u CRTC=%u",
                    i,
                    device.outputs[i].str(),
                    static_cast<unsigned>(device.outputs[i].output),
                    static_cast<unsigned>(device.outputs[i].crtc));
        }

        return lease_fd;
    }

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
            if(connection)
                xcb_disconnect(connection);
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

        std::vector<DrmDevice> devices;

        const xcb_atom_t connector_id_atom =
            GetAtom(connection, "CONNECTOR_ID");

        std::unordered_set<uint32_t> driver_connector_ids;

        drmModeRes* resources = drmModeGetResources(driver_fd);
        if(!resources) {
            Log(ERROR, "XRandR lease: failed to get DRM resources for requested device");
            xcb_disconnect(connection);
            return -1;
        }

        for(int i = 0; i < resources->count_connectors; ++i) {
            driver_connector_ids.insert(resources->connectors[i]);
        }

        drmModeFreeResources(resources);

        /*
        * Enumerate XRandR providers. Each provider corresponds to one DRM
        * device and owns a set of outputs and CRTCs.
        */
        xcb_screen_iterator_t screen =
            xcb_setup_roots_iterator(xcb_get_setup(connection));

        while(screen.rem) {
            const xcb_window_t root = screen.data->root;
            Log(INFO, "XRandR lease: inspect root=%u", static_cast<unsigned>(root));

            auto* providers = xcb_randr_get_providers_reply(
                connection,
                xcb_randr_get_providers(connection, root),
                nullptr);

            if(!providers) {
                Log(WARN, "XRandR lease: failed to get providers for root=%u", static_cast<unsigned>(root));
                xcb_screen_next(&screen);
                continue;
            }

            Log(INFO,
                "XRandR lease: root=%u providers=%u",
                static_cast<unsigned>(root), static_cast<unsigned>(providers->num_providers));

            xcb_randr_provider_t* provider_ids =
                xcb_randr_get_providers_providers(providers);

            for(int i = 0; i < providers->num_providers; ++i) {
                const xcb_randr_provider_t provider = provider_ids[i];

                auto* info = xcb_randr_get_provider_info_reply(
                    connection,
                    xcb_randr_get_provider_info( connection, provider, XCB_CURRENT_TIME),
                    nullptr);

                if(!info) {
                    Log(WARN, "XRandR lease: cannot query provider id=%u", static_cast<unsigned>(provider));
                    continue;
                }

                Log(INFO,
                    "XRandR lease: provider=%u CRTCs=%u outputs=%u name='%.*s'",
                    static_cast<unsigned>(provider),
                    static_cast<unsigned>(info->num_crtcs),
                    static_cast<unsigned>(info->num_outputs),
                    xcb_randr_get_provider_info_name_length(info),
                    xcb_randr_get_provider_info_name(info));

                DrmDevice device {};
                device.provider = provider;

                std::unordered_set<uint32_t> matched_connector_ids;

                /*
                * Provider outputs are already grouped by the DRM device,
                * so no CONNECTOR_ID -> DRM-device mapping is necessary here.
                */
                xcb_randr_output_t* outputs =
                    xcb_randr_get_provider_info_outputs(info);

                for(int output_index = 0;
                    output_index < info->num_outputs;
                    ++output_index) {

                    const xcb_randr_output_t output =
                        outputs[output_index];

                    const int connector_id =
                        GetConnectorId(connection, output, connector_id_atom);

                    if(driver_connector_ids.find(connector_id) != driver_connector_ids.end()) {
                        matched_connector_ids.insert(connector_id);
                    }

                    auto* output_info = xcb_randr_get_output_info_reply(
                        connection,
                        xcb_randr_get_output_info(
                            connection,
                            output,
                            XCB_CURRENT_TIME),
                        nullptr);

                    if(!output_info) {
                        Log(WARN, "XRandR lease: cannot query output id=%u", static_cast<unsigned>(output));
                        continue;
                    }

                    const int name_len =
                        xcb_randr_get_output_info_name_length(output_info);

                    const char* name =
                        reinterpret_cast<const char*>(
                            xcb_randr_get_output_info_name(output_info));

                    RandrOutput output_result {};
                    output_result.root = root;
                    output_result.output = output;
                    output_result.crtc = output_info->crtc;
                    output_result.name.assign(name, name_len);

                    Log(INFO, "XRandR lease: provider=%u output #%d name='%s' randr_id=%u current_CRTC=%u modes=%u",
                        static_cast<unsigned>(provider),
                        output_index,
                        output_result.str(),
                        static_cast<unsigned>(output_result.output),
                        static_cast<unsigned>(output_result.crtc),
                        static_cast<unsigned>(output_info->num_modes));

                    device.outputs.push_back(output_result);
                    device.lease_outputs.push_back(output);

                    if(output_info->crtc != XCB_NONE &&
                    std::find(
                        device.lease_crtcs.begin(),
                        device.lease_crtcs.end(),
                        output_info->crtc) ==
                        device.lease_crtcs.end()) {

                        Log(INFO, "XRandR lease: add active CRTC=%u for output '%s'",
                            static_cast<unsigned>(output_info->crtc),
                            output_result.str());

                        device.lease_crtcs.push_back(output_info->crtc);
                    } else if(output_info->crtc == XCB_NONE) {
                        Log(INFO, "XRandR lease: output '%s' has no active CRTC; connector will still be leased",
                            output_result.str());
                    }

                    free(output_info);
                }

                device.matching_connectors = matched_connector_ids.size();

                free(info);

                if(device.lease_outputs.empty()) {
                    Log(INFO, "XRandR lease: provider=%u has no outputs; skipping",
                        static_cast<unsigned>(provider));
                    continue;
                }

                if(device.lease_crtcs.empty()) {
                    Log(WARN, "XRandR lease: provider=%u has outputs but no CRTCs; skipping",
                        static_cast<unsigned>(provider));
                    continue;
                }

                devices.push_back(std::move(device));
            }

            free(providers);
            xcb_screen_next(&screen);
        }

        int lease_fd = -1;

        if(!devices.empty()) {
            /*
             * Match the XRandR provider to the requested DRM device using
             * the CONNECTOR_ID property of its outputs.
             *
             * CONNECTOR_ID values are local to a DRM device and are not
             * guaranteed to uniquely identify a provider across devices.
             * This is therefore a best-effort match.
             */
            auto matching_device = devices.end();
            size_t max_matches = 0;
            bool ambiguous = false;

            for(auto it = devices.begin(); it != devices.end(); ++it) {
                if(it->matching_connectors > max_matches) {
                    max_matches = it->matching_connectors;
                    matching_device = it;
                    ambiguous = false;
                } else if(it->matching_connectors == max_matches && max_matches > 0) {
                    ambiguous = true;
                }
            }

            if(max_matches == 0 || ambiguous) {
                Log(ERROR, "XRandR lease: unable to uniquely identify provider for the requested DRM device");
                return -1;
            }

            auto& device = *matching_device;
            const xcb_window_t root = device.outputs.front().root;
            const xcb_randr_lease_t lease = xcb_generate_id(connection);

            Log(INFO, "XRandR lease: creating lease provider=%u root=%u outputs=%zu CRTCs=%zu",
                static_cast<unsigned>(device.provider),
                static_cast<unsigned>(root),
                device.lease_outputs.size(),
                device.lease_crtcs.size());

            for(size_t i = 0; i < device.outputs.size(); ++i) {
                Log(INFO, "XRandR lease:   output[%zu] name='%s' randr_id=%u CRTC=%u",
                    i,
                    device.outputs[i].str(),
                    static_cast<unsigned>(device.outputs[i].output),
                    static_cast<unsigned>(device.outputs[i].crtc));
            }

            for(size_t i = 0; i < device.lease_crtcs.size(); ++i) {
                Log(INFO, "XRandR lease:   CRTC[%zu]=%u",
                    i,
                    static_cast<unsigned>(device.lease_crtcs[i]));
            }

            lease_fd = CreateXrandrLease(connection, root, lease, device, driver_device);

            if(lease_fd < 0) {
                Log(ERROR, "XRandR lease: failed to create lease for provider=%u",
                    static_cast<unsigned>(device.provider));

                xcb_disconnect(connection);
                return -1;
            }

            dev_t lease_device;
            if(!GetDrmDeviceNumber(lease_fd, lease_device)) {
                close(lease_fd);
                xcb_disconnect(connection);
                return -1;
            }
        }

        if(lease_fd < 0) {
            Log(ERROR, "XRandR lease: no lease created");
            xcb_disconnect(connection);
            return -1;
        }

        Log(INFO, "XRandR lease: Saving DPMS state...");

        if (!SaveDpmsState(connection)) {
            Log(ERROR, "XRandR lease: Failed to save DPMS state");
            close(lease_fd);
            xcb_disconnect(connection);
            return -1;
        }

        Log(INFO, "XRandR lease: Setting DPMS off...");

        if (SetDpms(connection, XCB_DPMS_DPMS_MODE_OFF) != 0) {
            Log(ERROR, "XRandR lease: Failed to set DPMS off");

            if (!RestoreDpmsState(connection))
                Log(ERROR, "XRandR lease: Failed to restore DPMS after acquisition failure");

            close(lease_fd);
            xcb_disconnect(connection);
            return -1;
        }

        xcb_disconnect(connection);

        Log(INFO, "XRandR lease: SUCCESS fd=%d", lease_fd);
        return lease_fd;
    }

    bool RestoreXrandrDpms()
    {
        int default_screen = 0;
        xcb_connection_t* connection = xcb_connect(nullptr, &default_screen);

        if(!connection || xcb_connection_has_error(connection)) {
            Log(ERROR, "Can't connect to X11");
            if(connection)
                xcb_disconnect(connection);
            return false;
        }

        bool restored = RestoreDpmsState(connection);
        xcb_disconnect(connection);

        if (!restored) {
            Log(ERROR, "XRandR lease: Failed to restore DPMS state");
            return false;
        }

        Log(INFO, "XRandR DPMS restore: SUCCESS");
        return true;
    }
}
