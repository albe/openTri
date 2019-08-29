# openTri
openTRI is a game engine for the Playstation Portable (PSP)

Coded by Tomaz, Raphael, InsertWittyName

See http://forums.qj.net/psp-development-forum/128874-release-triengine-full-featured-game-engine.html for original release post.


## Homebrew built with triEngine

Geometry Wars - http://www.neoflash.com/forum/index.php/topic,5322.0.html

LuaPlayer HomeMister v2 (LPHMv2) - http://forums.qj.net/showpost.php?p=2102549&postcount=100

PDF reader application - https://web.archive.org/web/20111130000918/http://forums.ps2dev.org:80/viewtopic.php?t=10845

Mudkip Adventures - https://github.com/albe/mudkip-adventures

(see also https://web.archive.org/web/20111130041704/http://www.fx-world.org:80/wordpress/?p=55)

# FEATURE LIST

## triGraphics:
- GU setup (all possible pixelformats)/Doublebuffering functionality
- Primitive drawing (lines, outlined/solid/gradient rect, rotated solid rect, solid/outlined/gradient circle, filled/outlined/gradient triangle, solid/gradient star)
- Image drawing (rotated and non-rotated)
- Color ops (and/or/not etc. see pspgu.h for a full list ;P)
- Image tinting (through the sceGuTexFunc functionality)
- Constant alpha for images
- Control blend modes
- Colorkeying
- Clipping windows
- Image animation (concated images/spritesheets) drawing (normal and rotated)
- Full render to image functionality
- Basic render to texture functionality (more complete renderbuffer functionality in tri3d)

### Images:
* load PNG (all formats), TGA (all formats), own format (supports mipmaps, animation, palettes and different compressions like RLE and GZIP)
* load and draw huge images (>512x512)
* save PNG (with/without alpha), TGA (with/without RLE compression, with/without alpha), own format
* supports paletted images as well as all other PSP formats (4444,5551,5650,DXT1/3/5)
* swizzle/unswizzle images, upload images to VRAM/download to RAM
* Image animation control (set loops, start, pause, reset, update)
* Load images animations as spritesheets from any supported format
* Capture framebuffer into image struct for saving (screenshots) or other use
* all image loads are refcounted (no double images in RAM)

### Fonts:
* based on pgeFont, loads any TTF/OTF font and creates a bitmap font, useable for printing text
very fast

## tri3d:
- Setup 3D mode (perspective/ortho) with depth buffer
- Pseudo Fullscreen Anti-Aliasing
- Smooth Dithering (for 16bit render modes)
- Renderbuffer interface - create renderbuffers, set renderbuffer as rendertarget or texture
- Set framebuffer as rendertarget or texture
- Set depthbuffer as texture (with support for a color ramp [palette] to map depth values to colors)
- A couple of high performance rendertarget convolution (blur) filters (up to 9x9)
- A quaternion based camera object with keyframing support
- An OpenGL style texture manager keeping most frequently used textures in VRAM with automatic mipmap generation and prioritizing textures to stay in VRAM
- A highly flexible particle engine supporting billboards, real 3D particles and line particles. Support for different particle interactions (bind to emitter, global force function (all particles), local force function (per particle), vortices (turbulences)) as well as different blend modes for the particles and up to 8 color/alpha fades during lifetime, giving a simple non-linear control of particle looks, also supports growing particles, selfrotation, glitter, global gravity, wind, emitter burnout and animated particle textures, plus all that can be driven by simple scripts so you can change particle systems without having to recompile your code!
- simple model management/rendering



## triAudio:
- support for AT3 playback for background music (using ME)
- support for WAV playback for sound effects

## triNet:
- support for connecting to sockets as client/server

## generic libraries:
- VRAM memory manager
- generic fast memory manager for object heaps
- stream interface with buffered streams layer and asynchronous buffered file reader for higher performance Memstick reading, as well as support for (zlib) compressed streams that are decompressed on the go
- a high performance memcpy through VFPU usage (https://github.com/albe/openTri/blob/master/src/streams/streams.c#L22)
- VMath, VFPU Vector/Matrix math library
- pmplib for decoding videos (AVC+MP3/AAC/AT3) with support for fullscreen playback (intro videos) or giving control over how the video gets displayed (ingame videos that can be used as additional briefing incoming or be displayed on 3D objects as texture)

