#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>
#include "terminated.h"

#define PIRPIN  17
#define REDPIN  18
#define BLUEPIN 24
#define BUZZERPIN  22
#define TEST_BUZZER_TIME 2000
#define MAX_PASSWORD_LEN 10
#define KEY 0xFACA

enum Alarm_state {
 ON,
 OFF
};


int introductionMenu();
int printMainMenu();
int getAuthentication();
char* printAlarmState();
void executeOption(int opt);
void printSettings();
void printLog();
void encrypt_decrypt();
int send_email();
void run_alarm(enum Alarm_state state);
int alarmState = OFF;
FILE *fptr = NULL;
time_t set_off_time = NULL;
extern int terminated;

int introductionMenu(){

   //clear menu screen
   system("clear");
   
   printf("Welcome to Burglar Alarm System\n\n\n");

   printf("Developed by Tarun Sabbineni, Vinamara Agrawal, ");
   printf("Tanmay Khanna, Balint Babik\n\n\n\n"); 
         
   delay(1000);

   printf("Please enter your password \n");

   return (getAuthentication());
}

int printMainMenu(){

   //clear menu screen
   system("clear");

   printf("Welcome \n\n\n");

   printf("Current Selected alarm State - %s\n\n\n", printAlarmState() );

   printf("Please select from one of these options \n\n");
   printf("1> Activate alarm \n");
   printf("2> Change alarm state \n");
   printf("3> Go to settings\n");
   printf("4> Print Log\n");
   printf("5> Exit\n\n");

   char choice[10];

   printf("Enter desired option number\n");
   scanf("%s", choice);
   
   while (strlen(choice) != 1) {
      printf("Invalid choice entered. Please try again!\n");
      scanf("%s", choice);
   }

   int option = atoi(choice);
 
   return option;
}

char* printAlarmState(){
   if(alarmState == 0){
      return "ON";
   }
   return "OFF";
}

int getAuthentication(){

   fptr = fopen("password.txt","r");

   if(fptr == NULL){
    printf("Unable to password file\n");
    exit(1);
   }

   //decrypting after opening from file to compare with input
   char password[MAX_PASSWORD_LEN];
   fscanf(fptr, "%s", password);
   encrypt_decrypt(password);

   char input[MAX_PASSWORD_LEN];

   for (int i = 5; i > 0; i--){
      scanf("%s",input);
      if(!strcmp(input,password)){
         free(password);
         fclose(fptr);
         return 1;
      }
      if ( i-1 > 0 ) {
       printf("Incorrect password %i attempts left\n",i-1);
      } else {
      printf("You have been visited by the lockout meme.\nUpboat this 5 times on leddit or the unlocking skeltal will never come to you.\n");
      send_email();
      run_alarm(OFF);
      }
   }

   fclose(fptr);

   return 0;
}

void executeOption(int opt){

   switch(opt){
    
      case 1:run_alarm(alarmState); 
             break;
      case 2:if(alarmState == OFF){
                alarmState = ON;
             } else {
                alarmState = OFF;
                }
             executeOption(printMainMenu());
             break;
      case 3:printSettings();
             break;
      case 4:printLog();
             break;
      case 5:printf("exiting\n");
             digitalWrite(REDPIN, LOW);
             digitalWrite(BLUEPIN, LOW);
             digitalWrite(BUZZERPIN, LOW);
             exit(0);
             break;
      default: printf("Invalid option given");
               
   }

}

int get_option(void) {
  printf("Enter desired option number\n");
  int option ;  
  char filler; 
  scanf(" %c", &filler); 
  option = filler - '0';
  while((filler = getchar()) != '\n');
  while(option < 1 || option > 4 ){
     printf("Invalid option entered retry\n");
     option  = getchar() - '0';
     while((filler = getchar()) != '\n');
  }
  return option;   
}

