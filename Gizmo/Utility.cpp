////// Copyright 2016 by Sean Luke
////// Licensed under the Apache 2.0 License
//////
////// UTILITY
//////
////// Utility.h/.cpp define various utility functions used by a variety of applications
////// 



#include "All.h"





//// COMPUTING MEDIANS

// presently unused
uint16_t medianOfFive(uint16_t array[])
    {
    uint16_t a[5];
    uint16_t c;
    memcpy(a, array, 5 * sizeof(uint16_t));

// From http://stackoverflow.com/questions/11350471/finding-median-of-5-elements        
// 1) use 3 comparisons to arrange elements in array such that a[1]<a[2] , a[4]<a[5] and a[1]<a[4]
// a) compare a[1] and a[2] and swap if necessary
        
    if (a[0] > a[1]) { c = a[0]; a[0] = a[1]; a[1] = c; }
        
// b) compare a[4] and a[5] and swap if necessary 

    if (a[3] > a[4]) { c = a[3]; a[3] = a[4]; a[4] = c; }
        
// c) compare a[1] and a[4].if a[4] is smaller than a[1] , then swap a[1] wid a[4] and a[2] wid a[5]

    if (a[0] > a[3]) { c = a[0]; a[0] = a[3]; a[3] = c; 
        c = a[1]; a[1] = a[4]; a[4] = c; }

// 2)if a[3]>a[2].if a[2]<a[4] median value = min(a[3],a[4]) else median value=min(a[2],a[5]) 

    if (a[2] > a[1])
        {
        if (a[1] < a[3])
            {
            return (a[2] < a[3] ? a[2] : a[3]);
            }
        else
            {
            return (a[1] < a[4] ? a[1] : a[4]);
            }
        }

// 3)if a[3]<a[2].if a[3]>a[4] median value = min(a[3],a[5]) else median value=min(a[2],a[4])
        
    else
        {
        if (a[3] < a[2])
            {
            return (a[2] < a[4] ? a[2] : a[4]);
            }
        else
            {
            return (a[1] < a[3] ? a[1] : a[3]);
            }
        }
    }






////// DEBUGGING CODE


uint8_t debug(int16_t val)
    {
    clearMatrix(led);
    clearMatrix(led2);
    writeNumber(led, led2, val);
    sendMatrix(led, led2);
    delay(500);
    return 1;
    }

uint8_t debug(const char* str, int8_t val)
    {
    char s[3];
    strncpy_P(s, str, 2);
    s[2] = 0;
    clearMatrix(led);
    clearMatrix(led2);
    memcpy_P(led2, font_3x5[GLYPH_3x5_A + s[0] - 'A'], 3);
    memcpy_P(led2 + 5, font_3x5[GLYPH_3x5_A + s[1] - 'A'], 3);
    writeShortNumber(led, val, false);
    sendMatrix(led, led2);
    delay(500);
    return 1;
    }
        
uint8_t debug(int8_t val1, int8_t val2)
    {
    clearMatrix(led);
    clearMatrix(led2);
    writeShortNumber(led2, val1, true);
    writeShortNumber(led, val2, false);
    sendMatrix(led, led2);
    delay(500);
    return 1;
    }










void goDownState(uint8_t nextState)
    {
//    defaultState = STATE_NONE;
//    state = nextState;
//    entry = true;
//    clearReleased();
    goUpState(nextState);
    defaultState = STATE_NONE;
    }

void goUpState(uint8_t nextState)
    {
    defaultState = state; 
    state = nextState;
    entry = true;
    clearReleased();
    }


void goUpStateWithBackup(uint8_t _nextState)
    {
    goUpState(_nextState);
    options = backupOptions;
    }

void goDownStateWithBackup(uint8_t _nextState)
    {
    goDownState(_nextState);
    options = backupOptions;
    }





#ifdef INCLUDE_STEP_SEQUENCER
        
// Starting at position pos, distributes the bits of the given byte among the high bytes >= pos
void distributeByte(uint16_t pos, uint8_t byte)
    {
    for(uint8_t i = 0; i < 8; i++)
        {
        data.slot.data.stepSequencer.buffer[pos + i] = 
            ((data.slot.data.stepSequencer.buffer[pos + i] & 127) | (byte & 128));
        byte = (byte << 1);
        }
    }

