// adapted from https://github.com/signal11/hidapi/blob/master/README.txt

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "hidapi.h"

int main(int argc, char* argv[])
{
  int res;

  // Initialize the hidapi library
  res = hid_init();

  struct hid_device_info *devs, *cur_dev;
  devs = hid_enumerate(0x0, 0x0);

  // Print all attached devices, for reference
  cur_dev = devs;
  while (cur_dev) {
    printf("Device Found\n"
           "  type: %04hx %04hx\n"
           "  path: %s\n"
           "  serial_number: %ls\n"
           "  manufacturer: %ls\n"
           "  product: %ls\n"
           "  usage page: %04hx\n"
           "  usage: %04hx\n\n",
           cur_dev->vendor_id, cur_dev->product_id, cur_dev->path,
           cur_dev->serial_number, cur_dev->manufacturer_string,
           cur_dev->product_string, cur_dev->usage_page, cur_dev->usage);
    cur_dev = cur_dev->next;
  }

  // Yubikey U2F
  //int target_vendor_id = 0x1050;
  //int target_product_id = 0x0407; // Previous generation was 0x0116
  int target_usage_page = 0xf1d0;
  int target_usage = 001;

  // Find the Yubikey
  // Can't do it by serial number because it's not available on Mac :/
  // https://github.com/signal11/hidapi/issues/257
  cur_dev = devs;
  while (cur_dev) {
    if (//cur_dev->vendor_id == target_vendor_id &&
        //cur_dev->product_id == target_product_id &&
        cur_dev->usage_page == target_usage_page &&
        cur_dev->usage == target_usage) {
      fprintf(stderr, "Found target device\n");
      break;
    }
    cur_dev = cur_dev->next;
  }

  if (!cur_dev) {
    fprintf(stderr, "!!  Did not find target device\n");
    return 1;
  }

  // Verify that this is the target device
  fprintf(stderr, "Manufacturer: %ls\nProduct: %ls\n",
          cur_dev->manufacturer_string, cur_dev->product_string);

  // Open the device
  hid_device *yubikey;
  yubikey = hid_open_path(cur_dev->path);

  if (!yubikey) {
    fprintf(stderr, "Failed to open device (possible permission error?)\n");
    return 1;
  } else {
    fprintf(stderr, "Successfully opened device\n");
  }

  // Now that we're done with the device info from hid_enumerate, we can free
  // the enumeration.
  hid_free_enumeration(devs);
  hid_close(yubikey);
  hid_exit();

  return 0;
}

