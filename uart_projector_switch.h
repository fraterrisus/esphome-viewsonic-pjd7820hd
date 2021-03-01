#include "esphome.h"

/*
 * Push "Menu" button
 *   data: [2, 20, 0, 4, 0, 52, 2, 4, 15, 97]
 *   {0x02,0x14,0x00,0x04,0x00,0x34,0x02,0x04,0x0F,0x61}
 *   OK:  {0x03 0x14 0x00 0x00 0x00 0x14}  (projector is on)
 *   ERR: {}  (projector is off)
 *
 * Projector on
 *   data: [6, 20, 0, 4, 0, 52, 17, 0, 0, 93]
 *   OK:  {<0x03> 0x14 0x00 0x00 0x00 0x14}
 *   ERR: {<0x00> 0x14 0x00 0x00 0x00 0x14}  (projector already on)
 *
 * Projector off
 *   data: [6, 20, 0, 4, 0, 52, 17, 1, 0, 94]
 *   OK:  {<0x03> 0x14 0x00 0x00 0x00 0x14}
 *   ERR: {<0x00> 0x00 0x00 0x00 0x00 0x00}  (projector already off)
 *
 * Projector power status
 *   data: [7, 20, 0, 5, 0, 52, 0, 0, 17, 0, 94]
 *   response: {0x05 0x14 0x00 0x03 0x00 0x00 0x00 <0x00> 0x17}  (off)
 *   response: {0x05 0x14 0x00 0x03 0x00 0x00 0x00 <0x01> 0x18}  (on)
 */

#define PROJECTOR_ON_BYTES {0x06,0x14,0x00,0x04,0x00,0x34,0x11,0x00,0x00,0x5D}
#define PROJECTOR_OFF_BYTES {0x06,0x14,0x00,0x04,0x00,0x34,0x11,0x01,0x00,0x5E}
#define PROJECTOR_POWER_STATE_BYTES {0x07,0x14,0x00,0x05,0x00,0x34,0x00,0x00,0x11,0x00,0x5E}

class UartProjectorSwitch : public Component, public Switch {

  private:
    UARTDevice *uart;

  public:
    UartProjectorSwitch(UARTComponent *parent): Switch() {
      uart = new UARTDevice(parent);
    }

    void setup() override { 
      ESP_LOGD("projector", "setup");
      publish_state(read_power_state());
    }

    void write_state(bool state) override {
      ESP_LOGD("projector", "empty_buffer");
      empty_buffer();
      if (state) {
        ESP_LOGD("projector", "projector_turn_on");
        if (projector_turn_on()) { publish_state(true); return; }
      } else {
        ESP_LOGD("projector", "projector_turn_off");
        if (projector_turn_off()) { publish_state(false); return; }
      }
      ESP_LOGD("projector", "empty_buffer");
      empty_buffer();
      ESP_LOGD("projector", "read_power_state");
      publish_state(read_power_state());
      ESP_LOGD("projector", "empty_buffer");
      empty_buffer();
    }

  private:
    bool projector_turn_on() {
      uart->write_array(PROJECTOR_ON_BYTES);
      return expect_ack();
    }

    bool projector_turn_off() {
      uart->write_array(PROJECTOR_OFF_BYTES);
      return expect_ack();
    }

    bool read_power_state() {
      const int expected_size = 9;
      static uint8_t buffer[expected_size];

      uart->write_array(PROJECTOR_POWER_STATE_BYTES);
      
      int bytes_read = 0;
      while (bytes_read < expected_size) {
        bytes_read += read_from_wire(buffer + bytes_read, expected_size - bytes_read);
      }

      if (buffer[0] == 0x0) {
        ESP_LOGD("projector.get", "Read 0, guessing OFF");
        return false;
      } else if (buffer[0] == 0x5) {
        if (buffer[7] == 0x0) {
          ESP_LOGD("projector.get", "Read OFF");
          return false;
        } else {
          ESP_LOGD("projector.get", "Read ON");
          return true;
        }
      }
      ESP_LOGW("projector.get", "Unexpected response");
      return false;
    }

    bool expect_ack() {
      const int expected_size = 6;
      static uint8_t buffer[expected_size];

      int bytes_read = 0;
      while (bytes_read < expected_size) {
        bytes_read += read_from_wire(buffer + bytes_read, expected_size - bytes_read);
      }

      if (buffer[0] == 0x03) {
        ESP_LOGD("projector.ack", "Successful ACK");
        return true;
      } else if (buffer[0] == 0x00) {
        ESP_LOGD("projector.ack", "Failed ACK");
        return false;
      } else {
        ESP_LOGW("projector.ack", "Unexpected response");
        return false;
      }
    }

    void empty_buffer() {
      static uint8_t buffer[32];
      int count = -1;
      while (count != 0) { count = read_from_wire(buffer, 32); }
    }

    int read_from_wire(uint8_t *buffer, const int max_bytes) {
      const int max_line_length = 80;
      static char log_string[max_line_length];

      /* Read bytes */
      int ptr = 0;
      while (uart->available() && ptr < max_bytes) {
        buffer[ptr++] = uart->read();
      }

      /* Format read bytes in hex */
      int log_ptr = snprintf(log_string, max_line_length, "Read");
      for (int i = 0; i < ptr; i++) {
        log_ptr += snprintf(log_string + log_ptr, max_line_length - log_ptr, " 0x%02x", buffer[i]);
      }

      /* Log it and set the sensor state */
      if (ptr > 0) { 
        ESP_LOGD("read_from_wire", "%s", log_string); 
      }
      return ptr;
    }
};
