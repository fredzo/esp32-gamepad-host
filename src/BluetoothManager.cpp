/*************************************************************************************************/
#include <Arduino.h>
#include <Esp32GamepadHost.h>
#include <BluetoothManager.h>

extern "C" {
#include <btstack.h>
#include "btstack_config.h"
#include "btstack_run_loop.h"
#ifdef ENABLE_LOG_INFO
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"
#endif
extern void btstack_init();
}


/*************************************************************************************************/

#define L2CAP_CHANNEL_MTU              MAX_BT_DATA_SIZE

#define CLASS_OF_DEVICE_GAMEPAD_START  0x002500
#define CLASS_OF_DEVICE_GAMEPAD_END    0x0025FF
#define CLASS_OF_DEVICE_WIIMOTE        0x002504

// Reference to Esp32GamepadHost instance
Esp32GamepadHost* gamepadHost;

/*************************************************************************************************/

static btstack_packet_callback_registration_t hci_event_callback_registration;

/*************************************************************************************************/

// App
enum BluetoothState {
    INIT,
    READY,
    CONNECTING,
    CONNECTED
};

static BluetoothState bluetoothState = INIT;

////////////////// Gap Inquiry
#define INQUIRY_INTERVAL 5

static void start_scan(void){
    LOG_DEBUG("Starting inquiry scan..\n");
    uint8_t result = gap_inquiry_start(INQUIRY_INTERVAL);
    if(result != ERROR_CODE_SUCCESS) {
        LOG_ERROR("Inquiry failed, status 0x%02x\n", result);
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

#ifdef ENABLE_LOG_INFO
    hci_dump_init(hci_dump_embedded_stdout_get_instance());
#ifndef ENABLE_LOG_DEBUG    
    hci_dump_enable_packet_log(false);
    hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_DEBUG,false);
#endif
#endif
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
                Gamepad* connectingGamepad = gamepadHost->getConnectingGamepad();
                if(connectingGamepad == NULL)
                {
                    LOG_ERROR("ERROR : SDP query complete event received but no device is currently connecting...\n");
                    return;
                }
                status = l2cap_create_channel(packet_handler, connectingGamepad->address, PSM_HID_CONTROL, L2CAP_CHANNEL_MTU, &(connectingGamepad->l2capHidControlCid));
                LOG_DEBUG("SDP_EVENT_QUERY_COMPLETE l2cap_create_channel with device index %d for cid  0x%04x and address %s : status 0x%02x\n",connectingGamepad->index, connectingGamepad->l2capHidControlCid, bd_addr_to_str(connectingGamepad->address), status);
                if (status){
                    LOG_ERROR("Connecting to HID Control failed: 0x%02x\n", status);
                }
            break;
    }
}

