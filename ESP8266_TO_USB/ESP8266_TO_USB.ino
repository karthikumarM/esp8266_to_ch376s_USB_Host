#include <Ch376msc.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>


void writeString(int add, String data);
String read_String(int add);
void write_usb();
void usb_checking();

SoftwareSerial serial1(13, 15); // RX, TX pins on CH376MCS
SoftwareSerial serial2(3, 1); // RX, TX pins on ATmega16A

Ch376msc flashDrive(serial1); // Ch376 object with software serial
int String_Delet;
char csv_frame[70] = {"BALE_NO,R_1,R_2,R_3,R_4,R_5,R_6,R_7,R_8,R_9,R_10,AVG"};
char new_line[10] = {0x0d, 0X0a};
String usb_check = "ch376s";
char Moisture_Data[4005] = "";
byte tmpCommand;
int moisture_value, data_length = 0;
int i, memory_status = 1;
volatile int eeprom_memory = 10;
char Uart_String_value[150] = "";




void setup() {
  Serial.begin(9600);// Important! First initialize soft serial object and after atmega16a
  serial1.begin(9600);// Important! First initialize soft serial object and after Ch376
  EEPROM.begin(512);
  flashDrive.init();
  Serial.println("");
  Serial.println("CH376S USB FLASH DRIVER");
  delay(10);
  eeprom_memory = EEPROM.read(0);
  delay(500);
  Serial.println('~');
  delay(100);
}

void loop() {
  usb_checking();
  if (eeprom_memory > 4000)
  {
    if (memory_status == 0)
    {
      delay(100);
      Serial.println('$');
      delay(10);
      memory_status = 1;
    }
  }
  else
  {
    memory_status = 0;
  }

  usb_checking();
  byte n = (Serial.available());
  if (n != 0) {
    usb_checking();
    (Serial.readBytesUntil('\n', Uart_String_value, 150) + 1);
    if ((usb_check.compareTo(Uart_String_value)) == 0)
    {
      write_usb();
      Serial.println("success");
      EEPROM.write(0, 10);
      delay(10);
      eeprom_memory = 10;
      EEPROM.commit();
    }
    else
    {
      usb_checking();
      writeString(eeprom_memory, Uart_String_value);  //Address 0 and String type data
      EEPROM.commit();
    }
    Serial.println(Uart_String_value);
    for (String_Delet = 0; String_Delet <= 150; String_Delet++) {
      Uart_String_value[String_Delet] = 0;
    }


  }
  if (serial1.available()) {
    usb_checking();
    tmpCommand = serial1.read();                                                    //read incoming bytes from the serial monitor
    if (((tmpCommand > 48) && (tmpCommand < 58)) && !flashDrive.driveReady()) {     // if the data is ASCII 1 - 9 and no flash drive are attached
      Serial.println("Attach flash drive first!");
      //tmpCommand = 10; // change the command byte
    }
  }
}

//**************************************USB CHECK *****************************************//

void usb_checking()
{
  if (flashDrive.checkIntMessage()) {
    if (flashDrive.getDeviceStatus()) {
      Serial.println(F("Flash drive attached!"));
      delay(100);
      Serial.println('@');
      delay(10);
    } else {
      Serial.println(F("Flash drive detached!"));
      delay(100);
      Serial.println('~');
      delay(10);
    }
  }  
}
//**************************************write data to EEPROM *****************************************//

void writeString(int add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  //EEPROM.write(0 ,_size);
  eeprom_memory += _size;
  Serial.print("eeprom_memory=");
  Serial.println(eeprom_memory);
  EEPROM.write(0 , eeprom_memory);
  EEPROM.commit();
}