void printSettings(){
   system("clear");

   printf("Please select from one of these options \n\n");
   printf("1> Change password \n");
   printf("2> Change log list address \n");
   printf("3> Change administration email address\n");
   printf("4> Go back\n");

   
   int option = get_option();

  
   // executing options
   switch(option) {

       case 1: printf("Enter current password: \n");
               if(getAuthentication()){

                 printf("Correct! Enter new password: \n");
                 char password[MAX_PASSWORD_LEN];
                 scanf("%s",password);

                 //encrypting before storing in file
                 encrypt_decrypt(password);
                 fptr = fopen("password.txt", "w");
                 fwrite(password, sizeof(password), 1, fptr);
                 fclose(fptr);

                 printf("password changes successfully\n");
                 delay(1000);
                 printf("Returning to main menu\n");
                 delay(500);
                 executeOption(printMainMenu());
               }
               break;
       case 2: //TODO: Implement log lists  
               break;

       case 3: // oprning data file
               fptr = fopen("email.txt","r");

               if(fptr == NULL){
                 printf("Unable to password file\n");
                 exit(1);
               }
               email = malloc(sizeof(char)*50);
               fscanf(fptr, "%s", email);
               //encrypt_decrypt(email);
         
               printf("Current email: %s\n", email);
              
               fclose(fptr);

 
               printf("Change email address (y/n)\n");
               getchar(); 
               char option;
               scanf("%c", &option);
              
               if (option == 'y'){
                  fptr = fopen("email.txt","w");
                  printf("Enter new administration email address:\n");
                  char input[50];
                  scanf("%s",input);
                  //encrypt_decrypt(input);
                  fwrite(email, sizeof(input), 1, fptr);
                  fclose(fptr);
               
                printf("email changes successfully\n");
                delay(1000);
                }
                printf("Returning to main menu\n");
                delay(500);
                free(email);
                executeOption(printMainMenu());
               break; 

       case 4: executeOption(printMainMenu());
               break;  
       default: printf("Error invalid option");
   }   
   
}
void printLog(){
 //TODO
}

void encrypt_decrypt(char *password){

  char key[] = "AHFLSHENDHSOGKNASBBKJKDVVKASHDASHBDASLLUHSC"; 
	
	for(int i = 0; i < strlen(password); i++) {

		password[i] = password[i] ^ key[i % (sizeof(key)/sizeof(char))];

	}

}

int send_email() {

  //obtaining email stored in file
  email = malloc(sizeof(char)*50);
  fscanf(fptr, "%s", email);
  fclose(fptr);


  //constructing appropriate command to send email
  char cmd[500];

  strcat(cmd, "curl --url \"smtps://smtp.gmail.com:465\" --ssl-reqd \
               --mail-from \"burglarpi@gmail.com\" \
               --mail-rcpt \"");

  strcat(cmd, email);

  strcat(cmd, "\" \
               --upload-file mail.txt \
               --user \"burglarpi@gmail.com:imperial15\" --insecure \
               &>/dev/null &");


  //calling appropriate command to send email
  system(cmd);

  //working code to help debug
  //system("curl --url \"smtps://smtp.gmail.com:465\" --ssl-reqd \
  //             --mail-from \"burglarpi@gmail.com\" \
  //             --mail-rcpt \"tsabbineni@gmail.com\" \
  //             --upload-file mail.txt \
  //             --user \"burglarpi@gmail.com:imperial15\" --insecure \
  //             &>/dev/null &");

  puts("ATTENTION: The homeowner has been notified of this incident.");

  return 0;
}


void run_alarm(enum Alarm_state state) {
  
 
  // setting all pins to 0
  wiringPiSetupGpio();

  // setting up output pins
  pinMode(REDPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);
  
  // setting up input pin
  pinMode(PIRPIN, INPUT);
  
  printf("Running alarm, current mode is:");
  
  // displaying mode setting
  if (state == ON) {
    printf("ON\n");
    digitalWrite(REDPIN, HIGH);
    digitalWrite(BLUEPIN, LOW);
  } else {
    printf("OFF\n");
    digitalWrite(REDPIN, LOW);
    digitalWrite(BLUEPIN, HIGH);
  }
  digitalWrite(BUZZERPIN, LOW);

  if(state == ON){   
    // settling
    printf("waiting for PIR to settle...\n");
    while (digitalRead(PIRPIN) == 1) {
    }
    printf("ready\n");

    while(1) {
      // PRE: Someone walks in.
      system("./send_limit"); //TODO
      if(digitalRead(PIRPIN) == 1) {
        digitalWrite(BUZZERPIN, HIGH);
        set_off_time = time();

        if (getAuthentication()) {
        digitalWrite(BUZZERPIN, LOW);
        terminated = 1;
        }  
        break; 
      }
    }
  } else if (!getAuthentication()) {
    digitalWrite(BUZZERPIN, HIGH);
  }
     
  delay(TEST_BUZZER_TIME);
  digitalWrite(BUZZERPIN, LOW);
  
  printf("Setting alarm off\n");
  digitalWrite(REDPIN, LOW);
  digitalWrite(BLUEPIN, LOW);
}

int main(void){
  // options
  // activate
  // change state
  // settigns (change password, log list ,email)
  // print loglist
  // Exit
 
  // print introduction and check for inital authentication
  if (introductionMenu()){
    // choose the menu option
    int menuOpt = printMainMenu();
    executeOption(menuOpt);
    return 0;
  } else {
    printf("Wrong passcode.\nAborting system\n");
    return 1;
  }
}