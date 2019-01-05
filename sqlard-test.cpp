
//#include "stdafx.h"
#include "sqlard.h"

int main()
{
	/* local */
	unsigned char ipAddr[] = { 127,0,0,1 };
	unsigned short port = 1433;
	wchar_t userName[] = L"arduino";
	wchar_t passWord[] = L"arduino";
	wchar_t database[] = L"test"; 

		SQLard MSSQL(ipAddr, port);
		if (MSSQL.connect()) 
		{
			MSSQL.setCredentials(database, userName, passWord, L"host");
			if (MSSQL.login()) {
				printf("login ok \n");

			}
			while(true)
			{	
			
				SQLardTableResult * tr = MSSQL.executeReader(L"SELECT water_enable FROM ROOM_STATUS WHERE dnd_guid = '4567A1FF-3790-4519-83EE-A709A59E238F'");
				printf("COLUMNS\n");
				for (int i = 0; i < tr->m_usColumnCount; i++) {
					wprintf(L"%s|", tr->m_arColumnData[i]->m_wcstrColumnName);
				}
				printf("\n");
				while (tr->GetRow() != nullptr)
				{
					const SQLardRowData & pRow = (*tr->GetRow());
					for (int i = 0; i < tr->m_usColumnCount; i++) {
						
						const SQLardRowFieldData * pField = pRow[i];
						switch (tr->GetColumnDataType(i)) 
						{
							/*case SQLardDataType::GUIDTYPE:
								break;*/
							case SQLardDataType::VARCHARTYPE:
							case SQLardDataType::CHARTYPE:
							case SQLardDataType::BIGVARCHRTYPE:
							case SQLardDataType::BIGCHARTYPE:
								printf("%s\t\t", pField->asVarchar());
								break;
							case SQLardDataType::BITTYPE:
								printf("%s\t\t", pField->interpret_integer<bool>() == true ? "true":"false");
								break;
							case SQLardDataType::INT1TYPE:
								printf("%d\t\t", pField->interpret_integer<unsigned char>());
								break;
							case SQLardDataType::INT2TYPE:
								printf("%d\t\t", pField->interpret_integer<signed short>());
								break;
							case SQLardDataType::INT4TYPE:
								printf("%d\t\t", pField->interpret_integer<signed long>());
								break;
							case SQLardDataType::NUMERICNTYPE: /* temporary */
							case SQLardDataType::INT8TYPE:
								printf("%lld\t\t", pField->interpret_integer<signed long long>());
								break;
							case SQLardDataType::FLT4TYPE:
								printf("%g\t\t", pField->asFloat());
								break;
							case SQLardDataType::FLT8TYPE:
								printf("%g\t\t", pField->asDouble());
								break;
							case SQLardDataType::BINARYTYPE:
							case SQLardDataType::BIGBINARYTYPE:
							case SQLardDataType::BIGVARBINTYPE:
							case SQLardDataType::VARBINARYTYPE:
							case SQLardDataType::GUIDTYPE:
								printf("0x");
								for (int i = 0; i < pField->m_usLength; i++) {
									printf("%02x", pField->getByte(i));
								}
								printf("\t\t");
								break;
							case SQLardDataType::DATETIMETYPE:
							{
								time_t val = pField->asDateTime();
								struct tm q;
								localtime_s(&q, &val);
								printf("now: %d-%d-%d %d:%d:%d\t\t",
									q.tm_year + 1900,
									q.tm_mon + 1,
									q.tm_mday,
									q.tm_hour,
									q.tm_min,
									q.tm_sec);

							}
								break;
							default:
								printf("undefined (yet)\n");
								break;
						}
					}
					printf("\n");
					tr->MoveNext();
				}
				delete tr;
			}
		
		}

		getchar();
    return 0;
}

