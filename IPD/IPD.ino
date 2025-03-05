// C++ code
//
// int f2 = 0;
// int f1=0;
// int f3=0;
// int fs1;
// void setup()
// {
  
//   Serial.begin(9600);
// }

// void loop()
// {
//   fs1 = analogRead(13);
//   // fs2 = analogRead(A1);
//   // fs3 = analogRead(A2);
//   // if(fs1<20){
//   //   f1=1;}
//   //  else{
//   //     f1=0;}
//   // if(fs2<20){
//   //   f2=1;}
//   //  else{
//   //    f2=0;}
//   //  if (fs3<20){
//   //   f3=1;}
//   // else{
//   //   f3=0;}
//     // Serial.print("F1 = ");
//   Serial.println(fs1);
// //  Serial.print("F2 = ");
// //   Serial.println(f2);
// //   Serial.print("F3 = ");
// //   Serial.println(f3);
//   delay(50); // Wait for 100 millisecond(s)
// }
const int flexPin = 15; 

void setup() { 
  Serial.begin(9600);
} 

void loop(){ 
  int flexValue;
  flexValue = analogRead(flexPin);
  Serial.print("sensor: ");
  Serial.println(flexValue);
 
  
 
  delay(20);
} 