// Gathers high bits starting at position pos to form a complete byte.
uint8_t gatherByte(uint16_t pos)
    {
    uint8_t byte = 0;
    for(uint8_t i = 0; i < 8; i++)
        {
        byte = ((byte << 1) | (data.slot.data.stepSequencer.buffer[pos + i] >> 7));
        }
    return byte;
    }
        
void stripHighBits()
    {
    // strip the high bits
    for(uint16_t i = 0; i < STEP_SEQUENCER_BUFFER_SIZE; i++)
        data.slot.data.stepSequencer.buffer[i] = (127 & data.slot.data.stepSequencer.buffer[i]);
    }

#endif









////// GENERIC STATE FUNCTIONS


// Private function, used by stateSave and stateLoad
// Displays all the slots, and their slot type, and lets the user choose
uint8_t stateSaveLoad(uint8_t includeOff)
    {
    // Maybe search for first empty slot first?  Not doing it now though.
    if (entry)
        {
        for(uint8_t i = 0; i < NUM_SLOTS; i++)
            {
            glyphs[i] = getSlotType(i);
            }
        }
    return doNumericalDisplay(includeOff ? -1 : 0, (NUM_SLOTS - 1), 0, includeOff, OTHER_GLYPH);
    }




// Saves a slot, either for the recorder or the sequencer. 
void stateSave(uint8_t backState)
    {
    secondGlyph = GLYPH_3x5_S;
    uint8_t result = stateSaveLoad(false);
    entry = false;
    switch (result)
        {
        case NO_MENU_SELECTED:
            break;
        case MENU_SELECTED:
            data.slot.type = slotTypeForApplication(application);
            switch(application)
                {
#ifdef INCLUDE_STEP_SEQUENCER
                case STATE_STEP_SEQUENCER:
                    {
                    uint8_t len = GET_TRACK_LENGTH();
                    uint8_t num = GET_NUM_TRACKS();
                                        
                    // pack the high-bit parts
                    for(uint8_t i = 0; i < num; i++)
                        {
                        uint16_t pos = i * len * 2;
#ifdef INCLUDE_EXTENDED_STEP_SEQUENCER

                        //// 1 bit type of data
                        if (local.stepSequencer.data[i] == STEP_SEQUENCER_DATA_NOTE)
                            {
                            distributeByte(pos, 0 << 7);  // Note data is a 0
                                
                            //// 1 bit mute
                            //// 5 bits MIDI out channel (including "use default")
                            //// 7 bits length
                            //// 8 bits velocity (including "use per-note velocity")
                            //// 7 bits fader
        
                            distributeByte(pos + 1, local.stepSequencer.muted[i] << 7);
                            distributeByte(pos + 2, local.stepSequencer.outMIDI[i] << 3);
                            distributeByte(pos + 7, local.stepSequencer.noteLength[i] << 1);
                            distributeByte(pos + 14, local.stepSequencer.velocity[i]);      
                            distributeByte(pos + 22, local.stepSequencer.fader[i] << 1);
                            }
                        else
                            {
                            distributeByte(pos, 1 << 7);  // Control data is a 1
                                
                            ////     3 bits: CC, NRPN, RPN, PC, BEND, AFTERTOUCH, INTERNAL
                            ////     14 bits Parameter
                            ////     5 bits MIDI out channel

                            uint8_t controlDataType = local.stepSequencer.data[i] - 1;
                            distributeByte(pos + 1, controlDataType << 4);
                            distributeByte(pos + 4, local.stepSequencer.noteLength[i] << 1);                // MSB of Control Parameter
                            distributeByte(pos + 11, local.stepSequencer.velocity[i] << 1);                 // LSB of Control Parameter
                            distributeByte(pos + 18, local.stepSequencer.outMIDI[i] << 3);
                            }
#else
                        //// 1 bit mute
                        //// 5 bits MIDI out channel (including "use default")
                        //// 7 bits length
                        //// 8 bits velocity (including "use per-note velocity")
                        //// 7 bits fader
        
                        distributeByte(pos, local.stepSequencer.muted[i] << 7);
                        distributeByte(pos + 1, local.stepSequencer.outMIDI[i] << 3);
                        distributeByte(pos + 6, local.stepSequencer.noteLength[i] << 1);
                        distributeByte(pos + 13, local.stepSequencer.velocity[i]);      
                        distributeByte(pos + 21, local.stepSequencer.fader[i] << 1);
#endif


                        }
                    saveSlot(currentDisplay);
                    stripHighBits();                        
                    }
                break;
#endif
#ifdef INCLUDE_RECORDER
                case STATE_RECORDER:
                    {
                    saveSlot(currentDisplay);
                    }
                break;
#endif
                }
                                        
            goUpState(backState);
            break;
        case MENU_CANCELLED:
            goUpState(backState);
            break;
        }
    }




