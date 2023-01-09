/* AddapFilt.c
 * 04.02.22
 * Ryan Colon
 * Least mean squares implementation of the Adaptive filter
 */ 

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#define MAX_DELAY 3200

jack_port_t *input_port;
jack_port_t* ref_port;
jack_port_t* outputPort1;
jack_port_t* outputPort2;
jack_client_t* client;

int M = 40, D = 50;
float delta = 0.001;
int Pass = 0;

int process_samples (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in;
	jack_default_audio_sample_t* out1;
	jack_default_audio_sample_t* out2;

	static float delayLine[MAX_DELAY] = {0.0};
	static float weights[MAX_DELAY] = {0.0};
	
	in = jack_port_get_buffer (input_port, nframes);
	out1 = jack_port_get_buffer (outputPort1, nframes);
	out2 = jack_port_get_buffer (outputPort2, nframes);
	
	//Enforce high limits
	D = (D > MAX_DELAY) ? MAX_DELAY : D;
	M = (M > MAX_DELAY - D) ? (MAX_DELAY - D) : M;
	delta = (delta >= 1) ? 0.99 : delta;
	
	//Enforce low limits
	D = (D <= 0) ? 1 : D;
	M = (M <= 0) ? 1 : M;
	delta = (delta <= 0) ? 0.001 : delta;

	for (int i=0; i < nframes ;i++) {
	
		for(int j = MAX_DELAY-1; j > 0; j--)
			delayLine[j] = delayLine[j-1];
	
		delayLine[0] = in[i];

		float predictor = 0.0;
		for(int j = D-1; j < (D+M); j++)
			 predictor += weights[(j - (D-1))] * delayLine[j];

		float error = in[i] - predictor;
		//printf("Error: %f\n", error);
		
		for(int j = 0; j < M; j++)
			weights[j] += delta*delayLine[D - 1 + j]*error;
			
		out1[i] = (Pass <= 0) ? error : 0;
		out2[i] = (Pass > 0) ? in[i] : 0;
	}
	
	return 0;
}





/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
	exit (EXIT_FAILURE);
}





int main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = "simple-client";   /***** you can change the client name *****/
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

	/* tell the JACK server to call `process_samples()' whenever
	 * there is work to be done.
	 */

	jack_set_process_callback (client, process_samples, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	 * it ever shuts down, either entirely, or if it
	 * just decides to stop calling us.
	 */

	jack_on_shutdown (client, jack_shutdown, 0);

	/* get current sample rate */

        sample_rate = jack_get_sample_rate (client);

        printf ("engine sample rate: %d\n", sample_rate);

	/* create two ports */

	input_port = jack_port_register (client, "input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
	ref_port = jack_port_register   (client, "input2",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 1);					 
	outputPort1 = jack_port_register (client, "output1",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
	outputPort2 = jack_port_register (client, "output2",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 1);

	if ((input_port == NULL) || (ref_port == NULL) || (outputPort1 == NULL) || (outputPort2 == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (EXIT_FAILURE);
	}

	
	/* Tell the JACK server that we are ready to begin.
	 * Our process_samples() callback will start running now.
	 */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (EXIT_FAILURE);
	}

	/* Connect the ports.
	 */

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (EXIT_FAILURE);
	}
	
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (EXIT_FAILURE);
	}

	/* Connect the ports.  */

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (EXIT_FAILURE);
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}
	
	if (jack_connect (client, ports[1], jack_port_name (ref_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (EXIT_FAILURE);
	}

	if (jack_connect (client, jack_port_name (outputPort1), ports[0])) {
		fprintf (stderr, "cannot connect output port 1\n");
	}

	if (jack_connect (client, jack_port_name (outputPort2), ports[1])) {
	fprintf (stderr, "cannot connect output port 2\n");
	}

	free (ports);

	/* it is now running....
         * do whatever needs to be done (if anything) while it is running.
         */

	sleep(1);     /* delay is to let other messages be output before ours */
	fprintf(stderr,"Running ... press CTRL-C to exit ...  \n");
	while (1) { 
		char selection;
		printf("Enter variable to change (M, D, d (delta), p (passthrough), ? (view current params): ");
		if(scanf("%c", &selection));
		
		switch(selection){
			case 'M' :
				printf("Enter the new value of coefficients: ");
				if(scanf("%d", &M));
				break;
			case 'D' :
				printf("Enter the new value of delay: ");
				if(scanf("%d", &D));
				break;
			case 'd' :
				printf("Enter the new value of delta: ");
				if(scanf("%f", &delta));
				break;
			case 'p' :
				printf("Enable/disable(>0, <=0) passthrough: ");
				if(scanf("%d", &Pass));
				break;
			case '?' : 
				printf(" M: %d\n D: %d\n delta: %f\n", M, D, delta);
				break;
		}
		
	}

	jack_client_close (client);
	exit (EXIT_SUCCESS);
}
