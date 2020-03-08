/*!
 *  @file		BLEMIDI.h
 */

#pragma once

#include "utility/Logging.h"

#include <MIDI.h>
using namespace MIDI_NAMESPACE;

#include "BLE-MIDI_Settings.h"
#include "BLE-MIDI_Defs.h"
#include "BLE-MIDI_Namespace.h"

BEGIN_BLEMIDI_NAMESPACE

template<class T, class _Settings = DefaultSettings>
class BLEMIDI
{
    typedef _Settings Settings;

private:
    byte mRxBuffer[Settings::MaxBufferSize];
    unsigned mRxIndex = 0;

    byte mTxBuffer[Settings::MaxBufferSize];
    unsigned mTxIndex = 0;
    
    char mDeviceName[24];
    
private:
	T mBleClass;

public:
	BLEMIDI(const char* deviceName)
	{
        strncpy(mDeviceName, deviceName, 24);
        
        mRxIndex = 0;
        mTxIndex = 0;
	}

    void begin(MIDI_NAMESPACE::Channel inChannel = 1)
    {
        mBleClass.begin(mDeviceName, this);
    }

    bool beginTransmission(MidiType)
    {
        getMidiTimestamp(&mTxBuffer[0], &mTxBuffer[1]);
        mTxIndex = 2;
        
        return true;
    }
    
    void write(byte inData)
    {
        // check for size! SysEx!!!
        if (false)
        {
            // should only happen from SysEx!
            // if we approach the end of the buffer, chop-up in segments until
            // we reach F7 (end of SysEx)
        }
        
        mTxBuffer[mTxIndex++] = inData;
    }

    void endTransmission()
    {
        mBleClass.write(mTxBuffer, mTxIndex);
        mTxIndex = 0;
    }
 
    unsigned available()
    {
        return mRxIndex;
    }
    
