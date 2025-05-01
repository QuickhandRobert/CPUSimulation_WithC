#include <stdio.h>
#define MAX_SIZE 256

int Stack[MAX_SIZE];
int top = -1;
int isNotEnded = 1;
int ReadInstruction(){
  char c ; 
  int counter = 0;
  while((c = getchar())!=' ' && c != '\n' && c != EOF){
    counter++;
  }
  switch(counter){

    case 3:
      return 3;
      break;
      
    case 4:
      return 4;
      break;
    
    case 5:
      return 5;
      break;    
  }
  if(c == EOF || counter == 1){
    isNotEnded = 0;
  }
  return 0;
  
}
int get_number(){
  int num = 0 , flag = 0;
  char n ;
  while((n = getchar())!= '\n'){
    if (n == '-'){
      flag = 1;
    }
    else{
      num  = num * 10 + ( n - '0');  
    }
  }
  if(flag){
    return -num;
  }
  return num;
}
void push(int SomeNumber){
  if(top >= MAX_SIZE - 1){
    printf("Stack Full!\n");
  }
  else{
    Stack[++top] = SomeNumber;
  }
}
void pop(){
  if(top < 0){
    printf("Stack Empty!\n");
  }
  else{
    --top;
  }
}
void print(){
  int j = top;
  if(top < 0){
    printf("Nothing to Print!\n");
  }
  else{
    while(j > 0){
      printf("%d ",Stack[j] );
      j--;
    }
    printf("%d\n",Stack[j]);
  }
}

int main(){
  int Number;
  while(isNotEnded){
    int command = ReadInstruction();
    switch(command){
      case 3:
        pop();
        break;
      
      case 4:
        Number = get_number();
        push(Number);
        break;
      
      case 5:
      print();
      break;  
    }
  }
}
