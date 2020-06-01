#ifndef FOLDER_H
#define FOLDER_H

#ifndef ARDUINO_H
#include "Arduino.h"
#endif
#include <Defines.h>
#include <EEPROM.h>

class Folder
{
public:
    /*
        Implementation details:
        LULLABYE: Mp3PlayerControl's autoplay method
        ALBUM:  folder's constructor
        RANDOM: folder's shuffle method
        SAVEPROGRESS: folder's next/prev methods
        ONELARGETRACK: Mp3PlayerControl's autoplay method
    */
    enum PlayMode
    {
        UNDEFINED = 0,      // Not implemented
        LULLABYE = 4,       // like ALBUM but stops playback after timeout TODO: RECORD VOICE OUTPUT.
        ALBUM = 2,          // Play tracks in order on SD card (1,2,3...), endless, rollover.
        RANDOM = 3,         // Like ALBUM but with shuffled non-repeating queue. TODO: RECORD VOICE OUTPUT
        SAVEPROGRESS = 5,   // like ALBUM but saves track that is currently active. TODO: RE-RECORD VOICE OUTPUT
        ONELARGETRACK = 1,  // So-called Hörspielmodus. Queue like ALBUM but stops playback after finishing track. TODO: RE-RECORD VOICE OUTPUT.
        ENUM_COUNT = 5,     // Last entry of enum to allow iteration (no value for content)
    };

public:
    Folder(uint8_t folderId, PlayMode playMode, uint8_t trackCount);
    Folder();
    ~Folder();

public:
    uint8_t get_current_track();
    uint8_t get_next_track();
    uint8_t get_prev_track();
    PlayMode get_play_mode();
    uint8_t get_folder_id();
    uint8_t get_track_count();
    bool is_valid();

private:
    void init_random_generator();
    void init_sorted_queue();
    void shuffle_queue();

private:
    uint8_t* trackQueue;
    uint8_t trackCount;
    uint8_t currentQueueEntry;
    uint8_t folderId;
    PlayMode playMode;
};

#endif // FOLDER_H