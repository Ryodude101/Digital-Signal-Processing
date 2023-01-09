/* Single Source DOA GPPC
 * 03.11.22
 * Ryan Colon
 * This program is the implementation of single source DOA estimation as I understand it
 * from the Despoina paper. This will be done on a general purpose PC first and then I'll 
 * try to translate it to blackfin. I'm doing it this way now because I don't yet fully
 * understand how to do blackfin programming.
 */

//TO-DO:
//Find a library to do the FFT
//Figure a way to display the DOA

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include "Single_Source_DOA_GPPC.h"

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

//Function get's called back by JACK every time a sample frame is available
//I think I'm going to do the FFT on the entire frame stack every time it's ready
//Surely the computer is fast enough to be able to do this
int process_samples (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in;
	jack_default_audio_sample_t *out;
	int i;
	float x;
	float y;
	
	in = jack_port_get_buffer (input_port, nframes);
	out = jack_port_get_buffer (output_port, nframes);

	for (i=0;i<(int)nframes;i++) {
		x=in[i];
		y=x;

		out[i]=y;
	}

	return 0;      
}

void jack_shutdown (void *arg)
{
	exit (EXIT_FAILURE);
}

int main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = "DOA-Estimation"; 
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;

        int sample_rate;

	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (EXIT_FAILURE);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	//declares process samples as the callback
	jack_set_process_callback (client, process_samples, 0);

	//declares the function for jack to shutdown
	jack_on_shutdown (client, jack_shutdown, 0);

	/* get current sample rate */

        sample_rate = jack_get_sample_rate (client);

        printf ("engine sample rate: %d\n", sample_rate);

	/* create two ports */

	input_port = jack_port_register (client, "input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
	output_port = jack_port_register (client, "output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);

	if ((input_port == NULL) || (output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (EXIT_FAILURE);
	}
	
	//Begins running the jack client, process samples is active now
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (EXIT_FAILURE);
	}

	//Grab all the ports we're going to use
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (EXIT_FAILURE);
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (EXIT_FAILURE);
	}

	if (jack_connect (client, jack_port_name (output_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free (ports);
	//Jack is free running now

	//Main loop, do whatever here
	sleep(1);     /* delay is to let other messages be output before ours */
	fprintf(stderr,"Running ... press CTRL-C to exit ...  ");
	while (1) {
		fprintf(stderr,"\b\\");          
		sleep(1);
		fprintf(stderr,"\b|");
		sleep(1);
		fprintf(stderr,"\b/");
		sleep(1);
		fprintf(stderr,"\b-");
		sleep(1);
	}

	//This may not be reached, but this needs to happen for the program to exit if it does
	jack_client_close (client);
	exit (EXIT_SUCCESS);
}
