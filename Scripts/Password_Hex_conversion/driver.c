#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARACTER_SIZE 16

void decToHexa(int n, char * hexVal) ;
int hexadecimalToDecimal(char hexVal[]) ;


int main(int argc, char const *argv[])
{
	printf("Password: %s\n", argv[1]);

	char * key = calloc(1, MAX_CHARACTER_SIZE);  
    strncpy(key, argv[1], MAX_CHARACTER_SIZE);

    int ASCII [MAX_CHARACTER_SIZE] = {'0'};

    printf("ASCII value of password: ");

    for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
    {
    	ASCII[i] = key[i];
    	printf("%d ", ASCII[i]);
    }  
    printf("\n");
    
    printf("HEX value of password: ");    
    char tempVal[2];
	char HEX[MAX_CHARACTER_SIZE][2];

    for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
    {
    	decToHexa(ASCII[i], tempVal);     	
    	strcpy(HEX[i], tempVal);

    	tempVal[0] = ' '; tempVal[1] = ' ';
    }
    printf("\n");  
    
    /*printf("HEX value of the password: \n"); 

    for (int i = 0; i < MAX_CHARACTER_SIZE * 2; i+=2)
    {
    	printf("%c%c ", HEX[0][i], HEX[0][i+1]);
    }    	
   
    printf("\n");*/

    printf("ASCII value from Hex value: ");  

    int DEC_from_HEX [MAX_CHARACTER_SIZE];
    int index = 0;
    char convTemp [2];

    for (int i = 0; i < MAX_CHARACTER_SIZE * 2; i += 2)
    {
    	convTemp[0] = HEX[0][i];
    	convTemp[1] = HEX[0][i+1];
    	DEC_from_HEX[index] =   hexadecimalToDecimal(convTemp);
		printf("%d ",DEC_from_HEX[index]);
		index++;
    }  
    printf("\n");

    printf("Password from the ASCII value: ");
    char * ASCII_from_DEC = calloc(1, MAX_CHARACTER_SIZE + 1);

    for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
    {
    	ASCII_from_DEC[i] = DEC_from_HEX[i];
    }
    ASCII_from_DEC[MAX_CHARACTER_SIZE] = '\0';

    printf("%s\n", ASCII_from_DEC);

	return 0;    


}

// C program to convert a decimal 
// number to hexadecimal number
// function to convert decimal to hexadecimal 
void decToHexa(int n, char * hexVal) 
{ 
	// counter for hexadecimal number array 
	int i = 0; 
	char hexaDeciNum[MAX_CHARACTER_SIZE];

	while(n!=0) 
	{ 
		// temporary variable to store remainder 
		int temp = 0; 
		
		// storing remainder in temp variable. 
		temp = n % 16; 
		
		// check if temp < 10 
		if(temp < 10) 
		{ 
			hexaDeciNum[i] = temp + 48; 
			i++; 
		} 
		else
		{ 
			hexaDeciNum[i] = temp + 55; 
			i++; 
		} 
		
		n = n/16; 
	} 	

	
	int indexTempVal = 0;
	
	// printing hexadecimal number array in reverse order 
	for(int j=i-1; j>=0; j--) 
	{
		printf("%c", hexaDeciNum[j]);
		hexVal[indexTempVal] = hexaDeciNum[j];
		indexTempVal++;
	}
	printf(" ");
	
} 

// Function to convert hexadecimal to decimal 
int hexadecimalToDecimal(char hexVal[]) 
{    
    int len = strlen(hexVal); 
      
    // Initializing base value to 1, i.e 16^0 
    int base = 1; 
      
    int dec_val = 0; 
      
    // Extracting characters as digits from last character 
    for (int i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (hexVal[i]>='0' && hexVal[i]<='9') 
        { 
            dec_val += (hexVal[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 16; 
        } 
  
        // if character lies in 'A'-'F' , converting  
        // it to integral 10 - 15 by subtracting 55  
        // from ASCII value 
        else if (hexVal[i]>='A' && hexVal[i]<='F') 
        { 
            dec_val += (hexVal[i] - 55)*base; 
          
            // incrementing base by power 
            base = base*16; 
        } 
    } 
      
    return dec_val; 
} 




