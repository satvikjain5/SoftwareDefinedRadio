/* rx with metadata */

#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct channel_config {
bladerf_channel channel;
unsigned int frequency;
unsigned int bandwidth;
unsigned int samplerate;
int gain;
};

static int init_sync_rx(struct bladerf *dev){
  int status;
  const unsigned int num_buffers = 16;
  const unsigned int buffer_size = 8192;
  const unsigned num_transfers = 8;
  const unsigned int timeout_ms = 3500;

  status = bladerf_sync_config(dev, BLADERF_RX_X2, BLADERF_FORMAT_SC16_Q11, num_buffers, buffer_size, num_transfers, timeout_ms);
  if (status != 0){
    fprintf(stderr, "Failed to configure TX sync interface: %s\n", bladerf_strerror(status));
    return status;    
  }
  else {
    printf("Successfully configured TX sync interface!\n");
  }
  return status;
}

int sync_rx_meta_schedule(struct bladerf *dev, int16_t *samples,
                          unsigned int samples_len,
                          unsigned int rx_count,
                          unsigned int samplerate,
                          unsigned int timeout_ms){
  int status;
  struct bladerf_metadata meta;
  unsigned int i;

  /*150 ms timestamp increment */
  const uint64_t ts_inc_150ms = ((uint64_t)samplerate) * 150 / 1000;

  /*1 ms timestamp increment */
  const uint64_t ts_inc_1ms = samplerate / 1000;
  memset(&meta, 0, sizeof(meta));

  /* Perform scheduled RXs by having the meta.timestamp set appropriately
  * and ensuring the BLADERF_META_FLAG_RX_NOW flag is cleared */
  meta.flags = 0;

  /* Retrieve the current timestamp */
  status = bladerf_get_timestamp(dev, BLADERF_RX, &meta.timestamp);
  if (status != 0){
    fprintf(stderr, "Failed to get current RX timestamp: %s\n",bladerf_strerror(status));
  }
  else{
    printf("Current RX timestamp: 0x%016" PRIx64 "\n", meta.timestamp);
  }

  /* Schedule first RX to be 150ms in the future */
  meta.timestamp += ts_inc_150ms;

  /* Receive samples and do work on them */
  for (i = 0; i < rx_count && status == 0; i++){
    status = bladerf_sync_rx(dev, samples, samples_len, &meta, 2 * timeout_ms);

    if (status != 0){
      fprintf(stderr, "Scheduled RX failed: %s\n\n", bladerf_strerror(status));
    }
    else if (meta.status & BLADERF_META_STATUS_OVERRUN){
      fprintf(stderr, "Overrun detected in scheduled RX. ""%u valid samples were read.\n\n",meta.actual_count);
    }
    else {
      printf("RX'd %u samples at t = 0x%016" PRIx64 "\n", meta.actual_count,meta.timestamp);
      fflush(stdout);
      meta.timestamp += samples_len + ts_inc_1ms;
    }
    }
  return status;
  }



int main(int argc, char* argv[])
{
  int status;
  struct channel_config config;
  struct bladerf *dev = NULL;
  struct bladerf_devinfo dev_info;

  bladerf_init_devinfo(&dev_info);

  if (argc >= 2) {
    strncpy(dev_info.serial, argv[1], sizeof(dev_info.serial) - 1);
  }

  status = bladerf_open_with_devinfo(&dev, &dev_info);
  if (status != 0){
    fprintf(stderr, "Unable to open device: %s\n", bladerf_strerror(status));
    return 1;
  }
  init_sync_rx(dev);

}