void stateLoad(uint8_t selectedState, uint8_t initState, uint8_t backState, uint8_t _defaultState)
    {
    secondGlyph = GLYPH_3x5_L;
    uint8_t result = stateSaveLoad(true);
    entry = false;
    switch (result)
        {
        case NO_MENU_SELECTED:
            break;
        case MENU_SELECTED:
            if (currentDisplay == -1)  // Init
                {
                state = initState;
                }
            else
                {
                loadSlot(currentDisplay);
                        
                if ((data.slot.type != slotTypeForApplication(application)))
                    {
                    state = initState;
                    }
                else
                    {
                    state = selectedState;
#ifdef INCLUDE_STEP_SEQUENCER
                    if (application == STATE_STEP_SEQUENCER)
                        {
                        // FIXME: did I fix the issue of synchronizing the beats with the sequencer notes?
                        local.stepSequencer.currentPlayPosition = 
                            div12((24 - beatCountdown) * notePulseRate) >> 1;   // get in sync with beats

                        uint8_t len = GET_TRACK_LENGTH();
                        uint8_t num = GET_NUM_TRACKS();
                                
                        // unpack the high-bit info
                        for(uint8_t i = 0; i < num; i++)
                            {
                            uint16_t pos = i * len * 2;

#ifdef INCLUDE_EXTENDED_STEP_SEQUENCER

                            //// 1 bit type of data
                            if (gatherByte(pos) >> 7 == 0)  // It's a note
                                {
                                //// 1 bit mute
                                //// 5 bits MIDI out channel (including "use default")
                                //// 7 bits length
                                //// 8 bits velocity (including "use per-note velocity")
                                //// 7 bits fader
                                local.stepSequencer.data[i] = STEP_SEQUENCER_DATA_NOTE;
                                
                                local.stepSequencer.muted[i] = (gatherByte(pos + 1) >> 7); // first bit
                                local.stepSequencer.outMIDI[i] = (gatherByte(pos + 2) >> 3);  // top 5 bits moved down 3
                                local.stepSequencer.noteLength[i] = (gatherByte(pos + 7) >> 1); // top 7 bits moved down 1
                                local.stepSequencer.velocity[i] = (gatherByte(pos + 14)); // all 8 bits
                                local.stepSequencer.fader[i] = (gatherByte(pos + 22) >> 1);  // top 7 bits moved down 1
                                }
                            else                        // It's a control sequence
                                {                               
                                ////     3 bits: CC, NRPN, RPN, PC, BEND, AFTERTOUCH, VOLTAGE A, VOLTAGE B
                                ////     14 bits Parameter
                                ////     5 bits MIDI out channel

                                uint8_t controlDataType = (gatherByte(pos + 1) >> 5);
                                local.stepSequencer.data[i] = controlDataType + 1;
                                local.stepSequencer.noteLength[i] = (gatherByte(pos + 4) >> 5);
                                local.stepSequencer.velocity[i] = (gatherByte(pos + 11) >> 5);
                                local.stepSequencer.outMIDI[i] = (gatherByte(pos + 18) >> 5);
                                }
#else
                            //// 1 bit mute
                            //// 5 bits MIDI out channel (including "use default")
                            //// 7 bits length
                            //// 8 bits velocity (including "use per-note velocity")
                            //// 7 bits fader
                            local.stepSequencer.muted[i] = (gatherByte(pos) >> 7); // first bit
                            local.stepSequencer.outMIDI[i] = (gatherByte(pos + 1) >> 3);  // top 5 bits moved down 3
                            local.stepSequencer.noteLength[i] = (gatherByte(pos + 6) >> 1); // top 7 bits moved down 1
                            local.stepSequencer.velocity[i] = (gatherByte(pos + 13)); // all 8 bits
                            local.stepSequencer.fader[i] = (gatherByte(pos + 21) >> 1);  // top 7 bits moved down 1
#endif
                            }
                            
                        stripHighBits();
                        }
#endif
                    }
                }

#ifdef INCLUDE_STEP_SEQUENCER
                                
            local.stepSequencer.solo = 0;
            local.stepSequencer.currentTrack = 0;
#ifdef INCLUDE_EXTENDED_STEP_SEQUENCER
            local.stepSequencer.transpose = 0;
#endif
#ifdef INCLUDE_PROVIDE_RAW_CC
            setParseRawCC(local.stepSequencer.data[local.stepSequencer.currentTrack] == STEP_SEQUENCER_DATA_CC);
#endif
            local.stepSequencer.currentEditPosition = 0;
            stopStepSequencer();
#endif
                
            defaultState = STATE_NONE;
            entry = true;
            
            break;
        case MENU_CANCELLED:
            goUpState(backState);
            defaultState = _defaultState;
            break;
        }
    }





