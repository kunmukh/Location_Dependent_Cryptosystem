
//Programmer: Kunal Mukherjee                       # Date completed:

// clang Audio_extraction.c -o driver 

//adding the header files
#include <stdio.h>
#include <math.h>
#include <stdint.h>


int main(int argc, char const *argv[])
{	 

    //open the respective files // 44.1 * 15100
    FILE * inputAudiofile;   
    inputAudiofile  = popen("ffmpeg -i input.wav -hide_banner -f s16le -ac 1 -", "r");    

    FILE * tempAudiosampleFile;
    tempAudiosampleFile = fopen("tempAud.txt","w");
     
    // Read, modify and write one sample at a time
    int16_t audioSample;        

    while( fread(&audioSample, 2, 1, inputAudiofile)) // read one 2-byte sample
    {   
        fprintf(tempAudiosampleFile, "%d\n", audioSample);       
    }
 
    pclose(inputAudiofile); 
    fclose(tempAudiosampleFile);

    //opening a temp file to put the values in
    FILE * outputAudiofile;
    outputAudiofile = popen("ffmpeg -y -f s16le -ar 44100 -ac 1 -i - output.wav -hide_banner", "w");

    FILE * tempAudiosampleFileInput;
    tempAudiosampleFileInput = fopen("tempAud.txt","r");

    int16_t audioSampleRead;

    while(fscanf(tempAudiosampleFileInput, "%hd" , &audioSampleRead) != EOF)
    {
        fwrite(&audioSampleRead, 2, 1, outputAudiofile);
    }
     
    // Close input and output pipes       
    pclose(outputAudiofile);    
    fclose(tempAudiosampleFileInput);    

    return 0;
}	

