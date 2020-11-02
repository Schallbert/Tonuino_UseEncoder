#ifndef PROMPTPLAYER_H
#define PROMPTPLAYER_H

#include "Mp3PlayerControl_interface/Mp3PlayerControl_interface.h"

class PromptPlayer_interface
{
public:
    virtual void checkPlayPrompt(VoicePrompt prompt) = 0;
    virtual void checkPlayFolderPreview(Folder previewFolder) = 0;
};

#endif // PROMPTPLAYER_H