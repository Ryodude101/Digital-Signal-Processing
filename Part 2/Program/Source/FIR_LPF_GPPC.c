/* Note: this is an example program and is heavily commented - you should remove or shorten some of the comments */

/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 *
 * This is a modified version of the original simple_client.c program.
 * The original was downloaded on 2012-01-14 from:
 *   http://trac.jackaudio.org/wiki/WalkThrough/Dev/SimpleAudioClient
 *   http://trac.jackaudio.org/browser/trunk/jack/example-clients/simple_client.c
 *   (previous versions/downloads: Oct. 2009)
 *
 * This program initializes the sound card and copies the audio samples from the input to the output.
 * It can be modified to create a filter or other DSP application which uses the sound card.
 * For most applications the only changes needed are in the marked areas; the rest of the program
 * should not need to be modified.
 * This version is monophonic. The input is on the left channel and output is on either the
 * left channel or both channels, depending on the sound card driver configuration.
 *
 * On Linux using gcc compile with the '-ljack' option to link with the jack library.
 * If you use math functions, e.g. cos, the '-lm' option is also needed to link with the math library.
 *   E.g., gcc -Wall myprog.c -ljack -lm -o myprog
 * The -Wall enables most warnings and the -o specifies the executable file name.
 *
 * Updated: Jan. 2022
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#define maxCoeff 200

jack_port_t *input_port;
jack_port_t* outputPort1;
jack_port_t* outputPort2;
jack_client_t* client;

/***** INSERT GLOBAL VARIABLE DECLARATIONS HERE *****/
/***** INSERT GLOBAL VARIABLE DECLARATIONS HERE *****/
/***** INSERT GLOBAL VARIABLE DECLARATIONS HERE *****/
int numCoeff = 0;
float coeffArray[maxCoeff];
char choise;

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int process_samples (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in;
	jack_default_audio_sample_t* out1;
	jack_default_audio_sample_t* out2;

	static float pipeline[maxCoeff] = {0.0};

	float y;
	float x;

/***** INSERT LOCAL VARIABLE DECLARATIONS HERE *****/
/***** INSERT LOCAL VARIABLE DECLARATIONS HERE *****/
/***** INSERT LOCAL VARIABLE DECLARATIONS HERE *****/
	
	in = jack_port_get_buffer (input_port, nframes);
	out1 = jack_port_get_buffer (outputPort1, nframes);
	out2 = jack_port_get_buffer (outputPort2, nframes);

	for (int i=0; i < nframes ;i++) {
/***** REPLACE NEXT LINE WITH YOUR ALGORITHM *****/
/***** REPLACE NEXT LINE WITH YOUR ALGORITHM *****/
/***** REPLACE NEXT LINE WITH YOUR ALGORITHM *****/
		x = in[i];

		for(int j = numCoeff - 1; j > 0; j--)
			pipeline[j] = pipeline[j-1];

		pipeline[0] = x;
		y = 0;
		
		for(int j = 0; j < numCoeff; j++)
			y += pipeline[j]*coeffArray[j];

		out1[i] = y;
		out2[i] = (choise == 'y' || choise == 'Y') ? x : 0;
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

/***** INSERT LOCAL VARIABLE DECLARATIONS HERE *****/
/***** INSERT LOCAL VARIABLE DECLARATIONS HERE *****/
/***** INSERT LOCAL VARIABLE DECLARATIONS HERE *****/

	char* preppend = "./Coefficients/";

	char fileAddress[strlen(preppend) + strlen(argv[1])];

	int count = 0;
	while(preppend[count] != '\0'){
		fileAddress[count] = preppend[count];
		++count;
	}
	
	int currCount = count;
	count = 0;
	while(argv[1][count] != '\0'){
		fileAddress[currCount] = argv[1][count];
		++count;
		++currCount;
	}

	fileAddress[currCount] = '\0';

	FILE* coeffFile = fopen(fileAddress, "r");

	if(NULL == coeffFile){
		printf("Couldn't open file, filename:\t%s\n",argv[1]);
		return(0);
	}

	char firstLine[56];
	if(fgets(firstLine, 56, coeffFile) == NULL)
		return(0);
		
	printf("%s\n", firstLine);

	float fileVal = 0;
	while(fscanf(coeffFile, "%f", &fileVal) == 1){
		if(numCoeff >= maxCoeff){
			printf("Too many coefficients, max number of coefficients:\t %d\nExitting Program.\n", maxCoeff);
			return 0;
		}
		coeffArray[numCoeff] = fileVal;
		numCoeff++;
	}	

	for(int i = 0; i < numCoeff; i++)
		printf("Coefficient %d:    \t%f\n", i, coeffArray[i]);
		
	fclose(coeffFile);

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
	outputPort1 = jack_port_register (client, "output1",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
	outputPort2 = jack_port_register (client, "output2",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);

	if ((input_port == NULL) || (outputPort1 == NULL) || (outputPort2 == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (EXIT_FAILURE);
	}

/***** INSERT INITIALIZATION CODE HERE *****/
/***** INSERT INITIALIZATION CODE HERE *****/
/***** INSERT INITIALIZATION CODE HERE *****/
	
	/* Tell the JACK server that we are ready to begin.
	 * Our process_samples() callback will start running now.
	 */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (EXIT_FAILURE);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
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

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

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

/***** (OPTIONAL) ADD TO OR REPLACE THE LINES BELOW WITH OTHER ACTIONS TO DO WHILE ALGORITHM IS RUNNING *****/
/***** (OPTIONAL) ADD TO OR REPLACE THE LINES BELOW WITH OTHER ACTIONS TO DO WHILE ALGORITHM IS RUNNING *****/
/***** (OPTIONAL) ADD TO OR REPLACE THE LINES BELOW WITH OTHER ACTIONS TO DO WHILE ALGORITHM IS RUNNING *****/

	/* Nothing to do in main program ... wait until stopped by the user.
	 * Make a little spinning thing to show the program is running.
	 * Output is to stderr, avoiding line buffering, to make this work.
	 */

	sleep(1);     /* delay is to let other messages be output before ours */
	fprintf(stderr,"Running ... press CTRL-C to exit ...  \n");
	while (1) {
		/*fprintf(stderr,"\b\\");  \b is the backspace character 
		sleep(1);
		fprintf(stderr,"\b|");
		sleep(1);
		fprintf(stderr,"\b/");
		sleep(1);
		fprintf(stderr,"\b-");
		sleep(1);*/
		
		printf("Enter y/n to enable to disable passthrough.\n");
		
		if(scanf("%s", &choise));  
		
	}

	/* This is may or may not be reached, depending on what is directly before it,
	 * but if the program had some other way to exit besides being killed,
	 * they would be important to call.
	 */

	jack_client_close (client);
	exit (EXIT_SUCCESS);
}
