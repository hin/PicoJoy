#include <tusb.h>
#include <pico/unique_id.h>

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                 _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))


/*
 * Device Descriptor
 */

tusb_desc_device_t const desc_device =
{
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0xcafe,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    
    .bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}

/*
 * HID Report Descriptor
 */

uint8_t const desc_hid_report[] =
{
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_JOYSTICK),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_REPORT_ID(1)
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
        HID_USAGE_MIN(1),
        HID_USAGE_MAX(32),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(1),
        HID_REPORT_COUNT(32),
        HID_REPORT_SIZE(1),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
        HID_LOGICAL_MIN_N(0x8001, 2),
        HID_LOGICAL_MAX_N(0x7fff, 2),
        HID_USAGE(HID_USAGE_DESKTOP_X),
        HID_USAGE(HID_USAGE_DESKTOP_Y),
        HID_USAGE(HID_USAGE_DESKTOP_Z),
        HID_USAGE(HID_USAGE_DESKTOP_SLIDER),
        HID_REPORT_COUNT(4),
        HID_REPORT_SIZE(16),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf)
{
    (void)itf;
    return desc_hid_report;
}

enum
{
    ITF_NUM_HID,
    ITF_NUM_CDC,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

#define EPNUM_HID       0x81
#define EPNUM_CDC_NOTIF 0x82
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x83

uint8_t const desc_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

    // HID: Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 5, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_configuration;
}

/*
 * String Descriptors
 */

static uint8_t str_to_descr_str (const char *src, uint16_t *dest, uint8_t maxlen)
{
    uint8_t len = strlen(src);
    if (len >= maxlen)
        len = maxlen - 1;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<len; i++)
    {
        dest[i+1] = src[i];
    }
    // first byte is length (including header), second byte is string type
    dest[0] = (TUSB_DESC_STRING << 8 ) | (2*len + 2);
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static uint8_t len;
    static uint16_t desc_str[32];
    static const uint8_t desc_str_len = sizeof(desc_str) / sizeof(desc_str[0]);

    switch(index)
    {
        case 0:
            len = 1;
            desc_str[1] = 0x0904; // Supported Language: English
            // first byte is length (including header), second byte is string type
            // first byte is length (including header), second byte is string type
            desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*len + 2);
            break;
        case 1:
            str_to_descr_str("HasseHans Fabrik", desc_str, desc_str_len);
            break;
        case 2:
            str_to_descr_str("PicoJoy", desc_str, desc_str_len);
            break;
        case 3:
        {
            char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
            pico_get_unique_board_id_string(serial, sizeof(serial));
            str_to_descr_str(serial, desc_str, desc_str_len);
            break;
        }
        case 4:
            str_to_descr_str("PicoJoy Serial Control", desc_str, desc_str_len);
            break;
        default:
            return NULL;
    }

  return desc_str;
}

