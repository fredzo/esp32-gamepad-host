/*************************************************************************************************/
#include <Arduino.h>
#include "btstack.h"
#include <BluetoothManager.h>

#include "btstack_config.h"
#include "btstack_run_loop.h"
#include "hci_dump.h"

#include "esp_bt.h"

#include <inttypes.h>

extern void btstack_init();

/*************************************************************************************************/

#define L2CAP_CHANNEL_MTU 128

#define MAX_ATTRIBUTE_VALUE_SIZE 300

#define CLASS_OF_DEVICE_GAMEPAD_START  0x002500
#define CLASS_OF_DEVICE_GAMEPAD_END    0x0025FF
#define CLASS_OF_DEVICE_WIIMOTE        0x002504



/*************************************************************************************************/

//static bd_addr_t remote_addr;

static btstack_packet_callback_registration_t hci_event_callback_registration;

/*************************************************************************************************/

// SDP
static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];

// Last packet
static uint8_t last_packet[128];

// App
static enum {
    APP_INIT,
    APP_IDLE,
    APP_CONNECTING,
    APP_CONNECTED
} app_state = APP_INIT;

////////////////// Gap Inquiry

#define MAX_DEVICES 20
enum DEVICE_STATE { CONNECTION_REQUESTED, CONNECTING, CONNECTED, DISCONNECTED };
struct device {
    bd_addr_t          address;
    uint8_t            pageScanRepetitionMode;
    uint16_t           clockOffset;
    uint32_t           classOfDevice;
    uint16_t           l2capHidControlCid;
    uint16_t           l2capHidInterruptCid;
    // Four output reports
    uint8_t            reportId;
    const uint8_t *    report;
    uint16_t           reportLength;

    // State
    enum DEVICE_STATE  state; 
};

#define INQUIRY_INTERVAL 5
struct device devices[MAX_DEVICES];
int deviceCount = 0;
int currentConnectingDevice = -1;

static int getDeviceIndexForAddress( bd_addr_t addr){
    int j;
    for (j=0; j< deviceCount; j++){
        if (bd_addr_cmp(addr, devices[j].address) == 0){
            return j;
        }
    }
    return -1;
}

static int getDeviceIndexForChannel(uint16_t channel){
    int j;
    if(channel > 0)
    {
        for (j=0; j< deviceCount; j++){
            if (devices[j].l2capHidControlCid == channel || devices[j].l2capHidInterruptCid == channel){
                return j;
            }
        }
    }
    return -1;
}

static void start_scan(void){
    printf("Starting inquiry scan..\n");
    uint8_t result = gap_inquiry_start(INQUIRY_INTERVAL);
    if(result != ERROR_CODE_SUCCESS) {
        printf("Inquiry failed, status 0x%02x\n", result);
    }
}

static void restart_scan(void){
    start_scan();
}

//////////////////



/* @section Main application configuration
 *
 * @text In the application configuration, L2CAP is initialized 
 */

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static void hid_host_setup(void)
{
    // Initialize L2CAP 
    l2cap_init();

    // register L2CAP Services for reconnections
    l2cap_register_service(packet_handler, PSM_HID_INTERRUPT, 0xffff, gap_get_security_level());
    l2cap_register_service(packet_handler, PSM_HID_CONTROL, 0xffff, gap_get_security_level());

    // Allow sniff mode requests by HID device and support role switch
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);

    // try to become master on incoming connections
    hci_set_master_slave_policy(HCI_ROLE_MASTER);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    //hci_dump_init(hci_dump_embedded_stdout_get_instance());
    //hci_dump_enable_packet_log(false);
    //hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_DEBUG,false);

    // Disable stdout buffering
    setvbuf(stdin, NULL, _IONBF, 0);
}

