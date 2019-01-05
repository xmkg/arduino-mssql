#ifndef SQLARD_H
#define SQLARD_H
#define UIPETHERNET
#include <stdarg.h>

#ifdef WINDOWS
	#define F 
	#define PROGMEM
	#include <boost/array.hpp>
	#include <boost/asio.hpp>
#else
	#ifdef UIPETHERNET
		#include <UIPEthernet.h>
	#else 
		#include <Ethernet.h>
	#endif
#endif
#define SQLARD_SKIP_COLUMN_NAMES
//#define SQLARD_VERBOSE_OUTPUT
/* 
	Memory benchmarks
	!! All examples are compiled for Arduino Nano !!

	Included components							Flash Memory Usage (bytes)		RAM usage (bytes)			Flash(SQLard)		RAM(SQLard)
	Ethernet.h									11062							289							-					-
	UIPEthernet.h(default)						19406							1189						-					-
	UIPEthernet.h(NO UDP)						12582							974							-					-
	UIPEthernet.h(NUDP),sqlard.h(no verbose)	18794							1209						6212				235
	UIPEthernet.h(NUDP),sqlard.h(verbose)		22034							1211						9452				237
	Ethernet.h, sqlard.h(no verbose)			17322							517							6260				228						
	Ethernet.h, sqlard.h(verbose)				21242							692							10180				403		

*/



/*
	<< SQLard >>

	* File 				: 	sqlard.h
	* Author			:	Mustafa K. GILOR
	* Created on 		:	24/12/2016
	* Last modified		:	15/01/2017
	* Status			:	Complete
	* Purpose *
		Providing MSSQL client functionality to any Arduino device, with minimal memory footprint.
		Implemented in TDS 7.0 standarts, compatible with both Ethernet and UIPEthernet libraries.
		Also can work under Windows(requires BOOST library)
	* Usage example * 
	void setup()
	{
		uint8_t ipAddr[] = { 127,0,0,1 };
		SQLard MSSQL(ipAddr, 1433);
		if (MSSQL.connect()) {
			MSSQL.setCredentials(L"arduino", L"arduino", L"arduino", L"host");
		if (MSSQL.login()) {
		printf("login ok \n");

		}
			MSSQL.executeNonQuery(L"INSERT INTO [dbo].[test]([data]) VALUES('DATAAAAA')");
	}
*/


/*
	RAII style buffer object
*/
template <typename T>
struct SQLardBuffer {
public:
	SQLardBuffer(const size_t allocation_size) {
		allc_size = allocation_size;
		m_pBuffer = new T[allc_size];
		memset(m_pBuffer, 0, allc_size);
	}
	~SQLardBuffer() {
		delete[] m_pBuffer;
		//Serial.print("free\n");
	}
	const size_t alloc_size() const { return allc_size; }
	T * operator()() { return m_pBuffer; }
	T & operator[](const size_t offset) { return m_pBuffer[offset]; }
private:
	T * m_pBuffer;
	size_t allc_size;
};

template <typename T>
struct SQLardRowElement
{
	SQLardRowElement(T v)
	{
		val = v;
	}
	// Stored value
	T val;
	SQLardRowElement<T> * prev = nullptr;
	SQLardRowElement() {
	}
	~SQLardRowElement<T>() {
		delete val;
	}
};

template <typename T>
struct SQLardRowList
{
public:

	SQLardRowList() {}

	~SQLardRowList()
	{
		Free();
	}

	void Free() {
		// Pop all element(s) and free the allocated space
		SQLardRowElement<T> * node = root;
		while (node != nullptr)
		{
			SQLardRowElement<T> * next = node->prev;
			delete node;
			node = next;
		}
	}


	/* Push a new element to the Queue */
	void Enqueue(T c)
	{
		SQLardRowElement<T> * tmp = Create(c);
		if (head)
			head->prev = tmp;
		head = tmp;

		// If root is nullptr, then Queue is empty and it's the first element
		if (root == nullptr)
		{
			root = tmp;
			current = tmp;
		}
	}
	SQLardRowElement<T> * GetRoot() { return root; }
	const SQLardRowElement<T> * GetCurrent() const { return current; }
	void MoveNext() {
		if (current != nullptr)
			current = current->prev;
	}
	void Reset() {
		current = root;
	}
	/* Returns true if Queue is empty (the root should be null) */
	bool isEmpty() { return root == nullptr; }
private:
	/* The first added elements' address */
	SQLardRowElement<T> * root = nullptr;
	/* The last added elements' address */
	SQLardRowElement<T> * head = nullptr;
	SQLardRowElement<T>  * current = nullptr;

	/* Creates a new LinkListElement object and returns its' address */
	SQLardRowElement<T> * Create(T val)
	{
		return new SQLardRowElement<T>(val);
	}

	/* Returns the value of existing LinkListElement object and frees the memory */
	T Remove(SQLardRowElement<T> * p)
	{
		T val = p->val;
		delete p;
		return val;
	}
};

enum SQLardDataType
{
	/* Null */
	NULLTYPE = 0x1,
	/* Tinyint */
	INT1TYPE = 0x30,
	/* Bit */
	BITTYPE = 0x32,
	/* Smallint */
	INT2TYPE = 0x34,
	/* Int */
	INT4TYPE = 0x38,
	/* Smalldatetime */
	DATETIM4TYPE = 0x3A,
	/* Real */
	FLT4TYPE = 0x3B,
	/* Money */
	MONEYTYPE = 0x3C,
	/* Datetime */
	DATETIMETYPE = 0x3D,
	/* Float */
	FLT8TYPE = 0x3E,
	/* Smallmoney */
	MONEY4TYPE = 0x7A,
	/* Bigint */
	INT8TYPE = 0x7F,
	/* Unique identifier 16 byte binary */
	GUIDTYPE = 0x24,
	/* N int */
	INTNTYPE = 0x26,
	/* Decimal */
	DECIMALTYPE = 0x37,
	/* Numeric */
	NUMERICTYPE = 0x3F,
	/* N bit */
	BITNTYPE = 0x68,
	/* Decimal N */
	DECIMALNTYPE = 0x6A,
	/* Numeric with variable length */
	NUMERICNTYPE = 0x6C,
	/* Float with variable length */
	FLTNTYPE = 0x6D,
	/* Money with variable length*/
	MONEYNTYPE = 0x6E,
	/* Date time with variable length*/
	DATETIMNTYPE = 0x6F,
	/* Char (legacy support)*/
	CHARTYPE = 0x2F,
	/*VarChar(legacy support)*/
	VARCHARTYPE = 0x27,
	/*Binary(legacy support)*/
	BINARYTYPE = 0x2D,
	VARBINARYTYPE = 0x25,
	/*VarBinary*/
	BIGVARBINTYPE = 0xA5,
	/* VarChar*/
	BIGVARCHRTYPE = 0xA7,
	/* Binary*/
	BIGBINARYTYPE = 0xAD,
	/*Char*/
	BIGCHARTYPE = 0xAF,
	/*NVarChar*/
	NVARCHARTYPE = 0xE7,
	/*NChar*/
	NCHARTYPE = 0xEF,
	/* Text */
	TEXTTYPE = 0x23,
	/* Image */
	IMAGETYPE = 0x22,
	/* Ntext */
	NTEXTTYPE = 0x63
};