void stateSure(uint8_t selectedState, uint8_t backState)
    {
    if (updateDisplay)
        {
        clearScreen();
        write8x5Glyph(led2, GLYPH_8x5_SURE_PT1);
        write8x5Glyph(led, GLYPH_8x5_SURE_PT2);
        }

    if (isUpdated(BACK_BUTTON, RELEASED))
        {
        //suggestedDefaultState = defaultState; 
        // send ALL NOTES OFF
        MIDI.sendControlChange(123, 0, options.channelOut);
        
        goUpState(backState);
        defaultState = selectedState;
        }
    else if (isUpdated(SELECT_BUTTON, PRESSED))
        {
        // just go back, don't reenter
        state = selectedState;
        }
    }



// bounds n to be between min and max 
uint8_t bound(uint8_t n, uint8_t min, uint8_t max)
    {
    if (n > max) n = max;
    if (n < min) n = min;
    return n;
    }

// increments n, then wraps it to 0 if it is >= max
uint8_t incrementAndWrap(uint8_t n, uint8_t max)
    {
    if (++n >= max)
        return 0;
    return n;
    }

        
GLOBAL uint8_t stateEnterNoteVelocity;

uint8_t stateEnterNote(uint8_t backState)
    {
    if (entry)
        {
        newItem = 0;            // clear any current note
        clearScreen();
        write3x5Glyphs(GLYPH_NOTE);
        entry = false;
        }
        
    if (isUpdated(BACK_BUTTON, RELEASED))
        {
        goUpState(backState);
        }
    
    // process these but do nothing with them so if the user
    // accidentally presses these buttons BEFORE he chooses a note,
    // the button press isn't queued for later -- I do that a lot.
    isUpdated(SELECT_BUTTON, RELEASED);
    isUpdated(MIDDLE_BUTTON, RELEASED);
        
    if (newItem == NEW_ITEM && itemType == MIDI_NOTE_ON)
        {
        // we don't send a NOTE ON here because we have no easy place to STOP playing it
        stateEnterNoteVelocity = itemValue;  // velocity
        return itemNumber;
        }
        
    return NO_NOTE;
    }


#ifdef INCLUDE_ENTER_CHORD

GLOBAL uint8_t chordCount;