const uint8_t rumble[] = {0xc0, 0x20, 
0xf3, // Update // 0c
0x02, 0x00,  //020 f7
0xFF, // Left
0xFF, // Right
0xFF, // Red
0x00, // Green
0xFF, // Blue
0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x43, 0x00, 0x4d, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t rumbleOff[] = {0xc0, 0x20, 
0xf3, // Update
0x02, 0x00, 
0x00, // Left
0x00, // Right
0x00, // Red
0xFF, // Green
0x00, // Blue
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x43, 0x00, 0x4d, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint16_t rumbleBtChanel;
bool shouldRumble = false;



/* @section SDP parser callback 
 * 
 * @text The SDP parsers retrieves the BNEP PAN UUID as explained in  
 * Section [on SDP BNEP Query example](#sec:sdpbnepqueryExample}.
 */

static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    uint8_t status;

    switch (hci_event_packet_get_type(packet))
    {
        case SDP_EVENT_QUERY_COMPLETE:
                if(currentConnectingDevice < 0)
                {
                    printf("ERROR : SDP query complete event received but no device is currently connecting...\n");
                    return;
                }
                status = l2cap_create_channel(packet_handler, devices[currentConnectingDevice].address, PSM_HID_CONTROL, L2CAP_CHANNEL_MTU, &(devices[currentConnectingDevice].l2capHidControlCid));
                printf("SDP_EVENT_QUERY_COMPLETE l2cap_create_channel with device index %d for cid  0x%04x and address %s : status 0x%02x\n",currentConnectingDevice, devices[currentConnectingDevice].l2capHidControlCid, bd_addr_to_str(devices[currentConnectingDevice].address), status);
                if (status){
                    printf("Connecting to HID Control failed: 0x%02x\n", status);
                }
            break;
    }
}

static void list_link_keys(void)
{
    bd_addr_t  addr;
    link_key_t link_key;
    link_key_type_t type;
    btstack_link_key_iterator_t it;

    int ok = gap_link_key_iterator_init(&it);
    if (!ok) {
        printf("Link key iterator not implemented\n");
        return;
    }
    uint8_t delete_keys = 0; //uni_platform_is_button_pressed();
    if (delete_keys)
        printf("Deleting stored link keys:\n");
    else
        printf("Stored link keys:\n");
    while (gap_link_key_iterator_get_next(&it, addr, link_key, &type)) {
        printf("%s - type %u, key: ", bd_addr_to_str(addr), (int)type);
        debug_hexdump(link_key, 16);
        if (delete_keys) {
        gap_drop_link_key_for_bd_addr(addr);
        }
    }
    printf(".\n");
    gap_link_key_iterator_done(&it);
}

static void do_connection_requests(void){
    int i;
    // Prevent simmultaneous connection process
    if(currentConnectingDevice >= 0) return;
    for (i=0;i<deviceCount;i++) {
        // remote name request
        if (devices[i].state == CONNECTION_REQUESTED){
            devices[i].state = CONNECTING;
            // Connect to device
            currentConnectingDevice = i;
            printf("Connect to device for index %d.\n",currentConnectingDevice);
            if(devices[i].classOfDevice == CLASS_OF_DEVICE_WIIMOTE)
            {   // For wiimote
                printf("Setting security level to 0 for wiimote.\n");
                gap_set_security_level(LEVEL_0);  
            }
            else
            {   // Default level is 2
                gap_set_security_level(LEVEL_2);  
            }

            printf("Start SDP HID query for remote HID Device with address=%s.\n",
            bd_addr_to_str(devices[i].address));
            //list_link_keys();
            uint8_t status = sdp_client_query_uuid16(&handle_sdp_client_query_result, devices[i].address, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
            if (status == ERROR_CODE_SUCCESS) {
                app_state = APP_CONNECTING;
            } else {
                printf("Host connection failed, status 0x%02x\n", status);
            }
            // End loop until connection is complete
            return;
        }
    }
}

int addDevice(bd_addr_t address, uint8_t pageScanRepetitionMode, uint16_t clockOffset, uint32_t classOfDevice, enum DEVICE_STATE state)
{
    int deviceIndex = (deviceCount < MAX_DEVICES) ? deviceCount : 0;  // add at the begining if no room left
    memcpy(devices[deviceIndex].address, address, 6);
    devices[deviceIndex].pageScanRepetitionMode = pageScanRepetitionMode;
    devices[deviceIndex].clockOffset = clockOffset;
    devices[deviceIndex].classOfDevice = classOfDevice;
    devices[deviceIndex].state = state;
    if(deviceIndex == deviceCount)  deviceCount++;
    return deviceIndex;
}

void sendOutputReport(int deviceIndex, uint8_t reportId, const uint8_t * report, uint8_t reportLength)
{
    if(deviceIndex < 0 || deviceIndex >= deviceCount)
    {
        printf("ERROR : Invalid device index %d.\n",deviceIndex);
        return;
    }
    if(devices[deviceIndex].state != CONNECTED)
    {
        printf("ERROR : Invalid device state for device with index %d.\n", deviceIndex);
        return;
    }
    devices[deviceIndex].reportId = reportId;
    devices[deviceIndex].report = report;
    devices[deviceIndex].reportLength = reportLength;
    uint8_t result = l2cap_request_can_send_now_event(devices[deviceIndex].l2capHidInterruptCid);
    if(result)
    {
        printf("ERROR while asking send now event : %04x.\n", result);
    }
}

static void on_l2cap_incoming_connection(uint16_t channel, uint8_t* packet, uint16_t size)
{
    uint16_t local_cid, remote_cid;
    uint16_t psm;
    hci_con_handle_t handle;
    bd_addr_t address;

    UNUSED(size);

    psm = l2cap_event_incoming_connection_get_psm(packet);
    handle = l2cap_event_incoming_connection_get_handle(packet);
    remote_cid = l2cap_event_incoming_connection_get_remote_cid(packet);
    local_cid = l2cap_event_channel_opened_get_local_cid(packet);
    l2cap_event_incoming_connection_get_address(packet,address);

    if(currentConnectingDevice < 0)
    {
        printf("L2CAP channel open event received but no device is currently connecting : adding a new device\n");
        currentConnectingDevice = addDevice(address,psm,0,CLASS_OF_DEVICE_GAMEPAD_START,CONNECTING);
    }

    printf("L2CAP_EVENT_INCOMING_CONNECTION (psm=0x%04x, local_cid=0x%04x, "
    "remote_cid=0x%04x, handle=0x%04x, channel=0x%04x\n", psm, local_cid, remote_cid, handle, channel);
    switch (psm)
    {
        case PSM_HID_CONTROL:
            devices[currentConnectingDevice].l2capHidControlCid = channel;
            l2cap_accept_connection(channel);
            break;
        case PSM_HID_INTERRUPT:
            devices[currentConnectingDevice].l2capHidInterruptCid = channel;
            l2cap_accept_connection(channel);
            break;
        default:
            printf("Unknown PSM = 0x%02x\n", psm);
    }
}

static void on_l2cap_channel_closed(uint16_t channel, uint8_t* packet, int16_t size)
{
    uint16_t local_cid;

    UNUSED(size);

    local_cid = l2cap_event_channel_closed_get_local_cid(packet);
    printf("L2CAP_EVENT_CHANNEL_CLOSED: 0x%04x (channel=0x%04x)\n", local_cid, channel);
    int deviceIndex = getDeviceIndexForChannel(channel);
    if(deviceIndex >= 0)
    {
        printf("Device with index %d disconnected.\n", deviceIndex);
        devices[deviceIndex].state = DISCONNECTED;
    }
}

static void on_l2cap_channel_opened(uint16_t channel, uint8_t* packet, uint16_t size)
{
    uint16_t psm;
    uint8_t status;
    uint16_t local_cid, remote_cid;
    uint16_t local_mtu, remote_mtu;
    hci_con_handle_t handle;
    bd_addr_t address;
    uint8_t incoming;

    UNUSED(size);

    printf("L2CAP_EVENT_CHANNEL_OPENED (channel=0x%04x)\n", channel);

    l2cap_event_channel_opened_get_address(packet, address);
    status = l2cap_event_channel_opened_get_status(packet);
    if (status) {
        printf("L2CAP Connection failed: 0x%02x.\n", status);
        // Practice showed that if any of these two status are received, it is
        // best to remove the link key. But this is based on empirical evidence,
        // not on theory.
        if (status == L2CAP_CONNECTION_RESPONSE_RESULT_RTX_TIMEOUT ||
            status == L2CAP_CONNECTION_BASEBAND_DISCONNECT)
        {
            printf("Removing previous link key for address=%s.\n",
            bd_addr_to_str(address));
            //uni_hid_device_remove_entry_with_channel(channel);
            // Just in case the key is outdated we remove it. If fixes some
            // l2cap_channel_opened issues. It proves that it works when the status
            // is 0x6a (L2CAP_CONNECTION_BASEBAND_DISCONNECT).
            gap_drop_link_key_for_bd_addr(address);
        }
        return;
    }
    psm = l2cap_event_channel_opened_get_psm(packet);
    local_cid = l2cap_event_channel_opened_get_local_cid(packet);
    remote_cid = l2cap_event_channel_opened_get_remote_cid(packet);
    handle = l2cap_event_channel_opened_get_handle(packet);
    incoming = l2cap_event_channel_opened_get_incoming(packet);
    local_mtu = l2cap_event_channel_opened_get_local_mtu(packet);
    remote_mtu = l2cap_event_channel_opened_get_remote_mtu(packet);

    printf(
    "PSM: 0x%04x, local CID=0x%04x, remote CID=0x%04x, handle=0x%04x, "
    "incoming=%d, local MTU=%d, remote MTU=%d\n",
    psm, local_cid, remote_cid, handle, incoming, local_mtu, remote_mtu);

    if(currentConnectingDevice < 0)
    {
        printf("ERROR : L2CAP channel open event received but no device is currently connecting...\n");
        return;
    }

    switch (psm)
    {
        case PSM_HID_CONTROL:
            devices[currentConnectingDevice].l2capHidControlCid = l2cap_event_channel_opened_get_local_cid(packet);
            if(!incoming)
            {
                printf("l2cap_create_channel PSM_HID_INTERRUPT");
                status = l2cap_create_channel(packet_handler, address, PSM_HID_INTERRUPT, L2CAP_CHANNEL_MTU, &(devices[currentConnectingDevice].l2capHidControlCid));
                if (status)
                {
                    printf("Connecting to HID Control failed: 0x%02x\n", status);
                    break;
                }
            }
            break;

        case PSM_HID_INTERRUPT:
            printf("L2CAP interrupt channel open for deviceIndex %d, connection complete.\n",currentConnectingDevice);
            devices[currentConnectingDevice].l2capHidInterruptCid = l2cap_event_channel_opened_get_local_cid(packet);
            // Connection successfull
            devices[currentConnectingDevice].state = CONNECTED;
            if(devices[currentConnectingDevice].classOfDevice == CLASS_OF_DEVICE_WIIMOTE)
            {   // TODO : Move this code to wiimote driver
                uint8_t leds = 0b0001;
                uint8_t payload[1];
                payload[0] = (uint8_t)(leds << 4);
                sendOutputReport(currentConnectingDevice,0x11,payload,1);
            }
            currentConnectingDevice = -1;            
            // Check for other connections
            do_connection_requests();
            break;
    }
}

/*
 * @section Packet Handler
 * 
 * @text The packet handler responds to various HCI Events.
 */


/* LISTING_START(packetHandler): Packet Handler */
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    /* LISTING_PAUSE */
    uint8_t   event;
    bd_addr_t event_addr;
    uint8_t   status;

    int deviceIndex;

    uint32_t classOfDevice;

    event = hci_event_packet_get_type(packet);

    /* LISTING_RESUME */
    if (app_state == APP_INIT) {
        /* @text In INIT, an inquiry  scan is started, and the application transits to 
         * ACTIVE state.
         */
        switch(event){
            case BTSTACK_EVENT_STATE:
                if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                    start_scan();
                    app_state = APP_IDLE;
                }
                break;
            default:
                break;
        }
    }
    else
    {
        /* LISTING_RESUME */
        switch (packet_type)
        {
            case HCI_EVENT_PACKET:

                //printf("HCI_EVENT_PACKET: 0x%02x\n", hci_event_packet_get_type(packet));

                switch (event)
                {
                    case GAP_EVENT_INQUIRY_RESULT:
                        if (deviceCount >= MAX_DEVICES) break;  // already full
                        gap_event_inquiry_result_get_bd_addr(packet, event_addr);

                        deviceIndex = getDeviceIndexForAddress(event_addr);
                        if (deviceIndex >= 0)
                        {   // already in our list
                            if(devices[deviceIndex].state != CONNECTED)
                            {   // Ask for reconnection
                                devices[deviceIndex].state = CONNECTION_REQUESTED;
                                do_connection_requests();
                            }
                        }
                        else
                        {
                            classOfDevice = gap_event_inquiry_result_get_class_of_device(packet);
                            if(classOfDevice >= CLASS_OF_DEVICE_GAMEPAD_START && classOfDevice <= CLASS_OF_DEVICE_GAMEPAD_END)
                            {   // Filter on gamepads
                                deviceIndex = addDevice(event_addr, 
                                gap_event_inquiry_result_get_page_scan_repetition_mode(packet),
                                gap_event_inquiry_result_get_clock_offset(packet),
                                classOfDevice,
                                CONNECTION_REQUESTED);

                                // print info
                                printf("Device found: %s ",  bd_addr_to_str(event_addr));
                                printf("with COD: 0x%06x, ", (unsigned int) classOfDevice);
                                printf("pageScan %d, ",      devices[deviceIndex].pageScanRepetitionMode);
                                printf("clock offset 0x%04x",devices[deviceIndex].clockOffset);
                                if (gap_event_inquiry_result_get_rssi_available(packet)){
                                    printf(", rssi %d dBm", (int8_t) gap_event_inquiry_result_get_rssi(packet));
                                }
                                if (gap_event_inquiry_result_get_name_available(packet)){
                                    char name_buffer[240];
                                    int name_len = gap_event_inquiry_result_get_name_len(packet);
                                    memcpy(name_buffer, gap_event_inquiry_result_get_name(packet), name_len);
                                    name_buffer[name_len] = 0;
                                    printf(", name '%s'", name_buffer);
                                }
                                printf("\n");
                                do_connection_requests();
                            }
                        }   

                        break;

                    case GAP_EVENT_INQUIRY_COMPLETE:
                        printf("Gap inquiry complete ! Starting a new scan...\n");
                        restart_scan();
                        break;

                    case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                        reverse_bd_addr(&packet[3], event_addr);
                        deviceIndex = getDeviceIndexForAddress(event_addr);
                        if (deviceIndex >= 0) {
                            if (packet[2] == 0) {
                                printf("Name: '%s'\n", &packet[9]);
                            } else {
                                printf("Failed to get name: page timeout\n");
                            }
                        }
                        break;

                    /* LISTING_PAUSE */
                    case HCI_EVENT_PIN_CODE_REQUEST:
                        // inform about pin code request
                        printf("Pin code request - using '0000'\n");
                        hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                        gap_pin_code_response(event_addr, "0000");
                        break;

                    /* @text When BTSTACK_EVENT_STATE with state HCI_STATE_WORKING
                    * is received and the example is started in client mode, the remote SDP HID query is started.
                    */
                    case BTSTACK_EVENT_STATE:
                        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING)
                        {
                            /*printf("[EVENT STATE] Start SDP HID query for remote HID Device with address=%s.\n",
                            bd_addr_to_str(remote_addr));
                            list_link_keys();
                            sdp_client_query_uuid16(&handle_sdp_client_query_result, remote_addr, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
                            */
                        }
                        break;

                    case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                        // inform about user confirmation request
                        printf("SSP User Confirmation Request with numeric value '%" PRIu32 "'\n", little_endian_read_32(packet, 8));
                        printf("SSP User Confirmation Auto accept\n");
                        break;

                    case HCI_EVENT_COMMAND_COMPLETE:
                    {
                        uint16_t opcode = hci_event_command_complete_get_command_opcode(packet);
                        const uint8_t* param = hci_event_command_complete_get_return_parameters(packet);
                        
                        status = param[0];
                        printf("--> HCI_EVENT_COMMAND_COMPLETE: opcode = 0x%04x - status=%d\n", opcode, status);
                        break;
                    }
                    case HCI_EVENT_AUTHENTICATION_COMPLETE_EVENT:
                    {
                        status = hci_event_authentication_complete_get_status(packet);
                        uint16_t handle =
                            hci_event_authentication_complete_get_connection_handle(packet);
                        printf(
                            "--> HCI_EVENT_AUTHENTICATION_COMPLETE_EVENT: status=%d, "
                            "handle=0x%04x\n",
                            status, handle);
                        break;
                    }

                    case HCI_EVENT_ROLE_CHANGE:
                        printf("--> HCI_EVENT_ROLE_CHANGE\n");
                        break;

                    case HCI_EVENT_CONNECTION_REQUEST:
                    {
                        printf("--> HCI_EVENT_CONNECTION_REQUEST: link_type = %d <--\n", hci_event_connection_request_get_link_type(packet));
                        hci_event_connection_request_get_bd_addr(packet, event_addr);
                        hci_event_connection_request_get_class_of_device(packet);
                        break;
                    }

                    case HCI_EVENT_CONNECTION_COMPLETE: 
                    {
                        printf("--> HCI_EVENT_CONNECTION_COMPLETE\n");
                        hci_event_connection_complete_get_bd_addr(packet, event_addr);
                        status = hci_event_connection_complete_get_status(packet);
                        if (status) {
                            printf("on_hci_connection_complete failed (0x%02x) for %s\n", status,
                                bd_addr_to_str(event_addr));
                            return;
                        }
                        break;
                    }
                    
                    case L2CAP_EVENT_INCOMING_CONNECTION:
                        printf("--> L2CAP_EVENT_INCOMING_CONNECTION\n");
                        on_l2cap_incoming_connection(channel, packet, size);
                        break;

                    case L2CAP_EVENT_CHANNEL_CLOSED:
                        on_l2cap_channel_closed(channel, packet, size);
                        break;

                    case L2CAP_EVENT_CHANNEL_OPENED: 
                        on_l2cap_channel_opened(channel, packet, size);
                        break;
                    case L2CAP_EVENT_CAN_SEND_NOW:
                        deviceIndex = getDeviceIndexForChannel(channel);
                        if(deviceIndex < 0)
                        {
                            printf("ERROR : Received can send event for channel 0x%04x : no device found.\n",channel);
                        }
                        if(devices[deviceIndex].reportLength <= 0)
                        {
                            printf("ERROR : Received can send event for channel 0x%04x : no report to send.\n",channel);
                        }
                        printf("Sending output report of length %d for device index %d.\n",devices[deviceIndex].reportLength,deviceIndex);
                        uint8_t header = (HID_MESSAGE_TYPE_DATA << 4) | HID_REPORT_TYPE_OUTPUT;
                        l2cap_reserve_packet_buffer();
                        uint8_t * out_buffer = l2cap_get_outgoing_buffer();
                        out_buffer[0] = header;
                        out_buffer[1] = devices[deviceIndex].reportId;
                        (void)memcpy(out_buffer + 2, devices[deviceIndex].report, devices[deviceIndex].reportLength);
                        l2cap_send_prepared(devices[deviceIndex].l2capHidInterruptCid, devices[deviceIndex].reportLength + 2);
                        devices[deviceIndex].reportLength = 0;
                        break;

                    default:
                        break;
                }
                break;
            case L2CAP_DATA_PACKET:
                //debug("L2CAP_DATA_PACKET ");
                
                // for now, just dump incoming data
                //if (channel == l2cap_hid_interrupt_cid)
                {
    /*                //printf("HID interrupt: ");
                    //debug_hexdump(packet, size);
                    gamepad_handler(packet, size);*/
                    if(size < 128 && (memcmp(last_packet,packet,size)!=0))
                    {
                        //printf("Mecmp = %d.\n",memcmp(last_packet,report,packetSize));
                        //printf_hexdump(last_packet,packetSize);
                        memcpy(last_packet,packet,size);
                        printf_hexdump(packet,size);
                        if(packet[7]>=0x20)
                        {
                            printf("Will Rumble.\n");
                            rumbleBtChanel = channel;
                            shouldRumble = true;
                        }

                    }

                }
                /*else if (channel == l2cap_hid_control_cid)
                {
                    printf("HID Control: ");
                    debug_hexdump(packet, size);
                }
                else
                    break;
                */
            default:
                //printf("default: 0x%02x\n", hci_event_packet_get_type(packet));
                break;
        }
    }
}


