# Villa-Gateway

This project contains the source code to be [sparse-checkouted](https://www.git-scm.com/docs/git-sparse-checkout) on a Raspberry Pi unit with [a radio bonnet](https://learn.adafruit.com/adafruit-radio-bonnets).
The goal of this Gateway is to receive the messages sent by the various custom radio sensors, then parse and commit the data in the house's HomeAssistant instance.

The project relies on [AppDaemon](https://appdaemon.readthedocs.io/en/latest/) for the python execution environment and sync with HomeAssistant.
It uses [Adafruit's python RFM69 library](https://docs.circuitpython.org/projects/rfm69/en/latest/) to control the radio chip and receive messages.