class SQLardUtil {
public:
#ifdef SQLARD_VERBOSE_OUTPUT
	#define PRINTF_BUF 255 // define the tmp buffer size (change if desired)
		static void printf(const char *format, ...)
		{
			#ifdef WINDOWS
			va_list ap;
			va_start(ap, format);
			vprintf(format, ap);
			va_end(ap);
			#else
				char buf[PRINTF_BUF];
				va_list ap;
				va_start(ap, format);
				vsnprintf(buf, sizeof(buf), format, ap);
				for (char *p = &buf[0]; *p; p++) // emulate cooked mode for newlines
				{
					if (*p == '\n')
						Serial.write('\r');
					Serial.write(*p);
				}
				va_end(ap);
			#endif
		}
	#ifndef WINDOWS
		#ifdef F // check to see if F() macro is available
			static void printf(const __FlashStringHelper *format, ...)
			{
				char buf[PRINTF_BUF];
				va_list ap;
				va_start(ap, format);
		#ifdef __AVR__
				vsnprintf_P(buf, sizeof(buf), (const char *)format, ap); // progmem for AVR
		#else
				vsnprintf(buf, sizeof(buf), (const char *)format, ap); // for the rest of the world
		#endif
				for (char *p = &buf[0]; *p; p++) // emulate cooked mode for newlines
				{
					if (*p == '\n')
						Serial.write('\r');
					Serial.write(*p);
				}
				va_end(ap);
			}
		#endif
	#endif
#endif

	static int freeRam(const char * who) {
		extern int __heap_start, *__brkval;
		int v;
		int fr = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
		Serial.print(who);
		Serial.print("Free ram: ");
		Serial.println(fr);

	}
	/*
	* @brief 	Measure the length of a wide char string
	* @return	Measured length as size_t 
	*/
	static size_t sqlard_wcslen(const wchar_t * s)
	{
		if (s == nullptr)
			return 0;
		const wchar_t *p;
		p = s;
		while (*p)
			p++;
		return p - s;
	}

	/*
	* @brief 	Copy a wide char array from one memory place to another.
	* @return	Beginning of the copied region as wide char pointer
	*/
	static wchar_t * sqlard_wmemcpy(void * d, const wchar_t * s, size_t n)
	{
		if (s == nullptr || d == nullptr)
			return nullptr;
		return (wchar_t *)memcpy(d, s, n * sizeof(wchar_t));
	}
	/*
	* @brief 	Copy a wide char string from one memory place to another.
	* @return	Beginning of the copied region as wide char pointer
	*/
	static wchar_t * sqlard_wstrcpy(void *d, const wchar_t * s)
	{
		return SQLardUtil::sqlard_wmemcpy(d, s, sqlard_wcslen(s));
	}

	/*
	* @brief 	Read an amount of bytes from source array to destination,
				and also move offset to offset + len.
	* @return	Beginning of destination array as uint8_t pointer.
	*/
	static uint8_t * sqlard_read_bytes(void * d, const uint8_t * s, size_t & offset, const uint16_t len) {
		void * p = memcpy(d, &s[offset], len);
		offset += len;
		return static_cast<uint8_t*>(p);
	}

	
	/*
	* @brief 	Convert a wide char string to multi byte string.
	*			The destination will be automatically null-terminated.
	*/
	static void sqlard_wctomb(uint8_t * dst, uint8_t * buf, const uint32_t len)
	{
		for (uint32_t i = 0, q = 0; i < len; i += 2, q += 1) {
			dst[q] = buf[i];
		}
		dst[len / 2] = '\0';
	}

	/*
	* @brief 	Allocate a new wide character array, and copy the source to it.	
	* @return	The wide character pointer to allocated wide character array.
	*/
	static wchar_t* sqlard_alloc_wstr(const wchar_t * s) {
		wchar_t * d = new wchar_t[sqlard_wcslen(s) + 1];
		wchar_t *save = d;
		for (; (*d = *s); ++s, ++d);
		return save;
	}

	/*
	* @brief 	Allocate a new wide character array, read wide character string
				from source array[offset], and move the offset to offset + (len * 2)
	* @return	The wide character pointer to allocated wide character array.
	*/
	static wchar_t* sqlard_read_nwstr(uint8_t * s, size_t & offset, const uint16_t wslen) {
		wchar_t * d = new wchar_t[wslen + 1];
		memcpy(d, &s[offset], wslen * 2);
		offset += (wslen * 2);
		/* null terminate the string */
		d[wslen] = '\0';
		return d;
	}

	/*
	* @brief 	Write a n bit integer to buffer[offset], in little endian bit order,
				and move the offset to offset + (n / 8).
	*/
	template<typename T>
	static void sqlard_write_le(uint8_t * buf, size_t & offset, const T val) {
		//printf("sizeof(T) : %d\n", sizeof(T));
		for (uint8_t shift = 0; shift < (sizeof(T) * 8); shift += 8, offset++) {
			//printf("\tShift %d offset %d", shift, offset);
			buf[offset] = static_cast<uint8_t>(val >> shift);
		}
		//printf("\n");
	}
	/*
	* @brief 	Write a n bit integer to buffer[offset], in big endian bit order,
				and move the offset to offset + (n / 8).
	*/
	template<typename T>
	static void sqlard_write_be(uint8_t * buf, size_t & offset, const T val) {
		for (char shift = ((sizeof(T) * 8) - 8); shift >= 0; shift -= 8, offset++) {
			buf[offset] = static_cast<uint8_t>(val >> shift);
		}
	}

	template<typename T>
	static T sqlard_read_le(uint8_t * buf, size_t & offset, size_t shift_max = sizeof(T) * 8) 
	{
		T value = 0;
		for (uint8_t shift = 0; shift < shift_max; shift += 8, offset++) {
			value |= static_cast<T>(static_cast<T>(buf[offset]) << shift);
		}
		return value;
	}

	template<typename T>
	static T sqlard_read_be(uint8_t * buf, size_t & offset)
	{
		T value = 0;
		for (char shift = ((sizeof(T) * 8) - 8); shift >= 0; shift -= 8, offset++) {
			value |= static_cast<T>(static_cast<T>(buf[offset]) << shift);
		}
		return value;
	}


	static void sqlard_rwstr_mb(uint8_t * dest, uint8_t * src, size_t & offset, const uint16_t len)
	{
		sqlard_wctomb(dest, &src[offset], len * 2);
		offset += (len * 2);
	}
};

class SQLardColumnData {
public:
	unsigned int m_uiUserType;
	uint16_t m_usFlags;
	uint8_t m_bType;
	uint16_t m_usLargeTypeSize;
	uint8_t m_bColumnNameLen;
	wchar_t * m_wcstrColumnName;