/*************************************************************************************************/

void btstackRun(void)
{
    btstack_run_loop_execute();
}


bool rumbleState = false;
void maybeRumble()
{
    if(shouldRumble)
    {
        shouldRumble = false;
        rumbleState = !rumbleState;
        printf("Before Rumble with state %d.\n",rumbleState);
        int deviceIndex = getDeviceIndexForChannel(rumbleBtChanel);
        sendOutputReport(deviceIndex,0x11,rumbleState ? rumble : rumbleOff,(sizeof(rumble)-1));
        printf("After Rumble !\n");
    }
}

int btStackMaxConnections;
void configuration_customizer(esp_bt_controller_config_t *cfg)
{
    cfg->bt_max_acl_conn = btStackMaxConnections;
}

/*************************************************************************************************/
//static const char remote_addr_string[] = "1F-97-19-05-06-07";
static const char remote_addr_string[] = "15-97-19-05-06-07";
//static const char remote_addr_string[] = "CC-9E-00-C9-FC-F1";

int btstackInit(int maxConnections)
{
    btStackMaxConnections = maxConnections;
    // Configure BTstack
    btstack_init();
    // Register configuration customizr
    btstack_esp32_register_configuration_customizer(configuration_customizer);

    // Setup for HID Host
    hid_host_setup();

    // Parse human readable Bluetooth address
    //sscanf_bd_addr(remote_addr_string, remote_addr);

    // Turn on the device 
    hci_power_control(HCI_POWER_ON);

    return 0;
}