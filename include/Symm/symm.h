#ifndef SYMM_H
#define SYMM_H

#ifdef SYMM_IMPLEMENTATION
#define SYMM_API extern "C"
#else
#define SYMM_API
#endif

// Define the graphics module
#ifdef SYMM_GRAPHICS_IMPLEMENTATION
namespace symm_graphics {
    SYMM_API void InitGraphics(); // Init Graphics
    SYMM_API void RenderFrame(); // Render a frame
    SYMM_API void createObject();  // Create a object
    SYMM_API void renderShaders(); // Render shaders
    SYMM_API void SymmCreateShaders(); // Create shaders
    SYMM_API void renderFont(); //Render font
}
#endif

// Define the audio module
#ifdef SYMM_AUDIO_IMPLEMENTATION
namespace symm_audio {
    SYMM_API void InitAudio(); // Init Audio
    SYMM_API void PlaySound(); // Play sound/s
    SYMM_API void replayAudio(); // Replay audio
    SYMM_API void AudioTimestamp(); // The audio timestamp
}
#endif

#endif // SYMM_H
