 #include <Ethernet.h>
#include "sqlard.h"


uint8_t Ethernet_MacAddr[6] = { 0x74,0x69,0x69,0x2D,0x30,0x31 }; /* MAC ADRESİ */
static byte Static_IPAddr[] = { 172,16,1,2 };            
static byte Gateway_IPAddr[] = { 172,16,1,1 };
static byte Subnet_Mask[] = { 255,255,0,0 };
EthernetClient client;

SQLard MSSQL(Gateway_IPAddr, 1433, &client);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial); 
  Ethernet.begin(Ethernet_MacAddr, Static_IPAddr, Gateway_IPAddr, Gateway_IPAddr, Subnet_Mask);
  if(MSSQL.connect())
  {
      MSSQL.setCredentials(L"arduino", L"ard_login",L"ard_password",L"hostx");
      MSSQL.login();
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int loop_count = 0;
  delay(5000);

  long affected_rows = MSSQL.executeNonQuery(L"INSERT INTO [dbo].[test]([data]) VALUES('deger1234') ");
  long yeni = MSSQL.executeNonQuery(L"DROP TABLE [dbo].[test23]");
  Serial.print(affected_rows);
  Serial.println(" row(s) affected.");
  if (!client) {
    Serial.println("disconnecting.");
    client.stop();
  }
  if(++loop_count == 10)
  {
    MSSQL.executeNonQuery(L"DELETE FROM [dbo].[test]");
    loop_count = 0;
  }
}
