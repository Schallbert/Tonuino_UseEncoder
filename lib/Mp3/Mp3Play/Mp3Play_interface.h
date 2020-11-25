#ifndef MP3PLAY_INTERFACE_H
#define MP3PLAY_INTERFACE_H

#include "Mp3Control/Mp3Control_interface.h"

class Mp3Play_interface
{
public:
    virtual void playPrompt(const VoicePrompt &prompt) const = 0;
    virtual void playFolder(Folder &folder) = 0;
    virtual void playPrev() = 0;
    virtual void playNext() = 0;
    virtual void autoplay() = 0;
    virtual bool isPlaying() const = 0;
};

#endif // MP3PLAY_INTERFACE_H