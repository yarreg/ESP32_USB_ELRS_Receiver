// SPDX-License-Identifier: MIT
// ESP32-S3 + ESP-IDF 5.5.x + esp_tinyusb v2.x
// ELRS USB Receiver: HID Gamepad (8x16-bit axes + 8 buttons) + CDC-ACM passthrough (Betaflight/ELRS flashing)

#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"

// TinyUSB / esp_tinyusb
#include "tinyusb.h"                // core (installs stack, TUD_* macros visible via tusb.h)
#include "tinyusb_default_config.h" // TINYUSB_DEFAULT_CONFIG()
#include "tinyusb_cdc_acm.h"        // esp_tinyusb CDC wrapper APIs
#include "tusb.h"                   // descriptor macros (TUD_CONFIG_DESCRIPTOR, TUD_HID_DESCRIPTOR, etc.)

// ------------------------- User config (GPIO/UART) -------------------------
#define UART_RX_BUF_SIZE (2048)
#define UART_TX_BUF_SIZE (2048)

#define SBUS_UART_NUM UART_NUM_1
#define SBUS_UART_RX_GPIO CONFIG_SBUS_UART_RX_GPIO

// -------------------------  SBUS protocol constants -------------------------
#define SBUS_FRAME_LEN 25
#define SBUS_START 0x0F
#define SBUS_END 0x00

typedef struct {
    uint16_t ch[16]; // 16 channels
    bool     frame_lost;
    bool     failsafe;
} sbus_data_t;

// ----------------- HID: Gamepad report descriptor ------------------
// 8 axes (X, Y, Z, Rx, Ry, Rz, Slider, Dial) : 0..2047 (11-bit values in 16-bit fields)
// + 8 buttons (1 bit each)

// clang-format off
#define TUD_HID_REPORT_DESC_GAMEPAD_ELRS(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ) , \
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD ) , \
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ) , \
    __VA_ARGS__ \
    HID_USAGE_PAGE   ( HID_USAGE_PAGE_DESKTOP ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_X ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_Y ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_Z ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_RX ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_RY ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_RZ ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_SLIDER ) , \
      HID_USAGE      ( HID_USAGE_DESKTOP_DIAL ) , \
      HID_LOGICAL_MIN  ( 0 ) , \
      HID_LOGICAL_MAX_N( 0x07ff, 2 ) , /* 0..2047 */ \
      HID_REPORT_COUNT ( 8 ) , \
      HID_REPORT_SIZE  ( 16 ) , \
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) , \
    HID_USAGE_PAGE   ( HID_USAGE_PAGE_BUTTON ) , \
      HID_USAGE_MIN    ( 1 ) , \
      HID_USAGE_MAX    ( 8 ) , \
      HID_LOGICAL_MIN  ( 0 ) , \
      HID_LOGICAL_MAX  ( 1 ) , \
      HID_REPORT_COUNT ( 8 ) , \
      HID_REPORT_SIZE  ( 1 ) , \
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) , \
  HID_COLLECTION_END
// clang-format on

#define HID_EP_SIZE 64

static const uint8_t g_usb_hid_report_desc[] = {TUD_HID_REPORT_DESC_GAMEPAD_ELRS()};

typedef struct __attribute__((packed)) {
    uint16_t ch[8]; // axes
    uint8_t  buttons;
} gamepad_report_t;

_Static_assert(sizeof(gamepad_report_t) <= HID_EP_SIZE, "HID report larger than endpoint packet size");

// ------------------------- USB configuration descriptor (CDC + HID) --------
enum {
    ITF_NUM_CDC = 0,  // CDC Comm
    ITF_NUM_CDC_DATA, // CDC Data
    ITF_NUM_HID,      // HID Gamepad
    ITF_NUM_TOTAL
};

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT 0x02
#define EPNUM_CDC_IN 0x82
#define EPNUM_HID_IN 0x83

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)

static const uint16_t g_usb_lang_id[]     = {0x0409}; // must be little-endian list of supported LANGIDs
static const char    *g_usb_string_desc[] = {
    (const char *) g_usb_lang_id, // 0: LANGID descriptor
    "ESP32-ELRS",                 // 1: Manufacturer
    "USB Receiver S3",            // 2: Product
    "S3-0001",                    // 3: Serial
    "ESP32 CDC",                  // 4: CDC Interface
    "ESP32 HID Gamepad",          // 5: HID Interface
};

// Composite device (CDC + HID) should use MISC / Common / IAD
#define MISC_SUBCLASS_COMMON 0x02
#define MISC_PROTOCOL_IAD 0x01

static const tusb_desc_device_t g_usb_device_desc = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x303A,
    .idProduct          = 0x4005,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

