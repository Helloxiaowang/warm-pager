#ifndef TEST_BLE_DEVICE_H
#define TEST_BLE_DEVICE_H

namespace FirmwareStub {
extern unsigned int advertisingStarts;
}

class BLEDevice {
public:
    static void startAdvertising() { ++FirmwareStub::advertisingStarts; }
};

#endif
