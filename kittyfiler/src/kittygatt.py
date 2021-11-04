#! /usr/bin/python3

from gattlib import DiscoveryService
from gattlib import GATTRequester, GATTResponse

service = DiscoveryService("hci0")
devices = service.discovery(2)

for address, name in devices.items():
    print("name: {}, address: {}".format(name, address))

class NotifyYourValue(GATTResponse):
    def on_response(self, value):
        print("value: {}".format(value))

response = NotifyYourValue()
req = GATTRequester("0c:38:f8:32:3c:55")
req.read_by_uuid_async("19B10000-E8F2-537E-4C6C-D104768A1215", response)

while True:
    sleep(1)