// Re-added full-speed configuration descriptor (CDC + HID)
static const uint8_t g_usb_config_desc[] = {
    // Config + CDC + HID
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 5, HID_ITF_PROTOCOL_NONE, sizeof(g_usb_hid_report_desc), EPNUM_HID_IN, HID_EP_SIZE,
                       1),
};

static uint16_t get_hid_value(uint16_t v, bool reverse) {
    const uint16_t min_val = 200, max_val = 1800;
    const uint16_t hid_min = 0, hid_max = 2047;

    // scale from [min_val..max_val] to [hid_min..hid_max]
    if (v < min_val)
        v = min_val;
    if (v > max_val)
        v = max_val;
    uint16_t scaled = (uint16_t) ((uint32_t) (v - min_val) * (hid_max - hid_min) / (max_val - min_val) + hid_min);
    if (reverse)
        scaled = hid_max - scaled;

    return scaled;
}

static void sbus_channel_to_hid(sbus_data_t *sbus_frame, gamepad_report_t *gp) {
    gp->ch[0] = get_hid_value(sbus_frame->ch[3], false); // yaw
    gp->ch[1] = get_hid_value(sbus_frame->ch[2], true);  // Throttle

    gp->ch[2] = get_hid_value(sbus_frame->ch[0], false); // roll
    gp->ch[3] = get_hid_value(sbus_frame->ch[1], false); // pitch

    gp->ch[4] = get_hid_value(sbus_frame->ch[4], false); // A
    gp->ch[5] = get_hid_value(sbus_frame->ch[5], false); // B
    gp->ch[6] = get_hid_value(sbus_frame->ch[6], false); // C
    gp->ch[7] = get_hid_value(sbus_frame->ch[7], false); // D
}

static bool sbus_read(sbus_data_t *sbus_frame) {
    // SBUS frame: 25 bytes
    // [0] = 0x0F (start)
    // [1..22] = 16 channels, 11 bits each, packed into 22 bytes
    // [23] = flags (digital channels, failsafe, frame lost)
    // [24] = 0x00 (end)

    // sync to start byte
    int     n;
    uint8_t frame[SBUS_FRAME_LEN];

    // Check for We are in middle of a frame, wait until next start byte
    do {
        n = uart_read_bytes(SBUS_UART_NUM, &frame[0], 1, pdMS_TO_TICKS(10));
        if (n != 1)
            return false; // timeout or error
    } while (frame[0] != SBUS_START);

    // read full frame and check tail byte
    n = uart_read_bytes(SBUS_UART_NUM, &frame[1], SBUS_FRAME_LEN - 1, pdMS_TO_TICKS(10));
    if (n != SBUS_FRAME_LEN - 1)
        return false; // timeout or error

    if (frame[24] != SBUS_END)
        return false; // invalid end byte

    // unpack channels
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t  bit  = (uint8_t) i * 11u;
        uint8_t  byte = bit >> 3;
        uint8_t  ofs  = bit & 7u;
        uint32_t v =
            (uint32_t) frame[1 + byte] | ((uint32_t) frame[2 + byte] << 8) | ((uint32_t) frame[3 + byte] << 16);
        sbus_frame->ch[i] = (uint16_t) ((v >> ofs) & 0x07FFu);
    }

    // flags: b0=CH17, b1=CH18, b2=frame_lost, b3=failsafe
    uint8_t flags          = frame[23];
    sbus_frame->frame_lost = (flags & 0x04u) != 0;
    sbus_frame->failsafe   = (flags & 0x08u) != 0;

    return true;
}

