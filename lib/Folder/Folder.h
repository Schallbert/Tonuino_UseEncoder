#ifndef FOLDER_H
#define FOLDER_H

#include "Tonuino_config.h"

#include "../Arduino_HardwareAbstraction/Arduino_DIcontainer_interface.h"
#include "../MessageHandler/MessageHandler_interface.h"

class Folder
{
public:
    enum ePlayMode
    {
        UNDEFINED = 0,     // Not implemented
        ALBUM = 1,         // Play tracks in order on SD card (1,2,3...), endless, rollover.
        RANDOM = 2,        // Like ALBUM but with shuffled non-repeating queue.
        SAVEPROGRESS = 3,  // like ALBUM but saves track that is currently active
        ONELARGETRACK = 4, // So-called Hörspielmodus. Queue like ALBUM but stops playback after finishing track.
        ENUM_COUNT = 4     // Last entry of enum to allow iteration (no value for content)
    };

public:
    Folder() = default;
    Folder(uint8_t ui8FolderId, ePlayMode ePlayMode);
    Folder(const Folder &cpySrcFolder);
    Folder &operator=(const Folder &cpySrcFolder); // = operator

public:
    // Returns current track in queue
    uint8_t getCurrentTrack();
    // Increments queue pointer (rollover) and returns next track [number]
    uint8_t getNextTrack(); // External dependency: EEPROM
    // Decrements queue pointer (rollover) and returns previous track [number]
    uint8_t getPrevTrack(); // External dependency: EEPROM
    // Returns folder's play mode [enum]
    ePlayMode getPlayMode() const;
    // Returns folder's id [number]
    uint8_t getFolderId() const;
    // Returns track count of folder [number], yielded from MP3 player request
    uint8_t getTrackCount() const;
    // Tries to initiate the track queue by using injected dependencies depending on play mode
    void setupDependencies(Arduino_DIcontainer_interface *pArduinoHal,
                           MessageHander_interface *pMessageHandler); // Dependency injection: Random seed & eeprom
    // returns false if trackCount cannot be set
    bool setTrackCount(uint8_t trackCount);

    // Returns true if the folder can be fully setup and is ready to be used in other modules
    bool isValid();
    // Returns true if folder is setup with relevant data (id, track count, playmode)
    bool isInitiated() const;

private:
    // Sets the queue pointer to first track and initializes the queue based on playMode
    void setup_track_queue();
    // Creates a sorted play queue (1= first track, 2= second track etc.)
    void init_sorted_queue();
    // Creates a 1:1 copy of the input track queue and saves in member variable
    void deep_copy_queue(const uint8_t *pTrackQueue);
    // Creates a Pseudo random queue, each track only once in queue
    void shuffle_queue();
    // Returns true if folder is bound to necessary external dependencies (eeprom, random seed)
    bool isDependencySet();
    // Returns true if folder's track queue has been initialized
    bool isTrackQueueSet();
    //
    void saveProgressIfRequired();

private:
    uint8_t m_ui8FolderId{0};
    ePlayMode m_ePlayMode{Folder::UNDEFINED};
    uint8_t m_ui8TrackCount{0};

    Arduino_DIcontainer_interface *m_pArduinoHal{nullptr};
    MessageHander_interface *m_pMessageHandler{nullptr};

    uint8_t m_TrackQueue[MAXTRACKSPERFOLDER + 1]{0};
    uint8_t m_ui8CurrentQueueEntry{0};
};
#endif // FOLDER_H