    byte read()
    {
        return mRxBuffer[--mRxIndex];
    }

private:
    void reverse(byte arr[], int n)
    {
        for (int low = 0, high = n - 1; low < high; low++, high--)
        {
            int temp = arr[low];
            arr[low] = arr[high];
            arr[high] = temp;
        }
    }
    
public:
    /*
    The general form of a MIDI message follows:
    n-byte MIDI Message
    Byte 0            MIDI message Status byte, Bit 7 is Set to 1.
    Bytes 1 to n-1    MIDI message Data bytes, if n > 1. Bit 7 is Set to 0
    There are two types of MIDI messages that can appear in a single packet: full MIDI messages and
    Running Status MIDI messages. Each is encoded differently.
    A full MIDI message is simply the MIDI message with the Status byte included.
    A Running Status MIDI message is a MIDI message with the Status byte omitted. Running Status
    MIDI messages may only be placed in the data stream if the following criteria are met:
    1.  The original MIDI message is 2 bytes or greater and is not a System Common or System
    Real-Time message.
    2.  The omitted Status byte matches the most recently preceding full MIDI message’s Status
    byte within the same BLE packet.
    In addition, the following rules apply with respect to Running Status:
    1.  A Running Status MIDI message is allowed within the packet after at least one full MIDI
    message.
    2.  Every MIDI Status byte must be preceded by a timestamp byte. Running Status MIDI
    messages may be preceded by a timestamp byte. If a Running Status MIDI message is not
    preceded by a timestamp byte, the timestamp byte of the most recently preceding message
    in the same packet is used.
    3.  System Common and System Real-Time messages do not cancel Running Status if
    interspersed between Running Status MIDI messages. However, a timestamp byte must
    precede the Running Status MIDI message that follows.
    4.  The end of a BLE packet does cancel Running Status.
    In the MIDI 1.0 protocol, System Real-Time messages can be sent at any time and may be
    inserted anywhere in a MIDI data stream, including between Status and Data bytes of any other
    MIDI messages. In the MIDI BLE protocol, the System Real-Time messages must be deinterleaved
    from other messages – except for System Exclusive messages.
    */
    void receive(byte* buffer, size_t bufferSize)
    {
        size_t index = 0;
        size_t i = 0;

        N_DEBUG_PRINT("Received (");
        N_DEBUG_PRINT(bufferSize);
        N_DEBUG_PRINTLN(") :");
        for (int j = 0; j < bufferSize; j++) {
            N_DEBUG_PRINT("0x");
            N_DEBUG_PRINT(buffer[j], HEX);
            N_DEBUG_PRINT(" ");
        }
        N_DEBUG_PRINTLN();
        N_DEBUG_PRINTLN();
    
        // Pointers used to search through payload.
        uint8_t lPtr = 0;
        uint8_t rPtr = 0;
        
        // lastStatus used to capture runningStatus
        uint8_t lastStatus;
        
        // Decode first packet -- SHALL be "Full MIDI message"
        lPtr = 2; //Start at first MIDI status -- SHALL be "MIDI status"
        
        // While statement contains incrementing pointers and breaks when buffer size exceeded.
        while (true)
        {
            lastStatus = buffer[lPtr];
            if( (buffer[lPtr] < 0x80) )
                break; // Status message not present, bail
            
            // Point to next non-data byte
            rPtr = lPtr;
            while ((buffer[rPtr + 1] < 0x80)
            &&     (rPtr < (bufferSize - 1)) )
                rPtr++;
            
            // look at l and r pointers and decode by size.
            if( rPtr - lPtr < 1 )
            {
                mRxBuffer[i++] = lastStatus;
            }
            else if( rPtr - lPtr < 2 ) {
                mRxBuffer[i++] = lastStatus;
                mRxBuffer[i++] = buffer[lPtr + 1];
            }
            else if( rPtr - lPtr < 3 ) {
                mRxBuffer[i++] = lastStatus;
                mRxBuffer[i++] = buffer[lPtr + 1];
                mRxBuffer[i++] = buffer[lPtr + 2];
            }
            else
            {
/*                //Too much data
                //If not System Common or System Real-Time, send it as running status
                switch( buffer[lPtr] & 0xF0 )
                {
                    case NoteOff:
                    case NoteOn:
                    case AfterTouchPoly:
                    case ControlChange:
                    case PitchBend:
                        for(int i = lPtr; i < rPtr; i = i + 2)
                            sendMIDI(lastStatus, buffer[i + 1], buffer[i + 2]);
                        break;
                    case ProgramChange:
                    case AfterTouchChannel:
                        for(int i = lPtr; i < rPtr; i = i + 1)
                            sendMIDI(lastStatus, buffer[i + 1]);
                        break;
                    default:
                        break;
                }
*/
            }
            
            // Point to next status
            lPtr = rPtr + 2;

            if (lPtr >= bufferSize)
                break; //end of packet
        }

        
        
        
        
        
        
        
        
        
        
        
        reverse(mRxBuffer, i);
        
        N_DEBUG_PRINT("mRxBuffer (");
        N_DEBUG_PRINT(i);
        N_DEBUG_PRINTLN(") :");
        for (int j = 0; j < i; j++) {
            N_DEBUG_PRINT("0x");
            N_DEBUG_PRINT(mRxBuffer[j], HEX);
            N_DEBUG_PRINT(" ");
        }
        N_DEBUG_PRINTLN();
        N_DEBUG_PRINTLN();
        
        mRxIndex = i;
    }

protected:
    /*
     The first byte of all BLE packets must be a header byte. This is followed by timestamp bytes and MIDI messages.
     
     Header Byte
     bit 7     Set to 1.
     bit 6     Set to 0. (Reserved for future use)
     bits 5-0  timestampHigh:Most significant 6 bits of timestamp information.
     The header byte contains the topmost 6 bits of timing information for MIDI events in the BLE
     packet. The remaining 7 bits of timing information for individual MIDI messages encoded in a
     packet is expressed by timestamp bytes.

     Timestamp Byte
     bit 7       Set to 1.
     bits 6-0    timestampLow: Least Significant 7 bits of timestamp information.
     
     The 13-bit timestamp for the first MIDI message in a packet is calculated using 6 bits from the
     header byte and 7 bits from the timestamp byte.
     
     Timestamps are 13-bit values in milliseconds, and therefore the maximum value is 8,191 ms.
     Timestamps must be issued by the sender in a monotonically increasing fashion.
     timestampHigh is initially set using the lower 6 bits from the header byte while the timestampLow is
     formed of the lower 7 bits from the timestamp byte. Should the timestamp value of a subsequent
     MIDI message in the same packet overflow/wrap (i.e., the timestampLow is smaller than a
     preceding timestampLow), the receiver is responsible for tracking this by incrementing the
     timestampHigh by one (the incremented value is not transmitted, only understood as a result of the
     overflow condition).
     
     In practice, the time difference between MIDI messages in the same BLE packet should not span
     more than twice the connection interval. As a result, a maximum of one overflow/wrap may occur
     per BLE packet.
     
     Timestamps are in the sender’s clock domain and are not allowed to be scheduled in the future.
     Correlation between the receiver’s clock and the received timestamps must be performed to
     ensure accurate rendering of MIDI messages, and is not addressed in this document.
     */

    /*
     Calculating a Timestamp
     To calculate the timestamp, the built-in millis() is used.
     The BLE standard only specifies 13 bits worth of millisecond data though,
     so it’s bitwise anded with 0x1FFF for an ever repeating cycle of 13 bits.
     This is done right after a MIDI message is detected. It’s split into a 6 upper bits, 7 lower bits,
     and the MSB of both bytes are set to indicate that this is a header byte.
     Both bytes are placed into the first two position of an array in preparation for a MIDI message.
     */
    static void getMidiTimestamp (uint8_t *header, uint8_t *timestamp)
    {
        auto currentTimeStamp = millis() & 0x01FFF;
        
        *header = ((currentTimeStamp >> 7) & 0x3F) | 0x80;        // 6 bits plus MSB
        *timestamp = (currentTimeStamp & 0x7F) | 0x80;            // 7 bits plus MSB
    }
    
    static void setMidiTimestamp (uint8_t header, uint8_t *timestamp)
    {
    }
    
public:
	// callbacks
	void(*_connectedCallback)() = nullptr;
	void(*_disconnectedCallback)() = nullptr;

public:
	void onConnected(void(*fptr)()) {
		_connectedCallback = fptr;
	}

	void onDisconnected(void(*fptr)()) {
		_disconnectedCallback = fptr;
	}

};

END_BLEMIDI_NAMESPACE