static void display_joystick_state(const gamepad_report_t *gp) {
    const tinyusb_cdcacm_itf_t CDC_ITF = TINYUSB_CDC_ACM_0;

    static uint32_t sample_count = 0;
    if (++sample_count % 50 != 0) {
        return; // throttle: only every 50th sample
    }

    static bool first = true;

    static const char *axis_lbl[8] = {"Yaw", "Throttle", "Roll", "Pitch", "A", "B", "C", "D"};
    const int          name_width  = 8; // max strlen("Throttle")
    char               out[512];
    size_t             pos = 0;

    // Clear screen once, then just move cursor to top-left (htop-like refresh)
    if (first) {
        pos += snprintf(out + pos, sizeof(out) - pos, "\x1b[2J");
        first = false;
    }
    // Cursor home
    pos += snprintf(out + pos, sizeof(out) - pos, "\x1b[H");
    pos += snprintf(out + pos, sizeof(out) - pos, "---- State (every 50 frames) ----\r\n");

    for (int i = 0; i < 8 && pos < sizeof(out); ++i) {
        uint16_t v       = gp->ch[i]; // 0..2047
        uint16_t clamped = (v > 2047) ? 2047 : v;
        int      bar_len = (clamped * 32 + 1023) / 2047; // wider bar (0..32) for better visual
        if (bar_len > 32)
            bar_len = 32;

        char bar[33];
        for (int b = 0; b < 32; ++b)
            bar[b] = (b < bar_len) ? '#' : '.';
        bar[32] = 0;

        pos += snprintf(out + pos, sizeof(out) - pos, "%-*s %4u [%s]\r\n", name_width, axis_lbl[i], v, bar);
    }

    if (pos < sizeof(out)) {
        char map[9];
        for (int b = 0; b < 8; ++b) {
            map[b] = (gp->buttons & (1u << b)) ? ('1' + b) : '-';
        }
        map[8] = 0;

        pos += snprintf(out + pos, sizeof(out) - pos, "Buttons: %s  (", map);
        bool first_btn = true;
        for (int b = 0; b < 8; ++b) {
            if (gp->buttons & (1u << b)) {
                pos += snprintf(out + pos, sizeof(out) - pos, "%s%d", first_btn ? "" : ",", b + 1);
                first_btn = false;
            }
        }
        if (first_btn && pos < sizeof(out))
            pos += snprintf(out + pos, sizeof(out) - pos, "none");
        if (pos < sizeof(out))
            pos += snprintf(out + pos, sizeof(out) - pos, ")\r\n");
    }

    if (pos && pos < sizeof(out)) {
        tinyusb_cdcacm_write_queue(CDC_ITF, (uint8_t *) out, pos);
        tinyusb_cdcacm_write_flush(CDC_ITF, 0);
    }
}

static void task_sbus_hid(void *arg) {
    (void) arg;

    sbus_data_t      sbus_frame = {0};
    gamepad_report_t gp         = {0};

    while (true) {
        if (!sbus_read(&sbus_frame)) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        sbus_channel_to_hid(&sbus_frame, &gp);
        if (tud_hid_ready() && tud_mounted()) {
            tud_hid_report(0, &gp, sizeof(gp)); // reportID=0 (single report)
        }

        display_joystick_state(&gp);
    }
}

// ------------------------- HID callbacks (report only) ---------------------

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return g_usb_hid_report_desc;
}

uint16_t tud_hid_get_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t *, uint16_t) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t, uint8_t, hid_report_type_t, const uint8_t *, uint16_t) {
}

static void init_uart(void) {
    // Init UART for SBUS
    const uart_config_t sbus_uat_cfg = {
        .baud_rate  = 100000,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_EVEN,
        .stop_bits  = UART_STOP_BITS_2,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(SBUS_UART_NUM, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, 0, NULL, 0);
    uart_param_config(SBUS_UART_NUM, &sbus_uat_cfg);
    uart_set_pin(SBUS_UART_NUM, UART_PIN_NO_CHANGE, SBUS_UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_line_inverse(SBUS_UART_NUM, UART_SIGNAL_RXD_INV);
}

void app_main(void) {
    const char *TAG = "app";
    ESP_LOGI(TAG, "Starting application");
    init_uart();

    // TinyUSB stack config
    tinyusb_config_t tusb_cfg             = TINYUSB_DEFAULT_CONFIG();
    tusb_cfg.descriptor.device            = &g_usb_device_desc;
    tusb_cfg.descriptor.string            = g_usb_string_desc;
    tusb_cfg.descriptor.string_count      = (int) (sizeof(g_usb_string_desc) / sizeof(g_usb_string_desc[0]));
    tusb_cfg.descriptor.full_speed_config = g_usb_config_desc;
    tusb_cfg.descriptor.high_speed_config = NULL;
    tusb_cfg.event_cb                     = NULL;
    tusb_cfg.event_arg                    = NULL;

    // Sanity: check descriptor length matches our macro constant
    if (CONFIG_TOTAL_LEN != sizeof(g_usb_config_desc)) {
        ESP_LOGW(TAG, "CONFIG_TOTAL_LEN(%u) != real config desc size(%u)", (unsigned) CONFIG_TOTAL_LEN,
                 (unsigned) sizeof(g_usb_config_desc));
    }

    ESP_LOGI(TAG, "Installing TinyUSB driver");
    esp_err_t err = tinyusb_driver_install(&tusb_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "tinyusb_driver_install failed: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "TinyUSB driver installed, proceeding with CDC init");

    // CDC config with active callbacks
    const tinyusb_config_cdcacm_t cdc_cfg = {
        .cdc_port                     = TINYUSB_CDC_ACM_0,
        .callback_rx                  = NULL,
        .callback_rx_wanted_char      = NULL,
        .callback_line_state_changed  = NULL,
        .callback_line_coding_changed = NULL,
    };
    err = tinyusb_cdcacm_init(&cdc_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "tinyusb_cdcacm_init failed: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "CDC class initialized");

    // Task creation
    xTaskCreatePinnedToCore(task_sbus_hid, "sbus_hid", 4096, NULL, 5, NULL, 0);
}
