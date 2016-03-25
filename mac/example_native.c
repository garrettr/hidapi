#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <CoreFoundation/CoreFoundation.h>

// from https://developer.apple.com/library/mac/documentation/DeviceDrivers/Conceptual/HID/new_api_10_5/tn2187.html
// function to create matching dictionary
static CFMutableDictionaryRef hu_CreateDeviceMatchingDictionary(UInt32 inUsagePage, UInt32 inUsage)
{
    // create a dictionary to add usage page/usages to
    CFMutableDictionaryRef result = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (result) {
        if (inUsagePage) {
            // Add key for device type to refine the matching dictionary.
            CFNumberRef pageCFNumberRef = CFNumberCreate(
                            kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
            if (pageCFNumberRef) {
                CFDictionarySetValue(result,
                        CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
                CFRelease(pageCFNumberRef);
 
                // note: the usage is only valid if the usage page is also defined
                if (inUsage) {
                    CFNumberRef usageCFNumberRef = CFNumberCreate(
                                    kCFAllocatorDefault, kCFNumberIntType, &inUsage);
                    if (usageCFNumberRef) {
                        CFDictionarySetValue(result,
                            CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
                        CFRelease(usageCFNumberRef);
                    } else {
                        fprintf(stderr, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
                    }
                }
            } else {
                fprintf(stderr, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
            }
        }
    } else {
        fprintf(stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
    }
    return result;
}   // hu_CreateDeviceMatchingDictionary


int main(int argc, char* argv[])
{
  IOHIDManagerRef hid_mgr = NULL;
  hid_mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
  CFDictionaryRef matchU2f = hu_CreateDeviceMatchingDictionary(0xf1d0, 0x0001);
  IOHIDManagerSetDeviceMatching(hid_mgr, matchU2f);
  IOHIDManagerScheduleWithRunLoop(hid_mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

  CFSetRef device_set = IOHIDManagerCopyDevices(hid_mgr);
  if (!device_set) {
    fprintf(stderr, "Target device not found\n");
    return 1;
  }

  CFIndex num_devices = CFSetGetCount(device_set);
  IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
  CFSetGetValues(device_set, (const void **) device_array);

  // Mirroring the logic in HIDAPI's hid_open_path
  for (int i = 0; i < num_devices; i++) {
    IOHIDDeviceRef os_dev = device_array[i];
    IOReturn ret = IOHIDDeviceOpen(os_dev, kIOHIDOptionsTypeSeizeDevice);
    if (ret == kIOReturnSuccess) {
      fprintf(stderr, "kIOReturnSuccess\n");
    } else if (ret == kIOReturnNotPrivileged) {
      fprintf(stderr, "kIOReturnNotPrivileged\n");
    } else {
      fprintf(stderr, "IOHIDDeviceOpen returned an unexpected error code: %d\n", ret);
    }
  }

  IOHIDManagerClose(hid_mgr, kIOHIDOptionsTypeNone);
  CFRelease(hid_mgr);
}
