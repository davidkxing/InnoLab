//implement all necessary functions from libretro.h in here
#include "core.h"
#pragma comment(lib, "Winmm.lib")
#include <mmsystem.h>

static uint8_t* frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static bool use_audio_cb;
static float last_aspect;
static float last_sample_rate;
char retro_base_directory[4096];          //directory path
char retro_game_path[4096];
float timeVal;
int tempValue = 1;

std::string teststring; //todo: remove later with implementing better logging functionality

static void fallback_log(enum retro_log_level level, const char* fmt, ...)
{
    (void)level;

    va_list va;
    va_start(va, fmt);

    std::string teststring = "[IC]\t";
    teststring = teststring.append(fmt);

    vfprintf(stderr, teststring.data(), va);
    va_end(va);
}

static retro_environment_t environ_cb;

//This function is called once, and gives the implementation a chance to initialize data structures.
void retro_init(void)
{
    frame_buf = (uint8_t*)malloc(VIDEO_PIXELS * sizeof(uint32_t));             //allocates pixels for screen
    if(frame_buf == NULL){
        printf("theres been a fatal error\n");
    }
    const char* dir = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
        snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);   //saves directory path
    }
    
    
}

//This function should free all state that was initialized during retro_init(). 
//After calling this function, the frontend can again call retro_init().
void retro_deinit(void)
{
    free(frame_buf);
    frame_buf = NULL;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;  //defined in libretro.h
}

//By default, gamepads will be assumed to be inserted into the implementation. 
//If the engine is sensitive to which type of input device is plugged in, 
//the frontend may call this function to set the device to be used for a certain player. 
//The implementation should try to auto-detect this if possible.
void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);   //edit?
}

//The frontend will request information about the core. The information returned must be allocated statically.
void retro_get_system_info(struct retro_system_info* info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "InnoLab_Core"; //YC
    info->library_version = "Pong 2.0";  //YC
    info->need_fullpath = true;   //IDK, edit?
    info->valid_extensions = "";     //if you were to load any other files in your core, you should specify the allowed types
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

//This function lets the frontend know essential audio/video properties of the game. 
//As this information can depend on the game being loaded, this info will only be queried after a valid ROM image has been loaded. 
//It is important to accurately report FPS and audio sampling rates, as FFmpeg recording relies on 
//exact information to be able to run in sync for several hours.
void retro_get_system_av_info(struct retro_system_av_info* info)  //TODO: edit, change later when you know what you need there
{
    float aspect = 0.0f;
    float sampling_rate = 30000.0f;


    info->geometry.base_width = VIDEO_WIDTH;
    info->geometry.base_height = VIDEO_HEIGHT;
    info->geometry.max_width = VIDEO_WIDTH;
    info->geometry.max_height = VIDEO_HEIGHT;
    info->geometry.aspect_ratio = aspect;

    last_aspect = aspect;
    last_sample_rate = sampling_rate;
}

static struct retro_rumble_interface rumble;

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging)) {
        log_cb = logging.log;
        log_cb(RETRO_LOG_INFO, "Horray, using desired logging functionality, lets celebrate! :D\n");
    }  
    else
    {
        log_cb = fallback_log;
        teststring = __func__;
        teststring += "(): Using fallback logging functionality.\n";
        log_cb(RETRO_LOG_WARN, teststring.data());
    }

    if(frame_buf == NULL){
        log_cb(RETRO_LOG_WARN, "oh no, frame_buf not happy'\n");
    }else{
        log_cb(RETRO_LOG_INFO, "all good on frame_buf front'\n");
    }


    static const struct retro_controller_description controllers[] = {
       { "Nintendo DS", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0) },    //edit if you need keyboard or mouse
    };

    static const struct retro_controller_info ports[] = {             //edit maybe, idk what this does
       { controllers, 1 },
       { NULL, 0 },
    };

    if (cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports)) {
        teststring = __func__;
        teststring += "(): Setting controller info did probably work.\n";
        log_cb(RETRO_LOG_INFO, teststring.data());
    }
    else
    {
        teststring = __func__;
        teststring += "(): Setting controller info did probably not work.\n";
        log_cb(RETRO_LOG_WARN, teststring.data());
    }

    const bool no_game = true;
    
    if (cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, (void*)&no_game)) {
        teststring = __func__;
        teststring += "(): NO_Game_Mode works :)\n";
        log_cb(RETRO_LOG_INFO, teststring.data());
    }
    else
    {
        teststring = __func__;
        teststring += "(): NO_Game_Mode does not work :(\n";
        log_cb(RETRO_LOG_WARN, teststring.data());
    }
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    log_cb(RETRO_LOG_INFO, "retro_set_video_refresh was called\n");
    video_cb = cb;
}

static unsigned x_coord;
static unsigned y_coord;
static unsigned phase;
static int mouse_rel_x;
static int mouse_rel_y;

void retro_reset(void)
{//todo: change vals to 0
    x_coord = VIDEO_WIDTH / 2;
    y_coord = VIDEO_HEIGHT / 2;
    game_reset();
}

