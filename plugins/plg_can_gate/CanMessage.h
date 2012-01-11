/*
 * CanMessage.h
 *
 */
//
#ifndef CANMESSAGE_H_
#define CANMESSAGE_H_

#include <string.h>

#ifdef WIN32

#define uint8_t		unsigned char
#define uint16_t	unsigned short
#define uint32_t	unsigned int
#define uint64_t	unsigned long long

#define int8_t		char
#define int16_t		short
#define int32_t		int
#define int64_t		long long

#pragma pack(push, 1)

#endif

//==============================================================================

// Structure of base CAN message
struct TCanMessage{
	uint8_t		m_nExtFrameFormat;
	uint8_t		m_nRemoteRequest;
/*
	// arbitration field
	// standart frame:
	// m_aFrameID[0] bit    7    6    5    4    3    2    1    0
	// arbitration bit   id10 id09 id08 id07 id06 id05 id04 id03
	// m_aFrameID[1] bit    7    6    5    4    3    2    1    0
	// arbitration bit   id02 id01 id00

	// extended frame:
	// m_aFrameID[0] bit    7    6    5    4    3    2    1    0
	// arbitration bit   id28 id27 id26 id25 id24 id23 id22 id21
	// m_aFrameID[1] bit    7    6    5    4    3    2    1    0
	// arbitration bit   id20 id19 id18 id17 id16 id15 id14 id13
	// m_aFrameID[2] bit    7    6    5    4    3    2    1    0
	// arbitration bit   id12 id11 id10 id09 id08 id07 id06 id05
	// m_aFrameID[3] bit    7    6    5    4    3    2    1    0
	// arbitration bit   id04 id03 id02 id01 id00
*/
	uint8_t 	m_aFrameID[4];
	uint8_t		m_nDataSize;
	uint8_t		m_aData[8];

#ifdef SEND_STATISTIC
	uint64_t	m_nCreateTime;
#endif	//	SEND_STATISTIC

	TCanMessage(){
		memset(this, 0, sizeof(TCanMessage));
	}

	TCanMessage(TCanMessage *p){
		if(0 != p)
		memcpy(this, p, sizeof(TCanMessage));
	}

	TCanMessage(uint32_t nArbitration, uint8_t nExtFrameFormat, uint8_t nRemoteRequest, uint8_t nDataSize, const uint8_t* pData = 0)
	 : m_nExtFrameFormat(nExtFrameFormat)
	 , m_nRemoteRequest(nRemoteRequest)
	 , m_nDataSize(nDataSize)
	{
		if(nExtFrameFormat) {
			m_aFrameID[0] = nArbitration >> 21;
			m_aFrameID[1] = nArbitration >> 13;
			m_aFrameID[2] = nArbitration >> 5;
			m_aFrameID[3] = nArbitration << 3;
		} else {
			m_aFrameID[0] = nArbitration >> 3;
			m_aFrameID[1] = nArbitration << 5;
		}

		if(nDataSize && pData) {
			memcpy(m_aData, pData, m_nDataSize);
		}
	}

	int Arbitration() const
	{
		if(m_nExtFrameFormat) {
			return uint32_t((m_aFrameID[0] << 21) | (m_aFrameID[1] << 13) | (m_aFrameID[2] << 5) | (m_aFrameID[3] >> 3));
		}

		return uint32_t((m_aFrameID[0] << 3) | (m_aFrameID[1] >> 5));
	}

	bool operator<(const TCanMessage& a) const
	{
		if(m_aFrameID[0] < a.m_aFrameID[0]){
			return true;
		}

		if(m_aFrameID[0] == a.m_aFrameID[0]){
			if(m_aFrameID[1] < a.m_aFrameID[1]){
				return true;
			}

			if(m_aFrameID[1] == a.m_aFrameID[1]){
				if(m_aFrameID[2] < a.m_aFrameID[2]){
					return true;
				}

				if(m_aFrameID[2] == a.m_aFrameID[2]){
					return (m_aFrameID[3] < a.m_aFrameID[3]);
				}
			}
		}

		return false;
	} // end of bool operator<(TCanMessage a)

}
#ifdef __GNUC__
__attribute__ ((packed))
#endif

#ifdef WIN32
#pragma pack(pop)
#endif
;

//==============================================================================

#endif /* CANMESSAGE_H_ */