	static SQLardColumnData * ParseColumnData(uint8_t * data, size_t & offset) {

		SQLardColumnData * colData = new SQLardColumnData();
		colData->m_uiUserType = SQLardUtil::sqlard_read_le<uint16_t>(data, offset);
		colData->m_usFlags = SQLardUtil::sqlard_read_le<uint16_t>(data, offset);
		colData->m_bType = SQLardUtil::sqlard_read_le<uint8_t>(data, offset);

		switch (static_cast<SQLardDataType>(colData->m_bType)) {
			case SQLardDataType::BITTYPE:
			case SQLardDataType::INT1TYPE:
			case SQLardDataType::INT2TYPE:
			case SQLardDataType::INT4TYPE:
			case SQLardDataType::INT8TYPE:
			case SQLardDataType::DATETIMETYPE:
			case SQLardDataType::FLT4TYPE:
			case SQLardDataType::FLT8TYPE:
				break;
			case SQLardDataType::DECIMALNTYPE:
			case SQLardDataType::NUMERICNTYPE:
				/* precision and shit */
				/* like i care..*/
				SQLardUtil::sqlard_read_le<uint16_t>(data, offset);
				SQLardUtil::sqlard_read_le<uint8_t>(data, offset);
				break;
			/*
				IMAGETYPE / NTEXTTYPE / SSVARIANTTYPE / 
				TEXTTYPE / XMLTYPE
			*/
			case SQLardDataType::IMAGETYPE:
			case SQLardDataType::NTEXTTYPE:
			case SQLardDataType::TEXTTYPE:
				colData->m_usLargeTypeSize = static_cast<uint16_t>(SQLardUtil::sqlard_read_le<uint32_t>(data, offset));
				break;

			/*
				BIGVARBINTYPE / BIGVARCHRTYPE / BIGBINARYTYPE /
				BIGCHARTYPE / NVARCHARTYPE / NCHARTYPE
			*/
			case SQLardDataType::BIGVARBINTYPE:
			case SQLardDataType::BIGVARCHRTYPE:
			case SQLardDataType::BIGBINARYTYPE:
			case SQLardDataType::BIGCHARTYPE:
			case SQLardDataType::NVARCHARTYPE:
			case SQLardDataType::NCHARTYPE:
				colData->m_usLargeTypeSize = SQLardUtil::sqlard_read_le<uint16_t>(data, offset);
				break;
			/*
				GUIDTYPE / INTNTYPE / DECIMALTYPE / NUMERICTYPE / BITNTYPE / 
				DECIMALNTYPE / NUMERICNTYPE / FLTNTYPE / MONEYNTYPE / DATETIMNTYPE / 
				DATENTYPE / TIMENTYPE/ DATETIME2NTYPE / DATETIMEOFFSETNTYPE / CHARTYPE 
				/ VARCHARTYPE / BINARYTYPE / VARBINARYTYPE
			*/ 
			case SQLardDataType::GUIDTYPE:
			case SQLardDataType::INTNTYPE:
			case SQLardDataType::DECIMALTYPE:
			case SQLardDataType::NUMERICTYPE:
			case SQLardDataType::BITNTYPE:
			case SQLardDataType::FLTNTYPE:
			case SQLardDataType::MONEYNTYPE:
			case SQLardDataType::DATETIMNTYPE:
			case SQLardDataType::CHARTYPE:
			case SQLardDataType::VARCHARTYPE:
			case SQLardDataType::BINARYTYPE:
			case SQLardDataType::VARBINARYTYPE:
				colData->m_usLargeTypeSize = SQLardUtil::sqlard_read_le<uint8_t>(data, offset);
				break;
		default:
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(F("ParseColumnData() >> undefined data type %d\n"), colData->m_bType);
			#endif
			break;
		}

		colData->m_bColumnNameLen = SQLardUtil::sqlard_read_le<uint8_t>(data, offset);
		#ifndef SQLARD_SKIP_COLUMN_NAMES
			colData->m_wcstrColumnName = SQLardUtil::sqlard_read_nwstr(data, offset, colData->m_bColumnNameLen);
		#else
			offset += colData->m_bColumnNameLen * 2;
		#endif
		return colData;
	}

	SQLardColumnData() {
		m_uiUserType = 0;
		m_usFlags = 0;
		m_bType = 0;
		m_usLargeTypeSize = 0;
		m_bColumnNameLen = 0;
		m_wcstrColumnName = nullptr;
	}
	~SQLardColumnData() {
		if (!(nullptr == m_wcstrColumnName))
			delete[] m_wcstrColumnName;
	}
};

class SQLardRowFieldData {
public:
	uint8_t * m_pData;
	uint16_t m_usLength;
	uint8_t m_bSignFlag;
	SQLardRowFieldData() {
		m_pData = nullptr;
		m_usLength = 0;
		m_bSignFlag = 1;
	}
	~SQLardRowFieldData() {
		if (!(nullptr == m_pData))
			delete[] m_pData;
	}

	const uint32_t asDateTime() const {
		size_t offset = 0;
		int32_t number_of_days = SQLardUtil::sqlard_read_le<int32_t>(m_pData, offset);
		int32_t number_of_threehund = SQLardUtil::sqlard_read_le<int32_t>(m_pData, offset);
		/* Convert 1/300th of seconds to seconds */
		number_of_threehund /= 300;
		/* Convert days to seconds */
		number_of_days *= 86400;
		/* NTP timestamp to UNIX conversion constant*/
		const int32_t conv_unix = 0x83AA7E80;
		return number_of_days  - conv_unix + (number_of_threehund);
	}

	const uint8_t getByte(const uint16_t index) const {
		if (index >= m_usLength)
			return -1;

		return m_pData[index];
	}

	const uint8_t * asGUID() const {
		return m_pData;
	}

	const uint8_t * asVarchar() const {
		return m_pData;
	}

	const float asFloat() const {
		float result;
		memcpy(&result, m_pData, 4);
		return result;
	}

	const double asDouble() const {
		double result;
		memcpy(&result, m_pData, 8);
		return result;
	}

