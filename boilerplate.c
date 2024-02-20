// gcc boilerplate.c -o boilerplate -lbladeRF

#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stuct to configure TX and RX channels independently */
struct channel_config {
  bladerf_channel channel;
  unsigned int frequency;
  unsigned int bandwidth;
  unsigned int samplerate;
  int gain;
};

int configure_channel(struct bladerf *dev, struct channel_config *c)
{
  int status;
  status = bladerf_set_frequency(dev, c->channel, c->frequency);
  if (status != 0){
    fprintf(stderr,"Failed to set frequency = %u: %s\n", c->channel,c->frequency,bladerf_strerror(status));
    return status;
  }

  status = bladerf_set_sample_rate(dev, c->channel, c->frequency,NULL);
  if (status != 0){
    fprintf(stderr,"Failed to set channel sample rate = %u: %s\n",c->channel, c->frequency, bladerf_strerror(status));
    return status;
  }

  status = bladerf_set_bandwidth(dev, c->channel, c->bandwidth, NULL);
  if (status != 0){
    fprintf(stderr,"Failed to set bandwidth = %u: %s\n",c->bandwidth, bladerf_strerror(status));
    return status;
  }

  status = bladerf_set_gain(dev, c->channel, c->gain);
  if (status != 0){
    fprintf(stderr,"Failed to set gain = %s\n",bladerf_strerror(status));
    return status;
  }
  return status;
}


/* Usage: 
* If a serial number is supplied, the program will attempt to
* open the device with the provided serial number.
* Otherwise, the first available device will be used
*/

int main(int argc, char *argv[]){
  int status;
  struct channel_config config;

  struct bladerf *dev = NULL;
  struct bladerf_devinfo dev_info;

  bladerf_init_devinfo(&dev_info);
}

