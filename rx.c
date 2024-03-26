/* rx with metadata */
#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>

// ---------------------------------- some sys stuff ---------------------------------

// ---------------------------------- some basic signal definition ------------------------------
#define SAMPLE_PER_SYMBOL 4 //4Msps sample rate
volatile int rx_buff_offset;
#define LEN_BUF_IN_SAMPLE (4*4096)
#define LEN_BUF (LEN_BUF_IN_SAMPLE * 2)
#define LEN_BUF_IN_SYMBOL (LEN_BUF_IN_SAMPLE/SAMPLE_PER_SYMBOL)

// ----------------------------------- bladeRF specific operation -------------------------------------
char *boardname = "bladeRF";
#define MAX_GAIN = 60
#define DEFAULT_GAIN 45

typedef struct bladerf_devinfo bladerf_devinfo;
typedef struct bladerf bladerf_device;
typedef int16_t IQ_TYPE;

struct bladerf_async_task {
  pthread_t rx_task;
  pthread_mutex_t stderr_lock;
};

struct bladerf_data {
  void                  **buffers;      /* Transmit Buffers */
  size_t                num_buffers;     /* Number of buffers */
  size_t                samples_per_buffer;    /* Samples per buffer */
  unsigned int          idx;              /* Next sample that needs to go out */
  volatile IQ_TYPE      *rx_buf;
};


struct bladerf_stream *stream;
struct bladerf_async_task async_task;
struct bladerf_data rx_data;


void *stream_callback(struct bladerf *dev, struct bladerf_stream *stream,
                      struct bladerf_metadata *metadata, void *samples,
                      size_t num_samples, void *user_data)
{
  struct bladerf_data *my_data = (struct bladerf_data *)user_data;
  size_t i;
  int16_t *sample = (int16_t *)samples;
  for (i = 0; i < num_samples; i++){
    rx_buf[rx_buff_offset] = (((*sample) >> 4) & 0xFF);
    rx_buff[rx_buf_offset+1] = (((*(sample+1)) >>  4) & 0xFF);
    rx_buff_offset = (rx_buff_offset + 2) & (LEN_BUF - 1);
    sample += 2;
  }

  if (do_exit) {
    return NULL;
  }
  else {
    void *rv = my_data->buffers[my_data->idx];
    my_data->idx = (my_data->idx + 1) % my_data -> num_buffers;
    return rv;
  }
}


struct channel_config {
bladerf_channel channel;
unsigned int frequency;
unsigned int bandwidth;
unsigned int samplerate;
int gain;
};


const unsigned samples_len = 10000;
int16_t *rx_samples = NULL;
const unsigned int buffer_size = 8192;
const unsigned int num_buffers = 16;
const unsigned int num_transfers = 8;
const unsigned int timeout_ms = 3500;
const unsigned int samplerate = 2.4e6;
static int init_sync_rx(struct bladerf *dev){
  int status;

  status = bladerf_sync_config(dev, BLADERF_RX_X2, BLADERF_FORMAT_SC16_Q11, num_buffers, buffer_size, num_transfers, timeout_ms);
  if (status != 0){
    fprintf(stderr, "Failed to configure RX sync interface: %s\n", bladerf_strerror(status));
    return status;    
  }
  else {
    printf("Successfully configured RX sync interface!\n");
  
  return status;
}
}

int sync_rx_meta_schedule(struct bladerf *dev, int16_t *samples,
                          unsigned int samples_len,
                          unsigned int rx_count,
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
  sync_rx_meta_schedule(dev, rx_samples, samples_len,
                        num_transfers, timeout_ms);
}