static void update_input(void)
{
    //input_poll_cb();
    //int16_t <- port, device, index, id
    //int16_t result = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
    //log_cb(RETRO_LOG_INFO, "input result: %d\n", result);
    //teststring = __func__;
    //teststring += "(): test\n";
    //log_cb(RETRO_LOG_INFO, teststring.data());
}

static void check_variables(void)
{
    //dunno what this is supposed to do, but it is from skeleton, not libretro.h, so not that important
   
    teststring = __func__;
    teststring += "(): checking variables\n";
    log_cb(RETRO_LOG_INFO, teststring.data());
}

static void audio_callback(void)
{

    //teststring = __func__;
    //teststring += "(): test\n";
    //log_cb(RETRO_LOG_INFO, teststring.data());

    /*for (unsigned i = 0; i < 30000 / 60; i++, phase++) //this is some noise and stuff
    {
        int16_t val = 0x800 * sinf(2.0f * M_PI * phase * 300.0f / 30000.0f);
        audio_cb(val, val);
    }*/

    phase %= 100;
}

static void audio_set_state(bool enable)
{
    (void)enable;
}

//This function will load content. If the implementation is an emulator, 
//this would be a game ROM image, if it is a game engine, this could be packaged upassets for the game, etc. 
//The function takes a structure that points to the path where the ROM was loaded from, 
//as well as a memory chunk of the already loaded file.
bool retro_load_game(const struct retro_game_info* info)
{
    
    //due to no-game-mode info == NULL

    struct retro_input_descriptor desc[] = {
       { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
       { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
       { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
       { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
       { 0 },
    };

    if (environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc)) {

    }
    else
    {
        teststring = __func__;
        teststring += "(): test: setting input descriptors did not work\n";
        log_cb(RETRO_LOG_WARN, teststring.data());
    }

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_WARN, "XRGB8888 is not supported.\n");
        return false;
    }    

    //snprintf(retro_game_path, sizeof(retro_game_path), "%s", info->path);
    //only needed if you want to load a game

    struct retro_audio_callback audio_cb = { audio_callback, audio_set_state };

    use_audio_cb = environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

    check_variables();   //does nothing so far

    (void)info;

    log_cb(RETRO_LOG_INFO, "game_init()\n");

    game_init();

    if(frame_buf == NULL){
        log_cb(RETRO_LOG_WARN, "ah fuck\n");
    }
    log_cb(RETRO_LOG_INFO, "all good on frame_buf front'\n");
    log_cb(RETRO_LOG_INFO, "SETUP COMPLETE\n");
    return true;
}

/*
There are two modes of loading files with libretro.
If the game engine requires to know the path of where the ROM image was loaded from,
the need_fullpath field in retro_system_info must be set to true.
If the path is required, the frontend will not load the file into the data/size fields,
and it is up to the implementation to load the file from disk.
The path might be both relative and absolute, and the implementation must check for both cases.
This is useful if the ROM image is too large to load into memory at once.
It is also useful if the assests consist of many smaller files, where it is necessary to
know the path of a master file to infer the paths of the others.

If need_fullpath is set to false, the frontend will load the ROM image into memory beforehand.
In this mode, the path field is not guaranteed to be non-NULL.
It should point to a valid path if the file was indeed, loaded from disk, however, it is possible that
the file was loaded from stdin, or similar, which has no well-defined path.
It is recommended that need_fullpath is set to false if possible, as it allows more features,
such as soft-patching to work correctly.
*/


//After a game has been loaded successfully, retro_run() will be called repeatedly as long as the user desires. 
//When called, the implementation will perform its inner functionality for one video frame. 
//During this time, the implementation is free to call callbacks for video frames, audio samples, 
//as well as polling input, and querying current input state. 
//The requirements for the callbacks are that video callback is called exactly once, i.e. it does not have to come last. 
//Also, input polling must be called at least once.

void playAudio() {
    PlaySound(TEXT("D:/bg_music.wav"), NULL, SND_ASYNC);    //change absolute path from bg_music.wav
    Sleep(10);
}


void retro_run(void)
{
    Timer timer;
    printf("%f\n", timeVal);
    if (timeVal >= 14.0f) {
        timeVal = 0;
        //print frame
        if (inGameCheck() == 1) {
            if (tempValue) {
                playAudio();
                tempValue = 0;
            }
            game_print(frame_buf);
        }
        else {
            menu_print(frame_buf);
        }
        video_cb(frame_buf, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * 4);  //todo: 4=bytes_per_pixel -> const

        //get user input
        input_poll_cb(); //port, device, index, id -> int16_t
        key_up = (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) ? true : false;
        key_down = (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)) ? true : false;
        key_left = (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)) ? true : false;
        key_right = (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) ? true : false;
        key_start = (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) ? true : false;

        //progess game
        game_run();

        /*//don't know what this does or what check_variables() is supposed to do
        bool updated = false;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
            check_variables();
        */

        //16.6ms pro Frame == 60fps
        //tempMsChecker = 0; da drinnen wird ms
        //tempMsChecker < 16.6ms
    }
}