uint8_t stateEnterChord(uint8_t* chord, uint8_t maxChordNotes, uint8_t backState)
    {
    if (entry)
        {
        newItem = 0;            // clear any current note
        clearScreen();
        write3x5Glyphs(GLYPH_CHORD);
        entry = false;
        chordCount = 0;
        }
        
    if (isUpdated(BACK_BUTTON, RELEASED))
        {
        goUpState(backState);
        }
    
    // process these but do nothing with them so if the user
    // accidentally presses these buttons BEFORE he chooses a note,
    // the button press isn't queued for later -- I do that a lot.
    isUpdated(SELECT_BUTTON, RELEASED);
    isUpdated(MIDDLE_BUTTON, RELEASED);
        
    if (newItem == NEW_ITEM && chordCount < maxChordNotes)
        {
        if (itemType == MIDI_NOTE_ON)
            {
            for(uint8_t i = 0; i < chordCount; i++)
                {
                if ((chord[i] & 127) == itemNumber)
                    return NO_NOTE;  // already have that one
                }
                
            // okay, so it's a new note
            chord[chordCount] = (itemNumber | 128);
            chordCount++;
            stateEnterNoteVelocity = itemValue;  // velocity
            }
        else if (itemType == MIDI_NOTE_OFF)
            {
            // first mark the note as up
                
            for(uint8_t i = 0; i < chordCount; i++)
                {
                if ((chord[i] & 127) == itemNumber)
                    {
                    chord[i] = itemNumber;  // move off of +128 if it's already there
                    break;
                    }
                }
                        
            // next check to see if any notes are still down
            // (this could easily be done with a count, but we only have a few notes, so no biggie)
            for(uint8_t i = 0; i < chordCount; i++)
                {
                if (chord[i] >= 128)  // something's being held down
                    return NO_NOTE;
                }

            // at this point nothing is held down.  Insertion Sort FTW!
            int8_t j;  // j goes negative so we have to be signed
            for(uint8_t i = 1; i < chordCount; i++)
                {
                uint8_t val = chord[i];
                j = i - 1;
                while((j >= 0) && (chord[j] > val))
                    {
                    chord[j + 1] = chord[j];
                    j = j - 1;
                    }
                chord[j + 1] = val;
                }

            // return the count
            return chordCount;
            }
        }
        
    return NO_NOTE;
    }

#endif


void playApplication()
    {
    switch(optionsReturnState)
        {
#ifdef INCLUDE_ARPEGGIATOR
        case STATE_ARPEGGIATOR_MENU:
        case STATE_ARPEGGIATOR_PLAY:
            playArpeggio();          
            break; 
#endif
#ifdef INCLUDE_STEP_SEQUENCER
        case STATE_STEP_SEQUENCER_MENU:
            playStepSequencer();
            break;
#endif
#ifdef INCLUDE_RECORDER
        case STATE_RECORDER_PLAY:  // note not MENU: we go directly to options from PLAY
            // This is a dummy function, which we include to keep the switch statement from growing by 100 bytes (!)
            // Because we do NOT want to play the recorder in the background ever.
            playRecorder();
            break; 
#endif
#ifdef INCLUDE_MEASURE
        case STATE_MEASURE:
            playMeasure();
            break;
#endif
        }
    }
        



//// CLEARSCREEN()
//
// Clears the screen buffer.  If there are two screens, clears both of them.

void clearScreen()
    {
    clearMatrix(led);
    clearMatrix(led2);
    }






GLOBAL static uint8_t glyphTable[
#ifdef INCLUDE_EXTENDED_GLYPH_TABLE
    19
#else
    15
#endif
    ][4] = 
    {
    // These first: ----, ALLC, DFLT, DECR, and INCR, must be the FIRST ones
    // because the correspond with the five glyph types in doNumericalDisplay
    {GLYPH_3x5_MINUS, GLYPH_3x5_MINUS, GLYPH_3x5_MINUS, GLYPH_3x5_MINUS},   // ----
    {GLYPH_3x5_A, GLYPH_3x5_L, GLYPH_3x5_L, GLYPH_3x5_C},   // ALLC
    {GLYPH_3x5_D, GLYPH_3x5_F, GLYPH_3x5_L, GLYPH_3x5_T},   // DFLT
    {GLYPH_3x5_D, GLYPH_3x5_E, GLYPH_3x5_C, GLYPH_3x5_R},   // DECR
    {GLYPH_3x5_I, GLYPH_3x5_N, GLYPH_3x5_C, GLYPH_3x5_R},   // INCR
    {GLYPH_3x5_F, GLYPH_3x5_R, GLYPH_3x5_E, GLYPH_3x5_E},   // FREE
    {GLYPH_3x5_N, GLYPH_3x5_O, GLYPH_3x5_T, GLYPH_3x5_E},   // NOTE
    {GLYPH_3x5_S, GLYPH_3x5_Y, GLYPH_3x5_S, GLYPH_3x5_X},   // SYSX
    {GLYPH_3x5_S, GLYPH_3x5_P, GLYPH_3x5_O, GLYPH_3x5_S},   // SPOS
    {GLYPH_3x5_S, GLYPH_3x5_S, GLYPH_3x5_E, GLYPH_3x5_L},   // SSEL
    {GLYPH_3x5_T, GLYPH_3x5_R, GLYPH_3x5_E, GLYPH_3x5_Q},   // TREQ
    {GLYPH_3x5_S, GLYPH_3x5_T, GLYPH_3x5_R, GLYPH_3x5_T},   // STRT
    {GLYPH_3x5_C, GLYPH_3x5_O, GLYPH_3x5_N, GLYPH_3x5_T},   // CONT
    {GLYPH_3x5_S, GLYPH_3x5_T, GLYPH_3x5_O, GLYPH_3x5_P},   // STOP
    {GLYPH_3x5_R, GLYPH_3x5_S, GLYPH_3x5_E, GLYPH_3x5_T},   // RSET
    
#ifdef INCLUDE_EXTENDED_GLYPH_TABLE
    {GLYPH_3x5_F, GLYPH_3x5_A, GLYPH_3x5_D, GLYPH_3x5_E},   // FADE
    {GLYPH_3x5_P, GLYPH_3x5_L, GLYPH_3x5_A, GLYPH_3x5_Y},   // PLAY
    {GLYPH_3x5_C, GLYPH_3x5_H, GLYPH_3x5_R, GLYPH_3x5_D},   // CHRD
    {GLYPH_3x5_H, GLYPH_3x5_I, GLYPH_3x5_G, GLYPH_3x5_H},       // HIGH
#endif

    };