	template<typename T>
	const T interpret_integer() const {
		size_t o = 0;
		if (m_bSignFlag == 0)
			return SQLardUtil::sqlard_read_le<T>(m_pData, o, (m_usLength * 8)) * (-1);
		return SQLardUtil::sqlard_read_le<T>(m_pData, o, (m_usLength * 8));
	}

	
	static SQLardRowFieldData * ParseField(const uint8_t fieldDataType,uint8_t * data, size_t & offset) {
		SQLardRowFieldData * fieldData = new SQLardRowFieldData();
		/*	DATE MUST NOT have a TYPE_VARLEN. The value is either 3 bytes or 0 bytes (null). 
			TIME, DATETIME2, and DATETIMEOFFSET MUST NOT have a TYPE_VARLEN. The lengths are determined by the SCALE as indicated in section 2.2.5.4.2. 
			PRECISION and SCALE MUST occur if the type is NUMERIC, NUMERICN, DECIMAL, or DECIMALN. 
			SCALE (without PRECISION) MUST occur if the type is TIME, DATETIME2, or DATETIMEOFFSET (introduced in TDS 7.3). PRECISION MUST be less than or equal to decimal 38 and SCALE MUST be less than or equal to the precision value. 
			COLLATION occurs only if the type is BIGCHARTYPE, BIGVARCHRTYPE, TEXTTYPE, NTEXTTYPE, NCHARTYPE, or NVARCHARTYPE.*/

		char extraBytes = 0;
		/* These types need null terminator. */
		switch (SQLardDataType(fieldDataType)) 
		{
			/* 2 byte length specifier */
			case SQLardDataType::VARCHARTYPE:
			case SQLardDataType::BIGVARCHRTYPE:
			case SQLardDataType::TEXTTYPE:
			case SQLardDataType::BIGCHARTYPE:
			case SQLardDataType::NTEXTTYPE:
				fieldData->m_usLength = SQLardUtil::sqlard_read_le<uint16_t>(data, offset);
				extraBytes = 1;
				break;
			case SQLardDataType::BIGBINARYTYPE:
			case SQLardDataType::BIGVARBINTYPE:
				fieldData->m_usLength = SQLardUtil::sqlard_read_le<uint16_t>(data, offset);
				break;
			case SQLardDataType::BINARYTYPE:
			case SQLardDataType::VARBINARYTYPE:
			case SQLardDataType::GUIDTYPE:
				/* These are legacy, must read fixed column amount*/
				fieldData->m_usLength = SQLardUtil::sqlard_read_le<uint8_t>(data, offset);
				break;
			/* 1 for INT1TYPE/BITTYPE */
			case SQLardDataType::BITTYPE:
			case SQLardDataType::INT1TYPE:
				fieldData->m_usLength = 1;
				break;
			/* 2 for INT2TYPE */
			case SQLardDataType::INT2TYPE:
				fieldData->m_usLength = 2;
				break;
			/* 4 for INT4TYPE / DATETIM4TYPE / FLT4TYPE / MONEY4TYPE*/
			case SQLardDataType::FLT4TYPE:
			case SQLardDataType::INT4TYPE:
			case SQLardDataType::MONEY4TYPE:
			case SQLardDataType::DATETIM4TYPE:
				fieldData->m_usLength = 4;
				break;
			/*8 for MONEYTYPE/DATETIMETYPE/FLT8TYPE/INT8TYPE*/
			case SQLardDataType::INT8TYPE:
			case SQLardDataType::MONEYTYPE:
			case SQLardDataType::DATETIMETYPE:
			case SQLardDataType::FLT8TYPE:
				fieldData->m_usLength = 8;
				break;
			

				/*PRECISION and SCALE MUST occur if the type is NUMERIC, NUMERICN, DECIMAL, or DECIMALN.*/
			case SQLardDataType::NUMERICTYPE:
			case SQLardDataType::NUMERICNTYPE:
			case SQLardDataType::DECIMALNTYPE:
			case SQLardDataType::DECIMALTYPE:
				fieldData->m_usLength = (SQLardUtil::sqlard_read_le<uint8_t>(data, offset)) -1;
				/* DIGIT COUNT */
				
				fieldData->m_bSignFlag = SQLardUtil::sqlard_read_le<uint8_t>(data, offset);
				break;
				/*GUIDTYPE, BITTYPE, INT1TYPE, INT2TYPE, INT4TYPE, INT8TYPE, DATETIMETYPE, DATETIM4TYPE, FLT4TYPE, FLT8TYPE, MONEYTYPE, MONEY4TYPE, DATENTYPE*/
				/* NOT SPECIFIED*/
				/*TIMENTYPE, DATETIME2NTYPE, DATETIMEOFFSETNTYPE*/
				/*1 byte specifying scale
				/*BIGVARBINTYPE, BIGBINARYTYPE*/
				/*2 bytes specifying max length */
				/*BIGVARCHRTYPE, BIGCHARTYPE, NVARCHARTYPE, NCHARTYPE */
				/*5-byte COLLATION, followed by a 2-byte max length */
			default:
				#ifdef SQLARD_VERBOSE_OUTPUT
					SQLardUtil::printf(F("ParseField() >> Undefined field type !!! %d\n"), fieldDataType);
				#endif 

			break;
		}
		printf("FIELD LEN : %d\n", fieldData->m_usLength);
		fieldData->m_pData = new uint8_t[fieldData->m_usLength + extraBytes];
		memset(fieldData->m_pData, '\0', (fieldData->m_usLength+extraBytes) * sizeof(uint8_t));
		SQLardUtil::sqlard_read_bytes(fieldData->m_pData, data, offset, fieldData->m_usLength);
		return fieldData;
	}
};

class SQLardRowData {
public:
	friend class SQLardTableResult;
	uint16_t m_usFieldCount;
	void allocateFieldArray(const uint16_t len) {
		m_arrFields = new SQLardRowFieldData *[len];
		memset(m_arrFields, 0, len);
		m_usFieldCount = len;
	}

	const SQLardRowFieldData * operator[](const size_t index)const  { 
		if ((signed)index <= (signed)(-1) || index > m_usFieldCount - 1)
			return nullptr;
		return m_arrFields[index];
	}
	

	~SQLardRowData() {
		for (uint16_t i = 0; i < m_usFieldCount; i++) {
			if (m_arrFields[i] == nullptr)
				continue;
			delete m_arrFields[i];
		}
		delete[] m_arrFields;
	}
protected:
	SQLardRowFieldData ** m_arrFields;
};



class SQLardTableResult {
public:
	SQLardColumnData ** m_arColumnData;
	uint16_t m_usColumnCount;

	SQLardRowList<SQLardRowData*>  m_llRows;

	SQLardTableResult() {
		m_arColumnData = nullptr;
	}
	void allocatedColumnArray(const uint16_t count) {
		m_usColumnCount = count;
		m_arColumnData = new SQLardColumnData *[count];
	}
	void appendRowData(SQLardRowData * pRow) {
		m_llRows.Enqueue(pRow);
	}

	SQLardDataType GetColumnDataType(const uint16_t columnIndex) {
		if (columnIndex >= m_usColumnCount) 
			return static_cast<SQLardDataType>(-1);
		return static_cast<SQLardDataType>(m_arColumnData[columnIndex]->m_bType);
	}


	SQLardRowData * GetRow() const {
		if (m_llRows.GetCurrent() != nullptr)
			return m_llRows.GetCurrent()->val;
		return nullptr;
	}
	void MoveNext() {
		m_llRows.MoveNext();
	}
	void ResetIterator() {
		m_llRows.Reset();
	}

	void ParseColumnData(uint8_t * data, size_t & offset) {
		/* Parse column data */
		uint16_t columnCount = SQLardUtil::sqlard_read_le<uint16_t>((uint8_t*)data, offset);
		allocatedColumnArray(columnCount);
		for (uint16_t i = 0; i < columnCount; i++) {
			m_arColumnData[i] = SQLardColumnData::ParseColumnData((uint8_t*)data, offset);
		}
	}

	void ParseRowData( uint8_t * data, size_t & offset) {
		SQLardRowData * pRowData = new SQLardRowData();
		pRowData->allocateFieldArray(m_usColumnCount);
		for (uint16_t i = 0; i < m_usColumnCount; i++) {
			pRowData->m_arrFields[i] = SQLardRowFieldData::ParseField(m_arColumnData[i]->m_bType, (uint8_t*)data, offset);
		}
		appendRowData(pRowData);
	}

