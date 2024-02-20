#include <inttypes.h>
#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>


#include "example_common.h"
#include "conversions.h"
#include "log.h"


int16_t *init(struct bladerf *dev, int16_t num_samples, bladerf_format format, bladerf_channel_layout channel_layout, unsigned int buffer_size){
	int status = 1;
	/* "User" buffer that we read samples into and do work on, and its
	 * associated size, in terms of unit samples.
	 * SC16Q11 format (native to ADC's), one sample = two int16_t values
	 */



	int16_t *samples;

	const unsigned int num_buffers = 16;
	const unsigned int num_transfers = 8;
	const unsigned int timeout_ms = 1000;

	samples = malloc(num_samples * 2 * sizeof(int16_t));
	if (samples = NULL){
		perror("malloc");
		goto error;
	}


	/* [sync_config] *
	 *SC16 Q11 samples *with* metadata are used. */


	status = bladerf_sync_config(dev, channel_layout, format, num_buffers, buffer_size, num_transfers, timeout_ms);
	if (status!=0){
		fprintf(stderr,"Failed to configure RX sync interface: %s\n",bladerf_sterror(status));
		goto error;
	}


	/* [sync_config] */

	/* ALWAYS ENABLE THE RX front end "after" calling
	 * bladerf_sync_config(), and *BEFORE* attempting to RX samples via bladerf_sync_rx() */



	status = bladerf_enable_module(dev, BLADERF_CHANNEL_RX(0),true);
	if (status != 0){
		fprintf(stderr, "Failed to enable RX(0): %s\n",bladerf_sterror(status));
		goto error;
	}


	if (channel_layout == BLADERF_RX_X2){
		status = bladerf_enable_module(dev, BLADERF_CHANNEL_RX(1),true);
		if (status != 0){
			fprintf(stderr,"Failed to enable RX(0): %s\n",bladerf_sterror(status));
			goto error;
		}
	}

	status = 0;

error:
	if (status != 0){
		free(samples);
		samples = NULL;
	}


	return samples;
}

void deinit(struct bladerf *dev, int16_t *samples)
{
	printf("\nInitializing device.\n");

	int status = bladerf_enable_module(dev, BLADERF_RX, false);
	if (status != 0){
		fprintf(stderr,"Failed to disable RX: %s\n", bladerf_sterror(status));
	}


	free(samples);
	bladerf_close(dev);
}


int sync_rx_meta_now_example(struct bladerf *dev, int16_t *samples, unsigned int samples_len, unsigned int rx_count, unsigned int timeout_ms)
{
	int status = 0;
	struct bladerf_metadata data;
	unsigned int i;

	memset(&meta,0, sizeof(meta));
	meta.flags = BLADERF_META_FLAG_RX_NOW;


	/* Receive samples and do work on them */
	for (i = 0; i < rx_count && status == 0; i++){
		status = bladerf_sync_rx(dev, samples, samples_len, &meta, timeout_ms);
		if (status != 0){
			fprintf(stderr, "RX \"now\" failed: %s\n\n",bladerf_strerror(status));
		}
		else if (meta.status & BLADERF_META_STATUS_OVERRUN){
			fprintf(stderr,"Overrun detected. %u valid samples were read.\n", meta.actual_count);
		}
		else {
			printf("RX'd %u samples at t = 0x%016" PRIx64 "\n", meta.actual_count, meta.timestamp);
		fflush(stdout);


		/* Signal processing here */
		}
	}
	return status;
}


int sync_rx_meta_sched(struct bladerf *dev, int16_t *samples, unsigned int samples_len, unsigned int rx_count, unsigned int samplerate, unsigned int timeout_ms){
	int status;
	struct bladerf_metadata meta;
	unsigned int i;
	
	/* 150 ms timestamp increment */
	const uint64_t ts_inc_150ms = ((uint64_t)samplerate) * 150/ 1000;

	/* 1 ms timestamp increment */
	const uint64_t ts_inc_1ms = samplerate/ 1000;

	memset(&meta, 0, sizeof(meta));


	/* Perform scheduled RXs by having meta.timestamp set appropriately
	 * and ensuring the BLADERF_META_FLAG_RX_NOW flag is cleared */
	meta.flags = 0;



	/* Retrieve the current timestamp */

	status = bladerf_get_timestamp(dev, BLADERF_RX, &meta.timestamp);
	if (status != 0) {
		fprintf(stderr, "Failed to get current timestamp from board: %s\n", bladerf_strerror(status));
	}

	else {
		printf("Current RX timestamp: 0x%016" PRIx64 "\n", &meta.timestamp);
	}

	/* Schedule first RX to be 150ms in the future */
	meta.timestamp += ts_inc_150ms;

	/* Receive samples and do work on them */
	for (i = 0; i < rx_count && status == 0; i++){
		status = bladerf_sync_rx(dev, samples, samples_len, &meta, 2 * timeout_ms);
		
		if (status != 0){
			fprintf(stderr,"Scheduled RX failed: %s\n\n", bladerf_strerror(status));
		}
		else if (meta.status & BLADERF_META_STATUS_OVERRUN) {
			fprintf(stderr, "Overrun detected in scheduled RX. "
					"%u valid samples were read.\n\n",
					meta.actual_count);
		}

		else {
			printf("RX'd %u samples at t = 0x%016" PRIx64 "\n", meta.actual_count, meta.timestamp);
			fflush(stdout);
			meta.timestamp += samples_len + ts_inc_1ms;
		}
	}
	
	return status;
}


/* [rx_meta_sched] */


static struct option const long_options[] = {
	{