static void do_connection_requests(void){
    Gamepad* gamepadToConnect = gamepadHost->askGamepadConnection();
    if(gamepadToConnect != NULL)
    {   // Connect to gamepad
        LOG_DEBUG("Connect to device for index %d.\n",gamepadToConnect->index);
        // TODO : move this to wimote sepcific code
        if(gamepadToConnect->classOfDevice == CLASS_OF_DEVICE_WIIMOTE)
        {   // For wiimote
            LOG_DEBUG("Setting security level to 0 for wiimote.\n");
            gap_set_security_level(LEVEL_0);  
        }
        else
        {   // Default level is 2
            gap_set_security_level(LEVEL_2);  
        }

        LOG_DEBUG("Start SDP HID query for remote HID Device with address=%s.\n",
        bd_addr_to_str(gamepadToConnect->address));
        uint8_t status = sdp_client_query_uuid16(&handle_sdp_client_query_result, gamepadToConnect->address, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
        if (status == ERROR_CODE_SUCCESS) {
            bluetoothState = CONNECTING;
        } else {
            LOG_ERROR("Host connection failed, status 0x%02x\n", status);
        }
    }
}

void bluetoothManagerSendOutputReport(Gamepad* gamepad)
{
    if(gamepad == NULL)
    {
        LOG_ERROR("ERROR : sendOutputReport gamepad must not be NULL.\n");
        return;
    }
    uint8_t result = l2cap_request_can_send_now_event(gamepad->l2capHidInterruptCid);
    if(result)
    {
        LOG_ERROR("ERROR while asking send now event : %04x.\n", result);
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

    Gamepad* connectingGamepad = gamepadHost->getConnectingGamepad();
    if(connectingGamepad == NULL)
    {
        LOG_INFO("L2CAP channel open event received but no device is currently connecting : adding a new device\n");
        connectingGamepad = gamepadHost->addGamepad(address,Gamepad::State::CONNECTING,psm);
    }

    LOG_DEBUG("L2CAP_EVENT_INCOMING_CONNECTION (psm=0x%04x, local_cid=0x%04x, "
    "remote_cid=0x%04x, handle=0x%04x, channel=0x%04x\n", psm, local_cid, remote_cid, handle, channel);
    switch (psm)
    {
        case PSM_HID_CONTROL:
            connectingGamepad->l2capHidControlCid = channel;
            l2cap_accept_connection(channel);
            break;
        case PSM_HID_INTERRUPT:
            connectingGamepad->l2capHidInterruptCid = channel;
            l2cap_accept_connection(channel);
            break;
        default:
            LOG_ERROR("Unknown PSM = 0x%02x\n", psm);
    }
}

static void on_l2cap_channel_closed(uint16_t channel, uint8_t* packet, int16_t size)
{
    uint16_t local_cid;

    UNUSED(size);

    local_cid = l2cap_event_channel_closed_get_local_cid(packet);
    LOG_DEBUG("L2CAP_EVENT_CHANNEL_CLOSED: 0x%04x (channel=0x%04x)\n", local_cid, channel);
    Gamepad* connectingGamepad = gamepadHost->getGamepadForChannel(channel);
    if(connectingGamepad != NULL)
    {
        LOG_INFO("Device with index %d disconnected.\n", connectingGamepad->index);
        connectingGamepad->state = Gamepad::State::DISCONNECTED;
        if(!gamepadHost->hasConnectedGamepad())
        {
            bluetoothState = READY;
        }
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

    LOG_DEBUG("L2CAP_EVENT_CHANNEL_OPENED (channel=0x%04x)\n", channel);

    l2cap_event_channel_opened_get_address(packet, address);
    status = l2cap_event_channel_opened_get_status(packet);
    if (status) {
        LOG_ERROR("L2CAP Connection failed: 0x%02x.\n", status);
        // Practice showed that if any of these two status are received, it is
        // best to remove the link key. But this is based on empirical evidence,
        // not on theory.
        if (status == L2CAP_CONNECTION_RESPONSE_RESULT_RTX_TIMEOUT ||
            status == L2CAP_CONNECTION_BASEBAND_DISCONNECT)
        {
            LOG_INFO("Removing previous link key for address=%s.\n",
            bd_addr_to_str(address));
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

    LOG_DEBUG(
    "PSM: 0x%04x, local CID=0x%04x, remote CID=0x%04x, handle=0x%04x, "
    "incoming=%d, local MTU=%d, remote MTU=%d\n",
    psm, local_cid, remote_cid, handle, incoming, local_mtu, remote_mtu);

    Gamepad* connectingGamepad = gamepadHost->getConnectingGamepad();
    if(connectingGamepad == NULL)
    {
        LOG_ERROR("ERROR : L2CAP channel open event received but no device is currently connecting...\n");
        return;
    }

    switch (psm)
    {
        case PSM_HID_CONTROL:
            connectingGamepad->l2capHidControlCid = l2cap_event_channel_opened_get_local_cid(packet);
            if(!incoming)
            {
                LOG_DEBUG("l2cap_create_channel PSM_HID_INTERRUPT");
                status = l2cap_create_channel(packet_handler, address, PSM_HID_INTERRUPT, L2CAP_CHANNEL_MTU, &(connectingGamepad->l2capHidControlCid));
                if (status)
                {
                    LOG_ERROR("Connecting to HID Control failed: 0x%02x\n", status);
                    break;
                }
            }
            break;

        case PSM_HID_INTERRUPT:
            LOG_INFO("L2CAP interrupt channel open for deviceIndex %d, connection complete.\n",connectingGamepad->index);
            connectingGamepad->l2capHidInterruptCid = l2cap_event_channel_opened_get_local_cid(packet);
            // Connection successfull
            bluetoothState = CONNECTED;
            gamepadHost->completeConnection(connectingGamepad);
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
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t   event;
    bd_addr_t event_addr;
    uint8_t   status;
    uint16_t vendorId;
    uint16_t productId;
    uint32_t classOfDevice;
    event = hci_event_packet_get_type(packet);
    Gamepad* gamepad;

    /* LISTING_RESUME */
    if (bluetoothState == INIT) {
        /* @text In INIT, an inquiry  scan is started, and the application transits to 
         * ACTIVE state.
         */
        switch(event){
            case BTSTACK_EVENT_STATE:
                if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                    start_scan();
                    bluetoothState = READY;
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

                //LOG_DEBUG("HCI_EVENT_PACKET: 0x%02x\n", event);
                switch (event)
                {
                    case GAP_EVENT_INQUIRY_RESULT:
                        gap_event_inquiry_result_get_bd_addr(packet, event_addr);

                        gamepad = gamepadHost->getGamepadForAddress(event_addr);
                        if (gamepad != NULL)
                        {   // already in our list
                            if(gamepad->state != Gamepad::State::CONNECTED)
                            {   // Ask for reconnection
                                gamepad->state = Gamepad::State::CONNECTION_REQUESTED;
                                do_connection_requests();
                            }
                        }
                        else
                        {
                            vendorId = gap_event_inquiry_result_get_device_id_vendor_id(packet);
                            productId = gap_event_inquiry_result_get_device_id_product_id(packet);
                            classOfDevice = gap_event_inquiry_result_get_class_of_device(packet);
                            if(classOfDevice >= CLASS_OF_DEVICE_GAMEPAD_START && classOfDevice <= CLASS_OF_DEVICE_GAMEPAD_END)
                            {   // Filter on gamepads
                                gamepad = gamepadHost->addGamepad(event_addr, 
                                Gamepad::State::CONNECTION_REQUESTED,
                                gap_event_inquiry_result_get_page_scan_repetition_mode(packet),
                                gap_event_inquiry_result_get_clock_offset(packet),
                                vendorId,productId,classOfDevice);

                                // print info
                                LOG_INFO("Device found: %s ",  bd_addr_to_str(event_addr));
                                LOG_INFO("with COD: 0x%06x, ", (unsigned int) classOfDevice);
                                LOG_INFO("pageScan %d, ",      gamepad->pageScanRepetitionMode);
                                LOG_INFO("clock offset 0x%04x",gamepad->clockOffset);
                                if (gap_event_inquiry_result_get_rssi_available(packet)){
                                    LOG_INFO(", rssi %d dBm", (int8_t) gap_event_inquiry_result_get_rssi(packet));
                                }
                                if (gap_event_inquiry_result_get_name_available(packet)){
                                    char name_buffer[240];
                                    int name_len = gap_event_inquiry_result_get_name_len(packet);
                                    memcpy(name_buffer, gap_event_inquiry_result_get_name(packet), name_len);
                                    name_buffer[name_len] = 0;
                                    LOG_INFO(", name '%s'", name_buffer);
                                }
                                LOG_INFO("\n");
                                do_connection_requests();
                            }
                        }   

                        break;

                    case GAP_EVENT_INQUIRY_COMPLETE:
                        LOG_DEBUG("Gap inquiry complete ! Starting a new scan...\n");
                        restart_scan();
                        break;

                    case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                        reverse_bd_addr(&packet[3], event_addr);
                        gamepad = gamepadHost->getGamepadForAddress(event_addr);
                        if (gamepad != NULL) {
                            if (packet[2] == 0) {
                                LOG_INFO("Name: '%s'\n", &packet[9]);
                            } else {
                                LOG_ERROR("Failed to get name: page timeout\n");
                            }
                        }
                        break;

                    /* LISTING_PAUSE */
                    case HCI_EVENT_PIN_CODE_REQUEST:
                        // inform about pin code request
                        LOG_INFO("Pin code request - using '0000'\n");
                        hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                        gap_pin_code_response(event_addr, "0000");
                        break;

                    /* @text When BTSTACK_EVENT_STATE with state HCI_STATE_WORKING
                    * is received and the example is started in client mode, the remote SDP HID query is started.
                    */
                    case BTSTACK_EVENT_STATE:
                        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING)
                        {
                            LOG_DEBUG("[EVENT STATE] Start SDP HID query for remote HID Device with address=%s.\n", bd_addr_to_str(event_addr));
                        }
                        break;

                    case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                        // inform about user confirmation request
                        LOG_DEBUG("SSP User Confirmation Request with numeric value '%" PRIu32 "'\n", little_endian_read_32(packet, 8));
                        LOG_DEBUG("SSP User Confirmation Auto accept\n");
                        break;

                    case HCI_EVENT_COMMAND_COMPLETE:
                    {
                        uint16_t opcode = hci_event_command_complete_get_command_opcode(packet);
                        const uint8_t* param = hci_event_command_complete_get_return_parameters(packet);
                        
                        status = param[0];
                        LOG_DEBUG("--> HCI_EVENT_COMMAND_COMPLETE: opcode = 0x%04x - status=%d\n", opcode, status);
                        break;
                    }
                    case HCI_EVENT_AUTHENTICATION_COMPLETE_EVENT:
                    {
                        status = hci_event_authentication_complete_get_status(packet);
                        uint16_t handle = hci_event_authentication_complete_get_connection_handle(packet);
                        LOG_DEBUG("--> HCI_EVENT_AUTHENTICATION_COMPLETE_EVENT: status=%d, handle=0x%04x\n", status, handle);
                        break;
                    }

                    case HCI_EVENT_ROLE_CHANGE:
                        LOG_DEBUG("--> HCI_EVENT_ROLE_CHANGE\n");
                        break;

                    case HCI_EVENT_CONNECTION_REQUEST:
                    {
                        LOG_DEBUG("--> HCI_EVENT_CONNECTION_REQUEST: link_type = %d <--\n", hci_event_connection_request_get_link_type(packet));
                        hci_event_connection_request_get_bd_addr(packet, event_addr);
                        hci_event_connection_request_get_class_of_device(packet);
                        break;
                    }

                    case HCI_EVENT_CONNECTION_COMPLETE: 
                    {
                        LOG_DEBUG("--> HCI_EVENT_CONNECTION_COMPLETE\n");
                        hci_event_connection_complete_get_bd_addr(packet, event_addr);
                        status = hci_event_connection_complete_get_status(packet);
                        if (status) {
                            LOG_ERROR("on_hci_connection_complete failed (0x%02x) for %s\n", status, bd_addr_to_str(event_addr));
                            return;
                        }
                        break;
                    }
                    
                    case L2CAP_EVENT_INCOMING_CONNECTION:
                        LOG_DEBUG("--> L2CAP_EVENT_INCOMING_CONNECTION\n");
                        on_l2cap_incoming_connection(channel, packet, size);
                        break;

                    case L2CAP_EVENT_CHANNEL_CLOSED:
                        on_l2cap_channel_closed(channel, packet, size);
                        break;

                    case L2CAP_EVENT_CHANNEL_OPENED: 
                        on_l2cap_channel_opened(channel, packet, size);
                        break;
                    case L2CAP_EVENT_CAN_SEND_NOW:
                    {
                        gamepad = gamepadHost->getGamepadForChannel(channel);
                        if(gamepad == NULL)
                        {
                            LOG_ERROR("ERROR : Received can send event for channel 0x%04x : no device found.\n",channel);
                        }
                        if(gamepad->reportLength <= 0)
                        {
                            LOG_ERROR("ERROR : Received can send event for channel 0x%04x : no report to send.\n",channel);
                        }
                        LOG_INFO("Sending output report of length %d for device index %d.\n",gamepad->reportLength,gamepad->index);
                        uint8_t header = (HID_MESSAGE_TYPE_DATA << 4) | HID_REPORT_TYPE_OUTPUT;
                        l2cap_reserve_packet_buffer();
                        uint8_t * out_buffer = l2cap_get_outgoing_buffer();
                        out_buffer[0] = header;
                        out_buffer[1] = gamepad->reportId;
                        memcpy(out_buffer + 2, gamepad->report, gamepad->reportLength);
                        l2cap_send_prepared(gamepad->l2capHidInterruptCid, gamepad->reportLength + 2);
                        gamepad->reportLength = 0;
                        break;
                    }

                    default:
                        break;
                }
                break;
            case L2CAP_DATA_PACKET:
                gamepad = gamepadHost->getGamepadForChannel(channel);
                if (gamepad != NULL)
                {
                    if(gamepad->parseDataPacket(packet,size))
                    {
                        LOG_DEBUG("Gamepad DATA_PACKET ");
                        LOG_HEXDUMP(packet,size);
                        if(packet[7]>=0x20)
                        {
                            printf("Will Rumble.\n");
                            rumbleBtChanel = channel;
                            shouldRumble = true;
                        }
                    }
                }
                else
                {
                    LOG_DEBUG("L2CAP_DATA_PACKET ");
                    LOG_HEXDUMP(packet,size);
                }
            default:
                //LOG_DEBUG("Default for packet type: 0x%02x\n", event);
                break;
        }
    }
}


/*************************************************************************************************/

void bluetoothManagerRun(void)
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
        Gamepad* gamepad = gamepadHost->getGamepadForChannel(rumbleBtChanel);
        gamepad->sendOutputReport(0x11,rumbleState ? rumble : rumbleOff,(sizeof(rumble)-1));
        printf("After Rumble !\n");
    }
}

int btStackMaxConnections;
void configuration_customizer(esp_bt_controller_config_t *cfg)
{
    cfg->bt_max_acl_conn = btStackMaxConnections;
}

/*************************************************************************************************/

int bluetoothManagerInit(int maxConnections)
{   // Store max copnnections param
    btStackMaxConnections = maxConnections;
    
    // Get reference to gamepadHost singleton instance
    gamepadHost = Esp32GamepadHost::getEsp32GamepadHost();

    // Configure BTstack
    btstack_init();
    // Register configuration customizr
    btstack_esp32_register_configuration_customizer(configuration_customizer);

    // Setup for HID Host
    hid_host_setup();

    // Turn on the device 
    hci_power_control(HCI_POWER_ON);

    return 0;
}