	~SQLardTableResult() {
		if (!(nullptr == m_arColumnData))
		{
			for (int i = 0; i < m_usColumnCount; i++)
			{
				delete m_arColumnData[i];
			}
			delete[] m_arColumnData;
		}
	}
};




class SQLardLOGIN7
{
public:
	SQLardLOGIN7() {
		/* Here are the default values */
		m_uiLength = 0;
		m_uiTDSVersion = 0x70000000; /* TDS 7.0 */
		m_uiPacketSize = 4096;
		m_uiClientProgVer = 117440512;
		m_uiConnectionID = 0;
		m_uiClientPID = 256;
		m_uiClientTimeZone = 0x000001e0;
		m_uiClientLCID = 0x00000409;
		m_ubOptionFlags1 = 0xE0;
		m_ubOptionFlags2 = 0x03;
		m_ubOptionFlags3 = 0x00;
		m_ubTypeFlags = 0x00;

		m_wcszUserName = nullptr;
		m_wcszPassword = nullptr;
		m_wcszHost = nullptr;
		m_wcszAppName = nullptr;
		m_wcszServerName = nullptr;
		m_wcszUnused = nullptr;
		m_wcszExtension = nullptr;
		m_wcszCltIntName = nullptr;
		m_wcszLanguage = nullptr;
		m_wcszDatabase = nullptr;
		m_wcszAttachDBFile = nullptr;
		m_wcszChangePassword = nullptr;
		m_wcszSSPI = nullptr;

		SetClientInterfaceName(L"ODBC");
		SetApplicationName(L"SQLARD");
		SQLardUtil::freeRam("l7");
	}
	~SQLardLOGIN7() {
		if (!(nullptr == m_wcszUserName))
			delete[] m_wcszUserName;
		if (!(nullptr == m_wcszPassword))
			delete[] m_wcszPassword;
		if (!(nullptr == m_wcszHost))
			delete[] m_wcszHost;
		if (!(nullptr == m_wcszAppName))
			delete[] m_wcszAppName;
		if (!(nullptr == m_wcszServerName))
			delete[] m_wcszServerName;
		if (!(nullptr == m_wcszUnused))
			delete[] m_wcszUnused;
		if (!(nullptr == m_wcszExtension))
			delete[] m_wcszExtension;
		if (!(nullptr == m_wcszCltIntName))
			delete[] m_wcszCltIntName;
		if (!(nullptr == m_wcszLanguage))
			delete[] m_wcszLanguage;
		if (!(nullptr == m_wcszDatabase))
			delete[] m_wcszDatabase;
		if (!(nullptr == m_wcszAttachDBFile))
			delete[] m_wcszAttachDBFile;
		if (!(nullptr == m_wcszChangePassword))
			delete[] m_wcszChangePassword;
		if (!(nullptr == m_wcszSSPI))
			delete[] m_wcszSSPI;
	}
	void SetLength(const uint32_t val) { m_uiLength = val; }
	void SetTDSVersion(const uint32_t val) { m_uiTDSVersion = val; }
	void SetPacketSize(const uint32_t val) { m_uiPacketSize = val; }
	void SetClientProgVer(const uint32_t val) { m_uiClientProgVer = val; }
	void SetClientPID(const uint32_t val) { m_uiClientPID = val; }
	void SetConnectionID(const uint32_t val) { m_uiConnectionID = val; }
	void SetClientTimeZone(const uint32_t val) { m_uiClientTimeZone = val; }
	/* collation */
	void SetClientLCID(const uint32_t val) { m_uiClientLCID = val; }
	void SetOptionFlags1(const uint8_t val) { m_ubOptionFlags1 = val; }
	void SetOptionFlags2(const uint8_t val) { m_ubOptionFlags2 = val; }
	void SetOptionFlags3(const uint8_t val) { m_ubOptionFlags3 = val; }
	void SetTypeFlags(const uint8_t val) { m_ubTypeFlags = val; }

	/* Login related */
	void SetUserName(const wchar_t * wcszUserName) {
		m_wcszUserName = SQLardUtil::sqlard_alloc_wstr(wcszUserName);
	};
	void SetPassword(const wchar_t * wcszPassword) {
		m_wcszPassword = SQLardUtil::sqlard_alloc_wstr(wcszPassword);
	};
	void SetHost(const wchar_t * wcszHost) {
		m_wcszHost = SQLardUtil::sqlard_alloc_wstr(wcszHost);
	};
	void SetApplicationName(const wchar_t * wcszAppName) {
		m_wcszAppName = SQLardUtil::sqlard_alloc_wstr(wcszAppName);
	};
	void SetServerName(const wchar_t * wcszServerName) {
		m_wcszServerName = SQLardUtil::sqlard_alloc_wstr(wcszServerName);
	};
	void SetExtension(const wchar_t * wcszExtension) {
		m_wcszExtension = SQLardUtil::sqlard_alloc_wstr(wcszExtension);
	};
	void SetClientInterfaceName(const wchar_t * wcszCltIntName) {
		m_wcszCltIntName = SQLardUtil::sqlard_alloc_wstr(wcszCltIntName);
	};
	/* Initial language (overrides user's default language) */
	void SetLanguage(const wchar_t * wcszLanguage) {
		m_wcszLanguage = SQLardUtil::sqlard_alloc_wstr(wcszLanguage);
	};
	/* Initial database (overrides user's default database) */
	void SetDatabase(const wchar_t * wcszDatabase) {
		m_wcszDatabase = SQLardUtil::sqlard_alloc_wstr(wcszDatabase);
	};
	void SetAttachDatabaseFile(const wchar_t * wcszAttachDBFile) {
		m_wcszAttachDBFile = SQLardUtil::sqlard_alloc_wstr(wcszAttachDBFile);
	};
	void SetChangePassword(const wchar_t * wcszChangePassword) {
		m_wcszChangePassword = SQLardUtil::sqlard_alloc_wstr(wcszChangePassword);
	};

