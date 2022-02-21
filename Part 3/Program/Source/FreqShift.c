/* 	FreqShift.C
	Ryan Colon
	02.14.22
	Program takes an input signal from mic (left channel) and a user input frequency shift to shift the frequency of the input
	signal and outputs it to stereo (left channel). Original signal is output stereo (channel right) */	

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <jack/jack.h>

#define maxCoeff 2000

jack_port_t* inPort;
jack_port_t* outPort;
jack_client_t* client;

int numCoeff = 0;
float coeffArray[maxCoeff];
float frequencyShiftInput = 0;
float delTheta = 0;
float thisTheta = 0;

//Function is called whenever audio samples become available from the soundcard
int process_samples (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t* in;
	jack_default_audio_sample_t* out1;

	static float pipeline[maxCoeff] = {0.0};

	float hilbTrans;
	
	in = jack_port_get_buffer (inPort, nframes);
	out1 = jack_port_get_buffer (outPort, nframes);

	for (int i=0; i < nframes ;i++) {

		for(int j = numCoeff - 1; j > 0; j--)
			pipeline[j] = pipeline[j-1];

		pipeline[0] = in[i];
		hilbTrans = 0;
		
		for(int j = 0; j < numCoeff; j++)
			hilbTrans += pipeline[j]*coeffArray[j];


		thisTheta = (thisTheta > 2*M_PI) ? (thisTheta - (2*M_PI)) : thisTheta;
		thisTheta = (thisTheta < (-2*M_PI)) ? (thisTheta + (2*M_PI)) : thisTheta;
		
		out1[i] = ((pipeline[(int)floor(numCoeff/2)]*cos(thisTheta)) - (hilbTrans*sin(thisTheta)));
		thisTheta += delTheta;
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
	const char *client_name = "Frequency-Shift-Client";   /***** you can change the client name *****/
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	int sample_rate;

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

	char firstLine[200];
	if(fgets(firstLine, 200, coeffFile) == NULL)
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

	jack_set_process_callback (client, process_samples, 0);

	jack_on_shutdown (client, jack_shutdown, 0);

	/* get current sample rate */

        sample_rate = jack_get_sample_rate (client);

        printf ("engine sample rate: %d\n", sample_rate);

	/* create two ports */

	inPort = jack_port_register (client, "input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
	outPort = jack_port_register (client, "output1",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
					  

	if ((inPort == NULL) || (outPort == NULL)) {
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

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (EXIT_FAILURE);
	}

	if (jack_connect (client, ports[0], jack_port_name (inPort))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);
	
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (EXIT_FAILURE);
	}

	if (jack_connect (client, jack_port_name (outPort), ports[0])) {
		fprintf (stderr, "cannot connect output port 1\n");
	}

	free (ports);

	sleep(1);     /* delay is to let other messages be output before ours */
	fprintf(stderr,"Running ... press CTRL-C to exit ...  \n");
	while (1) {
		printf("Enter desired frequency shift in Hz: ");
		
		if(scanf("%f", &frequencyShiftInput)); 
		delTheta = 2*M_PI*frequencyShiftInput/sample_rate;
	}

	jack_client_close (client);
	exit (EXIT_SUCCESS);
}
