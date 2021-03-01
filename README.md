# esphome-viewsonic-pjd7820hd
An ESPHome configuration for power on/off control of a ViewSonic PJD7820HD projector.

## Quick Start
1. Download the repo.
2. In Home Assistant, set up the ESPHome integration.
3. Build the controller (see **Build** below)
4. In your local repo, create `secrets.yaml` with three keys: `wifi_ssid`, `wifi_pass`, and `fallback_pass`. The first two should match your home's wifi network. The third probably doesn't matter, it's just the PSK for the fallback wifi AP that ESPHome uses to connect if your board can't talk to the regular wifi.
5. Plug the D1 into your computer and run `esphome ./projector.yaml run` to do the first flash.
6. Disconnect the D1, plug the thing into the projector, and plug the D1 into a power brick.
7. Once it boots, HomeAssistant should have a `switch.projector_power` that you can add to your setup.
8. Enjoy integrated control of your projector's on/off switch!

## Parts
* A [ViewSonic PJD7820HD](https://www.viewsonic.com/uk/products/projectors/pjd7820hd) projector, obviously.
* A [Wemos D1 Mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) board with an ESP8266. Cheap (around $5) and readily available.
* A [SparkFun RS232 Shifter](https://www.sparkfun.com/products/449) board. A bit more expensive but worth it.
* [HomeAssistant](https://www.home-assistant.io/) with the [ESPhome](https://esphome.io/) integration. (I ran esphome from the command line of my desktop instead of using the HASS add-on.)

## Build
The nice bit of the RS232 Shifter board is that you can drive it with whatever input voltage you want, and it'll do the conversion to RS232 logic levels for you. So you don't need any extra discrete components – the connections are straightforward. Since the D1 drives its GPIO pins at 3.3V, we'll connect the 3V3 output of the D1 to the VCC input of the Shifter. Here's the rest of the pins:

D1 Mini | RS232 Shifter
----|----
3V3 | VCC
G   | GND
TX  | TX
RX  | RX

I don't use the UART logger on the D1, so I wasn't worried about stealing the hardware UART, but you can pick different pins pretty easily.

Now, I made one small mistake, which is that the manual for the PJD7820HD says that you need a null-modem cable to connect the RS232 port to a computer. There are three ways to compensate for this:
1. (Best) Get a damn null-modem cable or dongle to sit between the Shifter board and the projector.
2. (Good) Cross the TX/RX pins between the D1 Mini and the Shifter.
3. (_what_) Swap the TX/RX pins on the UART in your ESPHome config, disabling the hardware UART and forcing it to use bit-blasting.

Guess which one I did? If you catch this sooner than I did, you have the option to do one of the other two, but you'll want to edit the YAML to un-swap the TX/RX pins.

In terms of building out hardware, that's it. You'll need a mini USB cable to both flash the D1 for the first time as well as provide power for it once it's connected to your projector. And maybe you want some sort of case for the D1 and Shifter board combo, depending on how you put them together.