	size_t FillBuffer(uint8_t * buf)
	{
		size_t offset = 0;
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiLength);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiTDSVersion);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiPacketSize);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiClientProgVer);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiClientPID);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiConnectionID);
		SQLardUtil::sqlard_write_le<uint8_t>(buf, offset, m_ubOptionFlags1);
		SQLardUtil::sqlard_write_le<uint8_t>(buf, offset, m_ubOptionFlags2);
		SQLardUtil::sqlard_write_le<uint8_t>(buf, offset, m_ubTypeFlags);
		SQLardUtil::sqlard_write_le<uint8_t>(buf, offset, m_ubOptionFlags3);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiClientTimeZone);
		SQLardUtil::sqlard_write_le<uint32_t>(buf, offset, m_uiClientLCID);
		FillStringTable(buf, offset);
		/* Now we need to adjust the size. */
		size_t temp_offset = 0;
		SQLardUtil::sqlard_write_le<uint32_t>(buf, temp_offset, offset);
		SQLardUtil::freeRam("fb");
		return offset;
	}
	size_t FillStringTable(uint8_t * buf, size_t & offset)
	{
		size_t table_size = 0;
		table_size += SQLardUtil::sqlard_wcslen(m_wcszHost);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszUserName);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszPassword);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszAppName);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszServerName);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszUnused);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszCltIntName);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszLanguage);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszDatabase);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszAttachDBFile);
		table_size += SQLardUtil::sqlard_wcslen(m_wcszChangePassword);
		/*
		Allocate required space for the table
		*/
		const size_t offset_table_size = 58;
	
		SQLardBuffer<uint8_t> table_buffer(table_size * 2);
		
		
		uint8_t offset_buffer[offset_table_size];
		size_t table_offset = 0;
		size_t entry_offsets[12];
		size_t entry_lengths[12];
		size_t current_index = 0;

		wchar_t * current_wstring = nullptr;
		for (int i = 0; i <12; i++)
		{
			switch (i)
			{
			case 0: current_wstring = m_wcszHost;           break;
			case 1: current_wstring = m_wcszUserName;       break;
			case 2: current_wstring = m_wcszPassword;       break;
			case 3: current_wstring = m_wcszAppName;        break;
			case 4: current_wstring = m_wcszServerName;     break;
			case 5: current_wstring = m_wcszUnused;         break;
			case 6: current_wstring = m_wcszCltIntName;     break;
			case 7: current_wstring = m_wcszLanguage;       break;
			case 8: current_wstring = m_wcszDatabase;       break;
			case 9: current_wstring = m_wcszSSPI;          break;
			case 10: current_wstring = m_wcszAttachDBFile;   break;
			case 11: current_wstring = m_wcszChangePassword; break;
			}
			SQLardUtil::sqlard_wstrcpy(&table_buffer[table_offset], current_wstring);
			if (i == 2)
			{
				for (size_t i = table_offset; i < (table_offset + SQLardUtil::sqlard_wcslen(current_wstring) * 2); i++)
				{
					table_buffer[i] = (((table_buffer[i] & 0x0F) << 4 | (table_buffer[i] & 0xF0) >> 4) ^ 0xA5);
				}
			}
			entry_offsets[i] = table_offset + offset + offset_table_size;
			entry_lengths[i] = SQLardUtil::sqlard_wcslen(current_wstring);
			table_offset += entry_lengths[i] * 2;
			//wprintf(L"index [%d] >> table offset [%d], string length [%d], string : [%s]\n", i, table_offset, entry_lengths[i], current_wstring);
		}
		/*if (table_offset > table_size * 2)
		{
		wprintf(L"EXCEEDED STRING TABLE\n");
		}*/
		size_t obuf_offset = 0;
		for (int i = 0; i < 9; i++) {
			SQLardUtil::sqlard_write_le<uint16_t>(offset_buffer, obuf_offset, entry_offsets[i]);
			SQLardUtil::sqlard_write_le<uint16_t>(offset_buffer, obuf_offset, entry_lengths[i]);
		}
		/* Client ID */
		uint8_t clientID[] = { 0x00,0x50,0x8B,0xE2,0xB7,0x8F };
		memcpy(&offset_buffer[obuf_offset], clientID, sizeof(clientID));
		obuf_offset += 6;
		for (int i = 9; i < 12; i++) {
			SQLardUtil::sqlard_write_le<uint16_t>(offset_buffer, obuf_offset, entry_offsets[i]);
			SQLardUtil::sqlard_write_le<uint16_t>(offset_buffer, obuf_offset, entry_lengths[i]);
		}
		/* SSPI long */
		SQLardUtil::sqlard_write_le<uint32_t>(offset_buffer, obuf_offset, 0);
		/* Place offset table */
		memcpy(&buf[offset], offset_buffer, offset_table_size);
		offset += offset_table_size;
		memcpy(&buf[offset], table_buffer(), table_size * 2);
		offset += table_size * 2;
		SQLardUtil::freeRam("fs");
		return table_size;
	}
private:
	uint32_t m_uiLength;
	uint32_t m_uiTDSVersion;
	uint32_t m_uiPacketSize;
	uint32_t m_uiClientProgVer;
	uint32_t m_uiClientPID;
	uint32_t m_uiConnectionID;
	uint32_t m_uiClientTimeZone;
	uint32_t m_uiClientLCID;
	uint8_t m_ubOptionFlags1;
	uint8_t m_ubOptionFlags2;
	uint8_t m_ubOptionFlags3;
	uint8_t m_ubTypeFlags;

	wchar_t * m_wcszUserName;
	wchar_t * m_wcszPassword;
	wchar_t * m_wcszHost;
	wchar_t * m_wcszAppName;
	wchar_t * m_wcszServerName;
	wchar_t * m_wcszUnused;
	wchar_t * m_wcszExtension;
	wchar_t * m_wcszCltIntName;
	wchar_t * m_wcszLanguage;
	wchar_t * m_wcszDatabase;
	wchar_t * m_wcszAttachDBFile;
	wchar_t * m_wcszChangePassword;
	wchar_t * m_wcszSSPI;

};



class SQLard
{
public:
	
	#ifdef WINDOWS
		SQLard(uint8_t * serverIP, const uint16_t port):resolver(io_service),socket(io_service) {
			memcpy(m_arrServerIPv4, serverIP, 6);
			m_usPort = port;
			m_pLogin7 = nullptr;
			m_bConnected = false;
			m_bLoggedIn = false;
			m_uiPacketIndex = 0;
		}
		bool connect() {
			long longIP = m_arrServerIPv4[0] << 24 | m_arrServerIPv4[1] << 16 | m_arrServerIPv4[2] <<8| m_arrServerIPv4[3] << 0;
 			boost::system::error_code error = boost::asio::error::host_not_found;
			boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> endP(boost::asio::ip::address_v4(longIP), m_usPort);
			socket.connect(endP, error);
			m_bConnected = (error == 0);
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(m_bConnected ? F("SQLARD > connect : MSSQL connection successfully established!\n") : F("SQLARD > connect : MSSQL connection failed!\n"));
			#endif
			return m_bConnected;
		}
	#else
		SQLard(uint8_t * serverIP, const uint16_t port, EthernetClient * pEthCl) {
			setServer(serverIP, port, pEthCl);
			m_pLogin7 = nullptr;
			m_bConnected = false;
			m_bLoggedIn = false;
			m_uiPacketIndex = 0;
		}
		SQLard() {
			m_pLogin7 = nullptr;
			m_bConnected = false;
			m_bLoggedIn = false;
			m_uiPacketIndex = 0;
		}
		void setServer(uint8_t * serverIP, const uint16_t port, EthernetClient * pEthCl) {
			memcpy(m_arrServerIPv4, serverIP, 6);
			m_usPort = port;
			m_pEthClient = pEthCl;
		}
		bool connect() {
			const int max_retry_count = 10;
			int current_retry = 0;
			do
			{
				delay(1000);
				m_bConnected = m_pEthClient->connect(m_arrServerIPv4, m_usPort);
			} while (current_retry++ < max_retry_count && !m_bConnected);
			#ifdef SQLARD_VERBOSE_OUTPUT
				Serial.println(m_bConnected ? F("SQLARD > connect : MSSQL connection successfully established!") : F("SQLARD > connect : MSSQL connection failed!"));
			#endif
			return m_bConnected;
		}

