import sys
from threading import Thread

import adafruit_rfm69
import board
import busio
import hassapi as hass
from digitalio import DigitalInOut


#
# Villa Gateway App
#
# Args:
#

# noinspection PyAttributeOutsideInit
class VillaGateway(hass.Hass):

    def initialize(self):
        self.log("Initializing…")

        # Transform encryption key (coming as a space separated hex string)
        key_string = self.args['radio_encryption_key']
        self.radio_key = bytes.fromhex(key_string)

        # Create worker thread
        self.t = Thread(target=self.worker)
        self.t.daemon = True
        self.t.start()

        # Prepare entities
        self.prepare_entities()

        self.log("Initialize done!")

    def worker(self):
        self.log("Starting worker")
        cs = DigitalInOut(board.CE1)
        reset = DigitalInOut(board.D25)
        spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
        radio = adafruit_rfm69.RFM69(spi, cs, reset, 868.0)
        radio.encryption_key = self.radio_key
        radio.node = 1

        while getattr(self.t, 'do_run', True):
            try:
                packet = radio.receive(with_ack=True, with_header=True, keep_listening=True, timeout=1.0)
                if packet is not None:
                    rssi = radio.last_rssi
                    sender = int(packet[1])
                    packet_text = packet[4:].replace(b'\x00', b'').decode('ascii')
                    self.log(f"From: {sender}\t {packet_text}\t ({rssi} dB)")
                    packet_parts = packet_text.split(",")
                    packet_parts.append(rssi)

                    sensor_ids = self.DEVICE_SENSORS[sender]
                    for i, content in enumerate(sensor_ids):
                        sensor_id, increment = content
                        attributes = self.SENSOR_ATTRIBUTES[sensor_id]
                        if increment:
                            value = self.safe_float(self.get_state(sensor_id)) + float(packet_parts[i])
                        else:
                            value = self.safe_float((packet_parts[i]))

                        self.set_state(sensor_id, state=value, attributes=attributes)
            except Exception:
                self.log(f"Error: {sys.exc_info()}")

        self.log("Stopping worker")
        radio.sleep()

    def terminate(self):
        self.log("Terminating…")
        self.t.do_run = False
        self.t.join()
        self.log("Terminate done!")

    def prepare_entities(self):
        for sensor_id, attributes in self.SENSOR_ATTRIBUTES.items():
            self.set_state(sensor_id, state='unavailable', attributes=attributes)

    @staticmethod
    def safe_float(s):
        try:
            i = float(s)
        except ValueError:
            i = 0.0
        return i

    SENSOR_ATTRIBUTES = {
        'sensor.villalinky_wh_reading': {
            'friendly_name': "Compteur d'électricité",
            'device_class': 'energy',
            'unit_of_measurement': 'Wh',
            'state_class': 'total_increasing'
        },
        'sensor.villalinky_apparent_power': {
            'friendly_name': "Puissance apparente",
            'device_class': 'apparent_power',
            'unit_of_measurement': 'VA',
            'state_class': 'measurement'
        },
        'sensor.villalinky_send_errors': {
            'friendly_name': "Erreurs de transmission du VillaLinky",
            'icon': 'mdi:network-strength-outline',
            'state_class': 'total'
        },
        'sensor.villalinky_signal_strength': {
            'friendly_name': "Force du signal VillaLinky",
            'device_class': 'signal_strength',
            'unit_of_measurement': 'dB',
            'state_class': 'measurement'
        },
        'sensor.villalinky_transmit_power': {
            'friendly_name': "Puissance de transmission du VillaLinky",
            'icon': 'mdi:antenna',
            'state_class': 'measurement'
        }
    }

    DEVICE_SENSORS = {
        2: [
            ('sensor.villalinky_wh_reading', False),
            ('sensor.villalinky_apparent_power', False),
            ('sensor.villalinky_send_errors', True),
            ('sensor.villalinky_transmit_power', False),
            ('sensor.villalinky_signal_strength', False)
        ]
    }
