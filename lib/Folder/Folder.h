#ifndef FOLDER_H
#define FOLDER_H
 
// TODO #include "GetAnalogValFromArduino.h"
#include <EEPROM_interface.h>
//#include "EEPROM_implementation.h"

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
    Folder(){};
    Folder(uint8_t m_ui8FolderId, PlayMode m_ePlayMode, uint8_t m_ui8TrackCount 
           , EEPROM_interface* m_pEeprom, uint32_t m_ui32RndmSeed); // External dependency: EEPROM
    Folder(const Folder &cpySrcFolder);
    Folder& operator=(const Folder &cpySrcFolder); // = operator
    ~Folder();

public:
    uint8_t get_current_track();
    uint8_t get_next_track(); // External dependency: EEPROM
    uint8_t get_prev_track(); // External dependency: EEPROM
    PlayMode get_play_mode();
    uint8_t get_folder_id();
    uint8_t get_track_count();
    bool is_valid();

private:
    void init_playmode_related_settings();
    void init_sorted_queue();
    void shuffle_queue();

private:
    uint8_t* m_pTrackQueue {nullptr};
    uint8_t m_ui8TrackCount {0};
    uint8_t m_ui8CurrentQueueEntry {0};
    uint8_t m_ui8FolderId {0};
    PlayMode m_ePlayMode {Folder::UNDEFINED};
    EEPROM_interface* m_pEeprom {nullptr};
    uint32_t m_ui32RndmSeed {0};
};

#endif // FOLDER_H