		/* Maintain the database connection. */
		void maintain() {
			while (!m_pEthClient->connected()) {
				Serial.println("retry connect");
				/* Baðlanana kadar dene. */
				if (connect()) {
					login();
				}
			}
		}
	#endif
	
	void setCredentials(const wchar_t * wcszdbName, const wchar_t * wcszUserName, const wchar_t * wcszPassword, const wchar_t *wcszHost) {
		if (m_pLogin7)
			delete m_pLogin7;
		m_pLogin7 = new SQLardLOGIN7();
		m_pLogin7->SetUserName(wcszUserName);
		m_pLogin7->SetPassword(wcszPassword);
		m_pLogin7->SetHost(wcszHost);
		m_pLogin7->SetDatabase(wcszdbName);
	}

	bool login() {
		if (!m_bConnected || !m_pLogin7)
		{
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(F("SQLARD > login : Not connected or no login structure!"));
			#endif
			return false;
		}
		uint8_t data[256] PROGMEM;
		//SQLardBuffer<uint8_t>data(512);
		size_t len = m_pLogin7->FillBuffer(data);
		//delete m_pLogin7;
		sendTDSPacket(0x10, data, len);
		return m_bLoggedIn;
	}

	/*
		Execute a INSERT, UPDATE or DELETE query.
		Returns affected row count.
	*/
	long executeNonQuery(const wchar_t* query) {
		{
			sendTDSPacket(0x01, (uint8_t*)query, SQLardUtil::sqlard_wcslen(query) * 2);
		}
		return m_uiDoneCount;
	}
	SQLardTableResult *  executeReader(const wchar_t* query) {
		{

			sendTDSPacket(0x01, (uint8_t*)query, SQLardUtil::sqlard_wcslen(query) * 2, false);
			SQLardUtil::freeRam("execreader");
		}
		SQLardUtil::freeRam("zzzz");
		return waitRowData();
	}
protected:
	void putTDSHeader(uint8_t * buf, const uint8_t opcode, const uint8_t status)
	{
		buf[0] = opcode;
		buf[1] = status;
		/* SPID */
		memset(&buf[4], 0, 2);
		/* Packet ID */
		buf[6] = static_cast<uint8_t>(((m_uiPacketIndex++) % 256));
		/* Window */
		buf[7] = 0;
	}
	void putTDSLength(uint8_t * buf, const uint16_t totalLen)
	{
		/* copy size in big endian order  */
		memset(&buf[2], reinterpret_cast<const uint8_t*>(&totalLen)[1], 1);
		memset(&buf[3], reinterpret_cast<const uint8_t*>(&totalLen)[0], 1);
	}
	void putTDSData(uint8_t * buf, uint8_t * data, const uint16_t dataSize)
	{
		memcpy(&buf[8], data, dataSize);
		putTDSLength(buf, dataSize + 8);
	}
	const uint16_t readTDSPacketSize(uint8_t * header)
	{
		return (static_cast<uint16_t>(header[2]) << 8) | header[3];
	}
	bool sendToServer(uint8_t * buf, const uint16_t len)
	{
		#ifndef WINDOWS
		int wCount = m_pEthClient->write(buf, len);
		m_pEthClient->flush();
		return wCount == len;
		#else
			boost::system::error_code ignored_error;
			size_t wcount =boost::asio::write(socket, boost::asio::buffer(buf, len), boost::asio::transfer_all(), ignored_error);
			return wcount == len;
		#endif
	}
	void sendTDSPacket(uint8_t opcode, uint8_t *data,const uint16_t len, bool bWaitResponse = true)
	{
		{
			SQLardBuffer<uint8_t>buf(len + 8);
			putTDSHeader(buf(), opcode, 0x01);
			putTDSData(buf(), data, len);
			sendToServer(buf(), buf.alloc_size());
		}
		if (bWaitResponse)
			waitResponse(opcode);
	}

	int waitData()
	{
		int num = 0;
		int timeout = 0;
		do {
			#ifndef WINDOWS
			num = m_pEthClient->available();
			#else
			num = socket.available();
			#endif
			timeout++;
			if (num < 8 && timeout < 5000) {
				//delay(100);  // adjust for network latency
			}
		} while (num < 8 && timeout < 5000);
		return num;
	}

	SQLardTableResult * waitRowData() {
		uint8_t header[8];
		int available = 0;
		//SQLardUtil::freeRam("abc");

		while (available < 8)
			available = waitData();
		#ifndef WINDOWS
			for (int i = 0; i < 8; i++) {
				header[i] = m_pEthClient->read();
			}
		#else	
			boost::asio::read(socket, boost::asio::buffer(header, 8));
		#endif
		int dataSize = readTDSPacketSize(header) - 8;
		if (dataSize <= 0) {
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(F("SQLARD > waitResponse : Invalid data length!"));
			#endif
			return nullptr;
		}
		/* allocate some buffer for data part */
	
		SQLardBuffer<uint8_t>data(dataSize);
	

		while (available < dataSize)
			available = waitData();
		#ifndef WINDOWS
			for (int i = 0; i < dataSize; i++) {
				data[i] = m_pEthClient->read();
			}
		#else
			size_t read_amount = boost::asio::read(socket, boost::asio::buffer(data, dataSize));
		#endif


		uint8_t optionToken = 0;
		uint16_t optionLength = 0;
		size_t readPos = 0;
		SQLardUtil::freeRam("btr");
		SQLardTableResult * pTableResult = new SQLardTableResult();
		while (true) {
			if (readPos > dataSize)
				break;
			optionToken = data[readPos++];
			
			switch (optionToken) {
			case 0x81: /* COLMETADATA */
				pTableResult->ParseColumnData(data(), readPos);
				SQLardUtil::freeRam("aftercd");
				break;
			case 0xD1:
				pTableResult->ParseRowData(data(), readPos);
				SQLardUtil::freeRam("afterrd");
				break;
			case 0xFF: /* DONEINPROC */
				break;
			case 0xFD: /* DONE */
			case 0xFE: /* DONEPROC */
				if (parseDone(data(), readPos))
					return pTableResult;
				break;
			case 0xAA: /* Error */
			case 0xAB: /* info */
				parseInformationMessage(data(), readPos);
				break;
			default:
				printf("unknown token %d\n", optionToken);
				break;
			}

		}
		#ifdef SQLARD_VERBOSE_OUTPUT
			SQLardUtil::printf(F("waitRowData() >> Unexpected return!"));
		#endif
			SQLardUtil::freeRam("wrd");
		return pTableResult;
	}

	

	void waitResponse(uint8_t sent_opcode)
	{
		uint8_t header[8];
		int available = 0;
		while (available < 8)
			available = waitData();

		#ifndef WINDOWS
		for (int i = 0; i < 8; i++) {
			header[i] = m_pEthClient->read();
		}
		#else	
		boost::asio::read(socket, boost::asio::buffer(header, 8));
		#endif
		int dataSize = readTDSPacketSize(header) - 8;
		if (dataSize <= 0) {
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(F("SQLARD > waitResponse : Invalid data length!"));
			#endif
			return;
		}
		/* allocate some buffer for data part */
		#ifndef SQLARD_VERBOSE_OUTPUT
				m_bLoggedIn = true;
				return;
		#endif
		SQLardBuffer<uint8_t>data(dataSize);

		while (available < dataSize)
			available = waitData();
		#ifndef WINDOWS
			for (int i = 0; i < dataSize; i++) {
				data[i] = m_pEthClient->read();
			}
		#else
			boost::asio::read(socket, boost::asio::buffer(data, dataSize));
		#endif
		switch (sent_opcode) {
		case 0x01:
		case 0x10:
			parseTokens(data());
			break;
		}
	}



