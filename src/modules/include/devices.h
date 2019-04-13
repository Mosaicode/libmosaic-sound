#ifndef mscsound_DEVICES_H
#define mscsound_mscsound_DEVICES_H

#ifndef mscsound_PORTAUDIO
#define mscsound_PORTAUDIO
#include <portaudio.h>
#endif /* PortAudio */

typedef struct {
  char *name;
  int maxInputChannels;
  int maxOutputChannels;
  float defaultSampleRate;
} mscsound_device_info_t;

typedef struct {
  int id;
  int defaultDisplayed; /* 0: Input; 1: Output*/
  mscsound_device_info_t *deviceInfo;
} mscsound_device_t;

typedef struct {
  void *output0;
  void (*process)(void *self);
  void (*show)(void *devices);
} mscsound_device_list_t;

void mscsound_devices_process();
void mscsound_show_devices();

mscsound_device_t *mscsound_create_device(int deviceID);
mscsound_device_list_t *mscsound_create_devices();

#endif /* devices.h */
