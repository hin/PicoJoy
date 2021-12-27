#include <bsp/board.h>
#include <tusb.h>

struct report
{
    uint32_t buttons;
    int16_t sliders[4];
} report;

void hid_task()
{
    // Poll every 1ms
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms += interval_ms;
    
    report.buttons = ((board_millis() / 1000) % 2) << ((board_millis()/2000) % 16);

    for (uint8_t s = 0; s < 4; s++)
    {
        report.sliders[s] = board_millis() % 0x8000;
    }

    if (tud_hid_ready())
    {
        tud_hid_n_report(0x00, 0x01, &report, sizeof(report));
    }
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{

}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{

    return 0;
}

int main()
{
    board_init();
    tusb_init();

    while(1)
    {
        tud_task();
        hid_task();
    }

    return 0;
}