	void parseTokens(uint8_t * data)
	{
		uint8_t optionToken = 0;
		uint16_t optionLength = 0;
		size_t readPos = 0;
		while (true) {
			optionToken = data[readPos++];

			switch (optionToken) {
			case 0xE3: /* EnvChange */
				parseEnvChange(data, readPos);
				break;
			case 0xAA: /* Error */
			case 0xAB: /* info */
				parseInformationMessage(data, readPos);
				break;
			case 0xAD: /* loginack */
				parseLoginAcknowledgement(data, readPos);
				break;

			case 0xFF: /* DONEINPROC */
				break;
			case 0xA5: /* COLINFO */
				break;
			case 0xFD: /* DONE */
			case 0xFE: /* DONEPROC */
				if (parseDone(data, readPos))
					return;
				break;
			default:
				
				break;
			}
		}
	}

	bool parseDone(uint8_t * data, size_t &readPos)
	{

		m_usDoneStatus = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);

		m_usDoneCurCmd = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);
		m_uiDoneCount = (long)SQLardUtil::sqlard_read_le<uint32_t>(data, readPos);
		/* is this last done token ? */
		if ((m_usDoneStatus & (1 << 0)) != 0)
			return false;
		return true;

	}

	void parseLoginAcknowledgement(uint8_t * data, size_t &readPos)
	{
		uint16_t tokenLength = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);
		#ifndef SQLARD_VERBOSE_OUTPUT
			readPos += tokenLength;
		#else
			uint8_t interface = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			uint32_t tdsVersion = SQLardUtil::sqlard_read_le<uint32_t>(data, readPos);
			uint8_t progName [64];
			uint8_t progNameLen = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			SQLardUtil::sqlard_rwstr_mb(progName, data, readPos, progNameLen);
			uint8_t majorVer = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			uint8_t minorVer = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			uint8_t buildHi = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			uint8_t buildLo = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(F("SQLARD > login_ack : Server [%s], version %d.%d.%d.%d, TDS ver.%d\n"),
					progName, majorVer, minorVer, buildHi, buildLo, tdsVersion);
				SQLardUtil::printf(F("SQLARD > login_ack : Successfully logged in!\n"));
			#endif
		#endif
		m_bLoggedIn = true;
	}

	void parseInformationMessage(uint8_t * data, size_t &readPos)
	{
		uint16_t tokenLength = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);
		#ifndef SQLARD_VERBOSE_OUTPUT
			readPos += tokenLength;
		#else
			long errorNumber = (long)SQLardUtil::sqlard_read_le<uint32_t>(data, readPos);
			uint8_t state = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			uint8_t severity = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			uint16_t messageLen = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);
			uint8_t message[128], server[32], proc[32];
			SQLardUtil::sqlard_rwstr_mb(message, data, readPos, messageLen);
			uint8_t servNameLen = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			SQLardUtil::sqlard_rwstr_mb(server, data, readPos, servNameLen);
			uint8_t procNameLen = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
			SQLardUtil::sqlard_rwstr_mb(proc, data, readPos, procNameLen);
			uint16_t lineNumber = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);
			#ifdef SQLARD_VERBOSE_OUTPUT
				SQLardUtil::printf(F("SQLARD > information : Message %d (severity %d, state %d)"), errorNumber, severity, state);
				SQLardUtil::printf(F(", from %s, line number %d\n"), server, lineNumber);
				SQLardUtil::printf(F("\t\t%s\n"), message);
			#endif
		#endif
	}

	void parseEnvChange(uint8_t * data, size_t & readPos)
	{
		uint16_t tokenLength = SQLardUtil::sqlard_read_le<uint16_t>(data, readPos);
		#ifndef SQLARD_VERBOSE_OUTPUT
			readPos += tokenLength;
		#else
			readPos += 2;
			uint8_t envChangeType = data[readPos++];
			switch (envChangeType) {
			case 0x01: /* Database */
			{
				uint8_t newdb[32]PROGMEM, olddb[32]PROGMEM;
				uint8_t newValueLength = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
				SQLardUtil::sqlard_rwstr_mb(newdb, data, readPos, newValueLength);
				uint8_t oldValueLength = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
				SQLardUtil::sqlard_rwstr_mb(olddb, data, readPos, oldValueLength);
				#ifdef SQLARD_VERBOSE_OUTPUT
					SQLardUtil::printf(F("SQLARD > Environment change : Changed database context from '%s' to '%s'.\n"), newdb, olddb);
				#endif
			}
			break;
			case 0x02: /* language */
			case 0x04: /* packet size*/
			{
				uint8_t newlang[128]PROGMEM, oldlang[128]PROGMEM;
				uint8_t newValueLength = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
				SQLardUtil::sqlard_rwstr_mb(newlang, data, readPos, newValueLength);
				uint8_t oldValueLength = SQLardUtil::sqlard_read_le<uint8_t>(data, readPos);
				SQLardUtil::sqlard_rwstr_mb(oldlang, data, readPos, oldValueLength);
				#ifdef SQLARD_VERBOSE_OUTPUT
					SQLardUtil::printf(F("SQLARD > Environment change : %s changed from '%s' to '%s'.\n"), envChangeType == 0x02 ? F("Language") : F("Packet size"), oldlang, newlang);
				#endif
			}
			break;

			case 0x07: /* collation */
			{
				uint8_t newValueLength = data[readPos++];
				uint16_t codepage, flags;
				uint8_t charsetid;
				codepage = (static_cast<uint16_t>(data[readPos]) << 8) | data[readPos + 1];
				readPos += 2;
				flags = (static_cast<uint16_t>(data[readPos]) << 8) | data[readPos + 1];
				readPos += 2;
				charsetid = data[readPos++];
				uint8_t oldValueLength = data[readPos++];
				#ifdef SQLARD_VERBOSE_OUTPUT
					SQLardUtil::printf(F("SQLARD > Environment change : Collation change received (CP : %d, Flags : %d, Charset ID : %d)\n"), codepage, flags, charsetid);
				#endif
			}
			break;
			}
		#endif
	}
private:
	bool m_bConnected;
	bool m_bLoggedIn;
	uint8_t m_arrServerIPv4[6];
	uint16_t m_usPort;
	#ifndef WINDOWS
		EthernetClient * m_pEthClient;
	#else
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::resolver resolver;
		boost::asio::ip::tcp::socket socket;
	#endif
	SQLardLOGIN7 * m_pLogin7;
	uint32_t m_uiPacketIndex;

	uint32_t m_uiDoneCount;
	uint16_t m_usDoneStatus;
	uint16_t m_usDoneCurCmd;
};


#endif