//**************************************read data to EEPROM *****************************************//
String read_String(int add)
{
  int i;
  char data[990]; //Max 990 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 4000) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}
//**************************************write data to usb *****************************************//
void write_usb()
{

  String recivedData;
  recivedData = read_String(10);

  recivedData.toCharArray(Moisture_Data, recivedData.length() + 1);

  for ( i = 0; i < recivedData.length() + 1; i++)
  {
    Serial.print(Moisture_Data[i]);
  }
  Serial.print("");
  Serial.println(strlen(Moisture_Data));
  Serial.println("COMMAND1: Create and write data to file : DATA.CSV");    // Create a file called DATA.CSV
  flashDrive.setFileName("DATA.CSV");  //set the file name
  //flashDrive.openFile();                //open the file
  if (flashDrive.openFile() == 0x14)
  {
    flashDrive.moveCursor(CURSOREND);
    flashDrive.writeFile(new_line, strlen(new_line));//new line
    flashDrive.writeFile(new_line, strlen(new_line));//new line
  }
  else {                                               //write text from string(adat) to flash drive 20 times
    flashDrive.writeFile(csv_frame, strlen(csv_frame));//csv file formate array
    flashDrive.writeFile(new_line, strlen(new_line));//new line
  }
  data_length = strlen(Moisture_Data);
  if (data_length <= 255)

  {
    flashDrive.writeFile(Moisture_Data, data_length); //string, string length
    Serial.println("255");
  }
  else if ((data_length > 255) && (data_length <= 510))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    Serial.println("510");
  }
  else if ((data_length > 510) && (data_length <= 765))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    Serial.println("765");
  }
  else if ((data_length > 765) && (data_length <= 1020))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
  }
  else if ((data_length > 1020) && (data_length <= 1275))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
  }
  else if ((data_length > 1275) && (data_length <= 1530))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
  }
  else if ((data_length > 1530) && (data_length <= 1785))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
  }
  else if ((data_length > 1785) && (data_length <= 2040))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
  }
  else if ((data_length > 2040) && (data_length <= 2295))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1786], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
  }
  else if ((data_length > 2295) && (data_length <= 2550))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
  }
  else if ((data_length > 2550) && (data_length <= 2805))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
    flashDrive.writeFile(&Moisture_Data[2550], (data_length - 2550));
  }
  else if ((data_length > 2805) && (data_length <= 3060))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
    flashDrive.writeFile(&Moisture_Data[2550], (data_length - 2550));
    flashDrive.writeFile(&Moisture_Data[2805], (data_length - 2805));
  }
  else if ((data_length > 3060) && (data_length <= 3315))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
    flashDrive.writeFile(&Moisture_Data[2550], (data_length - 2550));
    flashDrive.writeFile(&Moisture_Data[2805], (data_length - 2805));
    flashDrive.writeFile(&Moisture_Data[3060], (data_length - 3060));
  }

  else if ((data_length > 3315) && (data_length <= 3570))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
    flashDrive.writeFile(&Moisture_Data[2550], (data_length - 2550));
    flashDrive.writeFile(&Moisture_Data[2805], (data_length - 2805));
    flashDrive.writeFile(&Moisture_Data[3060], (data_length - 3060));
    flashDrive.writeFile(&Moisture_Data[3315], (data_length - 3315));
  }
  else if ((data_length > 3570) && (data_length <= 3825))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
    flashDrive.writeFile(&Moisture_Data[2550], (data_length - 2550));
    flashDrive.writeFile(&Moisture_Data[2805], (data_length - 2805));
    flashDrive.writeFile(&Moisture_Data[3060], (data_length - 3060));
    flashDrive.writeFile(&Moisture_Data[3315], (data_length - 3315));
    flashDrive.writeFile(&Moisture_Data[3570], (data_length - 3570));
  }
  else if ((data_length > 3825) && (data_length <= 4080))
  {
    flashDrive.writeFile(Moisture_Data, 255);
    flashDrive.writeFile(&Moisture_Data[255], (data_length - 255));
    flashDrive.writeFile(&Moisture_Data[510], (data_length - 510));
    flashDrive.writeFile(&Moisture_Data[765], (data_length - 765));
    flashDrive.writeFile(&Moisture_Data[1020], (data_length - 1020));
    flashDrive.writeFile(&Moisture_Data[1275], (data_length - 1275));
    flashDrive.writeFile(&Moisture_Data[1530], (data_length - 1530));
    flashDrive.writeFile(&Moisture_Data[1785], (data_length - 1785));
    flashDrive.writeFile(&Moisture_Data[2040], (data_length - 2040));
    flashDrive.writeFile(&Moisture_Data[2295], (data_length - 2295));
    flashDrive.writeFile(&Moisture_Data[2550], (data_length - 2550));
    flashDrive.writeFile(&Moisture_Data[2805], (data_length - 2805));
    flashDrive.writeFile(&Moisture_Data[3060], (data_length - 3060));
    flashDrive.writeFile(&Moisture_Data[3315], (data_length - 3315));
    flashDrive.writeFile(&Moisture_Data[3570], (data_length - 3570));
    flashDrive.writeFile(&Moisture_Data[3825], (data_length - 3825));
  }

  flashDrive.closeFile();               //at the end, close the file
  Serial.println("Done!");


}