/*
# Input
Abstracting gamepad and other input devices is the hardest part of defining a multi-system API as it differs across every system.
The libretro API therefore provides the RetroPad -- a gamepad/joystick abstraction available with digital and analog controls --
to allow cores to be written without platform-specific input code.
Input device abstractions are also available for keyboards, mice, pointers, and lightguns.

# Video/Audio synchronization considerations
Libretro is based on fixed rates; video FPS and audio sampling rates are always assumed to be constant.
Frontends will have control of the speed of playing, typically using VSync to obtain correctspeed.
The frontend is free to "fast-forward", i.e. play as fast as possible without waiting, or slow-motion.
For this reason, the engine should not rely on system timers to perform arbitrary synchronization.
This is common and often needed in 3D games to account for varying frame rates while still maintaining a playable game.

However, libretro targets classic systems where one can assume that 100% real-time performance will always be met,
thus avoiding the need for careful timing code.
By default, the libretro implementation should replace any arbitrary sleep() and time() patterns with
simply calling video/audio callbacks. The frontend will make sure to apply the proper synchronization.
This is mostly a problem with game ports, such as PrBoom.
For the libretro port of PrBoom, which heavily relied on timers and sleeping patterns,
sleeping was replaced with simply running for one frame, and calling the video callback.
After that, enough audio was rendered to correspond to one frames worth of time, 1/fps seconds.
All sleeping and timing patterns could be removed, and synchronization was correct.

# Audio callback options
The libretro API has two different audio callbacks. Only one of these should be used;
the implementation must choose which callback is best suited.
One audio frame is always made up of 2 int16_t samples, corresponding to the left and right channels, respectively.

PER-SAMPLE AUDIO
The first audio callback is called per-sample, but actually deals with a single audio frame.
It has the prototype void (*retro_audio_sample_t)(int16_t left, int16_t right).
This should be used if the implementation generates a single audio frame at a time.
The frontend will make sure to partition the audio data into suitable chunks to avoid incurring too much syscall overhead.

BATCH AUDIO
If audio is output in a "batch" fashion, i.e. 1 / fps seconds worth of audio data at a time,
the batch approach should be considered.
Rather than looping over all audio frames and calling the per-sample callback every time,
the batch callback should be used instead.
Its prototype is size_t (*retro_audio_sample_batch_t)(const int16_t * samples, size_t num_frames).
The number of samples should be 2 * num_frames, with left and right channels interleaved every frame.
Using the batch callback, audio will not be copied in a temporary buffer, which can buy a slight performance gain.
Also, all data will be pushed to audio driver in one go, saving some slight overhead.

It is not recommended to use the batch callback for very small (< 32 frames) amounts of data.
The data passed to the batch callback should, if possible, be aligned to 16 bytes (depends on platform),
to allow accelerated SIMD operations on audio.

retro_serialize_size(), retro_serialize(), and retro_unserialize()¶
Serialization is optional to implement.
Serialization is better known as "save states" in emulators, and these functions are certainly
more useful in emulators which have a fixed amount of state.
It allows the frontend to take a snapshot of all internal state, and later restore it.
This functionality is used to implement e.g. rewind and netplay.

Some important considerations must be taken to implement these functions well:

If serialization is not supported, retro_serialize_size() should return 0.
If retro_serialize_size() returns non-zero, it is assumed that serialization is properly implemented.
The frontend should call retro_serialize_size() before calling retro_serialize() to determine
the amount of memory needed to correctly serialize.
The size eventually passed to retro_serialize() must be at least the size of the value returned in retro_serialize_size().
If too large a buffer is passed to retro_serialize(), the extra data should be ignored (or memset to 0).
It is valid for the value returned by retro_serialize_size() to vary over time, however, it cannot ever increase over time.
If it should ever change, it must decrease.
This is rationalized by the ability to predetermined a fixed save state size right
after retro_load_game() that will always be large enough to hold any following serialization.
This certainty is fundamental to the rewind implementation. This requirement only holds between
calls to retro_load_game() and retro_unload_game().

If possible, the implementation should attempt to serialize data at consistent offsets in the memory buffer.
This will greatly help the rewind implementation in RetroArch to use less memory.
Both retro_serialize() and retro_unserialize() return a boolean value to let the frontend know
if the implementation succeeded in serializing or unserializing.
*/

//After the user desired to stop playing, retro_unload_game() will be called. 
//This should free any internal data related to the game, and allow retro_load_game() to be called again.
void retro_unload_game(void)
{
    game_deinit();
}


/*
# Thread safety
The libretro API does not make guarantees about thread safety.
Therefore the core developer should assume the functions declared in the libretro header are
neither reentrant nor safe to be called by multiple threads at the same time.
If a core is multi-threaded then the core developer is responsible for thread safety when making libretro API calls.

It is discouraged to do libretro API calls outside of retro_run() i.e. outside of the main thread.
*/


//blindly copied:


unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info* info, size_t num)
{
    return false;
}

size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void* data_, size_t size)
{
    return false;
}

bool retro_unserialize(const void* data_, size_t size)
{
    return false;
}

void* retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char* code)
{
    (void)index;
    (void)enabled;
    (void)code;
}

