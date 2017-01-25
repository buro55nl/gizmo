////// Copyright 2016 by Sean Luke
////// Licensed under the Apache 2.0 License

#include "All.h"

#if defined(__MEGA__)

void resetDistributionNotes() 
    { 
    memset(local.thru.distributionNotes, NO_NOTE, NUM_MIDI_CHANNELS);
    local.thru.currentDistributionChannelIndex = 0; 
    }


void stateThruPlay()
    {
    if (entry)
        {
        sendAllNotesOff();
        resetDistributionNotes();
        entry = false;
        }
                        
    if (updateDisplay)
        {
        clearScreen();
        write3x5Glyphs(GLYPH_PLAY);
        }

    if (isUpdated(BACK_BUTTON, RELEASED))
        {
        goUpState(STATE_THRU);
        sendAllNotesOff();
        }

    else if (!bypass && newItem && (itemChannel == options.channelIn || options.channelIn == CHANNEL_OMNI) && options.channelOut != 0)
        {
        uint8_t channel = options.channelOut;
                
        if (itemType == MIDI_NOTE_ON)
            {
            // NOTE DISTRIBUTION OVER MULTIPLE CHANNELS
            if (options.thruNumDistributionChannels > 0)
                {
                // revise the channel
                channel = (options.channelOut + local.thru.currentDistributionChannelIndex - 1) % NUM_MIDI_CHANNELS + 1;
                                        
                // do I need to turn off a note?
                if (local.thru.distributionNotes[local.thru.currentDistributionChannelIndex] != NO_NOTE)
                    {
                    for(uint8_t i = 0; i <= options.thruExtraNotes; i++)            // do at least once
                        {
                        sendNoteOff(local.thru.distributionNotes[local.thru.currentDistributionChannelIndex], 127, channel);
                        }
                    }
                
                // store the note and update
                local.thru.distributionNotes[local.thru.currentDistributionChannelIndex] = itemNumber;
                local.thru.currentDistributionChannelIndex++;
                if (local.thru.currentDistributionChannelIndex > options.thruNumDistributionChannels )  // yes, it's > not >= because options.thruNumDistributionChannels starts at *1*
                    local.thru.currentDistributionChannelIndex = 0;
                }
    
            // NOTE REPLICATION
            for(uint8_t i = 0; i <= options.thruExtraNotes; i++)            // do at least once
                {
                sendNoteOn(itemNumber, itemValue, channel);
                }
                
            // CHORD MEMORY
            for(uint8_t i = 1; i < options.thruChordMemorySize; i++)  // Yes, I see the *1*.  We aren't playing the bottom note a second time
            	{
            	uint8_t note = options.thruChordMemory[i] - options.thruChordMemory[0] + itemNumber;  // can't overflow, it'll only go to 254 (127 + 127).
            	if (note <= 127)
	            	sendNoteOn(note, itemValue, channel);
            	}
            }
        else if (itemType == MIDI_NOTE_OFF)
            {
            // NOTE DISTRIBUTION OVER MULTIPLE CHANNELS
            if (options.thruNumDistributionChannels > 0)
                {
                // turn off ALL instances of this note on ALL channels
                for(uint8_t i = 0; i <= options.thruNumDistributionChannels; i++)
                    {
                    if (local.thru.distributionNotes[i] == itemNumber)
                        {
                        channel = (options.channelOut + i - 1) % NUM_MIDI_CHANNELS + 1;
                        for(uint8_t i = 0; i <= options.thruExtraNotes; i++)            // do at least once
                            {
                            sendNoteOff(itemNumber, itemValue, channel);
                            }
                        local.thru.distributionNotes[i] = NO_NOTE;
                        }
                    }
                }
            else
                {
                // NOTE REPLICATION
                for(uint8_t i = 0; i <= options.thruExtraNotes; i++)            // do at least once
                    {
                    sendNoteOff(itemNumber, itemValue, channel);
                    }
                }
                
             // CHORD MEMORY
            for(uint8_t i = 1; i < options.thruChordMemorySize; i++)  // Yes, I see the *1*.  We aren't playing the bottom note a second time
            	{
            	uint8_t note = options.thruChordMemory[i] - options.thruChordMemory[0] + itemNumber;  // can't overflow, it'll only go to 254 (127 + 127).
            	if (note <= 127)
	            	sendNoteOff(note, itemValue, channel);
            	}
           }
        else if (itemType == MIDI_AFTERTOUCH_POLY)
            {
            // NOTE DISTRIBUTION OVER MULTIPLE CHANNELS
            if (options.thruNumDistributionChannels > 0)
                {
                // change ALL instances of this note on ALL channels
                for(uint8_t i = 0; i <= options.thruNumDistributionChannels; i++)
                    {
                    if (local.thru.distributionNotes[i] == itemNumber)
                        {
                        channel = (options.channelOut + i - 1) % NUM_MIDI_CHANNELS + 1;
                        for(uint8_t i = 0; i <= options.thruExtraNotes; i++)            // do at least once
                            {
                            sendPolyPressure(itemNumber, itemValue, channel);
                            }
                        }
                    }
                }
            else
                {
                // NOTE REPLICATION
                for(uint8_t i = 0; i <= options.thruExtraNotes; i++)            // do at least once
                    {
                    sendPolyPressure(itemNumber, itemValue, channel);
                    }
                }

            // CHORD MEMORY
            for(uint8_t i = 1; i < options.thruChordMemorySize; i++)  // Yes, I see the *1*.  We aren't playing the bottom note a second time
            	{
            	uint8_t note = options.thruChordMemory[i] - options.thruChordMemory[0] + itemNumber;  // can't overflow, it'll only go to 254 (127 + 127).
            	if (note <= 127)
	            	sendPolyPressure(note, itemValue, channel);
            	}
            }
        }
    }
        
#endif // defined(__MEGA__)

