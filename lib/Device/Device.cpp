#include "Device.h"

Device::Device(const char* name, DEVICE_TYPE type) {
  _name = String(name);
  _type = type;
}