// Writes any of the above glyph sets to the screen
void write3x5Glyphs(uint8_t index)
    {
    uint8_t *glyphs = glyphTable[index];
    write3x5Glyph(led2, glyphs[0], 0);
    write3x5Glyph(led2, glyphs[1], 4);
    write3x5Glyph(led, glyphs[2], 0);
    write3x5Glyph(led, glyphs[3], 4);
    }



void drawMIDIChannel(uint8_t channel)
    {
    // finally, draw the channel
    if (channel == CHANNEL_OMNI)
        {
        for(uint8_t i = 2; i < 6; i++)
            setPoint(led2, i, 0);
        }
    else if (channel == CHANNEL_OFF)
        {
        setPoint(led2, 0, 0);
        setPoint(led2, 2, 0);
        setPoint(led2, 5, 0);
        setPoint(led2, 7, 0);
        }
    else
        {
        drawRange(led2, 0, 0, 16, (uint8_t)(channel - 1));
        }
    }


GLOBAL static uint8_t clickNote = NO_NOTE;
        
void doClick()
    {
    // turn off previous click
    if (pulse && (clickNote != NO_NOTE))
        {
        sendNoteOff(clickNote, 127, options.channelOut);
        clickNote = NO_NOTE;
        }
                
    // turn on new click
    if (beat && (options.click != NO_NOTE))
        {
        sendNoteOn(options.click, options.clickVelocity, options.channelOut);
        clickNote = options.click;
        }
    }




#ifdef INCLUDE_OPTIONS_MENU_DELAY
///// SCROLL DELAY
///// 


////// We only redraw once every 4 ticks.
////// Thus a delay of X means 4 * X ticks
////// A tick is 1/3125 sec, so a delay of 78 means is about 1/10 sec
////// I find that faster than 1/20 sec the screen becomes hard to watch.
////// So I'm going with 40

GLOBAL static uint8_t menuDelays[11] = { NO_MENU_DELAY, EIGHTH_MENU_DELAY, QUARTER_MENU_DELAY, THIRD_MENU_DELAY, HALF_MENU_DELAY, DEFAULT_MENU_DELAY, DOUBLE_MENU_DELAY, TREBLE_MENU_DELAY, QUADRUPLE_MENU_DELAY, EIGHT_TIMES_MENU_DELAY, HIGH_MENU_DELAY };

// SET MENU DELAY
// Changes the menu delay to a desired value (between 0: no menu delay, and 11: infinite menu delay).  The default is 5
void setMenuDelay(uint8_t index)
    {
    if (index > 10) index = 5;
    setScrollDelays(menuDelays[index], DEFAULT_SHORT_DELAY);
    }
#endif
