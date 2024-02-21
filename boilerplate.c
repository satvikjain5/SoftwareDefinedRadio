/* Save to a file, e.g. boilerplate.c, and then compile:
 * $ gcc boilerplate.c -o libbladeRF_example_boilerplate -lbladeRF
 */
 
#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
/* The RX and TX channels are configured independently for these parameters */
struct channel_config {
 bladerf_channel channel;
 unsigned int frequency;
 unsigned int bandwidth;
 unsigned int samplerate;
 int gain;
};


static int init_sync(struct bladerf *dev){
 int status;
 const unsigned int num_buffers = 16;
 const unsigned int buffer_size = 8192;
 const unsigned int num_transfer = 8;
 const unsigned int timeout_ms = 3500;


 status = bladerf_sync_config(dev,BLADERF_RX_X2,BLADERF_FORMAT_SC16_Q11, num_buffers, buffer_size, num_transfers, stream_timeout);
 if (status != 0){
  fprintf(stderr, "Failed to configure RX sync interface: %s\n", bladerf_strerror(status));
  return status;
 }
 return status;
}

int sync_rx_samples(struct bladerf *dev){
 int status, ret;
 bool done = false;
 bool have_tx_data = false;

 /* One sample = two int16_t values */
 int16_t *rx_samples = NULL;
 const unsigned int samples_len = 10000;

 /* Allocate the buffer to store the samples */
 rx_samples = malloc(samples_len * 2 * 1 * sizeof(int16_t));
 if (rx_samples == NULL){
  perror("malloc");
  return BLADERF_ERR_MEM;
 }


}

int configure_channel(struct bladerf *dev, struct channel_config *c)
{
 int status;
 
 status = bladerf_set_frequency(dev, c->channel, c->frequency);
 if (status != 0) {
 fprintf(stderr, "Failed to set frequency = %u: %s\n", c->frequency,
 bladerf_strerror(status));
 return status;
 }
 
 status = bladerf_set_sample_rate(dev, c->channel, c->samplerate, NULL);
 if (status != 0) {
 fprintf(stderr, "Failed to set samplerate = %u: %s\n", c->samplerate,
 bladerf_strerror(status));
 return status;
 }
 
 status = bladerf_set_bandwidth(dev, c->channel, c->bandwidth, NULL);
 if (status != 0) {
 fprintf(stderr, "Failed to set bandwidth = %u: %s\n", c->bandwidth,
 bladerf_strerror(status));
 return status;
 }
 
 status = bladerf_set_gain(dev, c->channel, c->gain);
 if (status != 0) {
 fprintf(stderr, "Failed to set gain: %s\n", bladerf_strerror(status));
 return status;
 }
 
 return status;
}
 
/* Usage:
 * libbladeRF_example_boilerplate [serial #]
 *
 * If a serial number is supplied, the program will attempt to open the
 * device with the provided serial number.
 *
 * Otherwise, the first available device will be used.
 */
int main(int argc, char *argv[])
{
 int status;
 struct channel_config config;
 
 struct bladerf *dev = NULL;
 struct bladerf_devinfo dev_info;
 
 /* Initialize the information used to identify the desired device
 * to all wildcard (i.e., "any device") values */
 bladerf_init_devinfo(&dev_info);
 
 /* Request a device with the provided serial number.
 * Invalid strings should simply fail to match a device. */
 if (argc >= 2) {
 strncpy(dev_info.serial, argv[1], sizeof(dev_info.serial) - 1);
 }
 
 status = bladerf_open_with_devinfo(&dev, &dev_info);
 if (status != 0) {
 fprintf(stderr, "Unable to open device: %s\n",
 bladerf_strerror(status));
 
 return 1;
 }
 
 /* Set up RX channel parameters */
 config.channel = BLADERF_CHANNEL_RX(0);
 config.frequency = 910000000;
 config.bandwidth = 2000000;
 config.samplerate = 2.4e6;
 config.gain = 39;
 
 status = configure_channel(dev, &config);
 if (status != 0) {
 fprintf(stderr, "Failed to configure RX channel. Exiting.\n");
 goto out;
 }
 
 /* Set up TX channel parameters */
 config.channel = BLADERF_CHANNEL_TX(0);
 config.frequency = 918000000;
 config.bandwidth = 1500000;
 config.samplerate = 2.4e6;
 config.gain = -14;
 
 status = configure_channel(dev, &config);
 if (status != 0) {
 fprintf(stderr, "Failed to configure TX channel. Exiting.\n");
 goto out;
 }
 
 /* Application code goes here.
 */
 printf("Hello world\n");
 
out:
 bladerf_close(dev);
 return status;
}

