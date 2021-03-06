#ifndef MSCSOUND_H
#define MSCSOUND_H

/* gui */
#include "/usr/include/mosaic/mosaic-sound/include/grid.h"
#include "/usr/include/mosaic/mosaic-sound/include/gui.h"
#include "/usr/include/mosaic/mosaic-sound/include/volume.h"
#include "/usr/include/mosaic/mosaic-sound/include/vubar.h"
#include "/usr/include/mosaic/mosaic-sound/include/waveform.h"

/* joystick */
#include "/usr/include/mosaic/mosaic-sound/include/joystick.h"

/* midi */
#include "/usr/include/mosaic/mosaic-sound/include/midi.h"

/* osc */

/* sound */
#include "/usr/include/mosaic/mosaic-sound/include/adsr.h"
#include "/usr/include/mosaic/mosaic-sound/include/audiofloatmath.h"
#include "/usr/include/mosaic/mosaic-sound/include/audiomath.h"
#include "/usr/include/mosaic/mosaic-sound/include/biquad.h"
#include "/usr/include/mosaic/mosaic-sound/include/channelshootersplitter.h"
#include "/usr/include/mosaic/mosaic-sound/include/delay.h"
#include "/usr/include/mosaic/mosaic-sound/include/devices.h"
#include "/usr/include/mosaic/mosaic-sound/include/highshelving.h"
#include "/usr/include/mosaic/mosaic-sound/include/lowshelving.h"
#include "/usr/include/mosaic/mosaic-sound/include/mic.h"
#include "/usr/include/mosaic/mosaic-sound/include/oscillators.h"
#include "/usr/include/mosaic/mosaic-sound/include/parametricequalizer.h"
#include "/usr/include/mosaic/mosaic-sound/include/playback.h"
#include "/usr/include/mosaic/mosaic-sound/include/record.h"
#include "/usr/include/mosaic/mosaic-sound/include/rms.h"
#include "/usr/include/mosaic/mosaic-sound/include/speaker.h"
#include "/usr/include/mosaic/mosaic-sound/include/whitenoise.h"

/* util */
#include "/usr/include/mosaic/mosaic-sound/include/list.h"

static int mscsound_callback();
static void mscsound_finished(void *data);
/*
 * This routine is called by portaudio when playbak is done.
 */
static void StreamFinished(void *data) { mscsound_finished(data); }

static void *mscsound_initialize(int sampleRate, int framesPerBuffer) {
  PaStreamParameters inputParameters, outputParameters;
  PaStream *stream;
  PaError err;

  mscsound_midi_initialize();

  err = Pa_Initialize();
  if (err != paNoError)
    goto error;

  /*inputParameters*/
  inputParameters.device =
      Pa_GetDefaultInputDevice(); /* default input device */
  if (inputParameters.device == paNoDevice)
    goto error;

  inputParameters.channelCount = 1; /* mono input */
  inputParameters.sampleFormat = paFloat32;
  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = 0;

  /*-------------------------------------------------------------------------*/
  /*outputParameters*/
  outputParameters.device =
      Pa_GetDefaultOutputDevice(); /* default output device */
  if (outputParameters.device == paNoDevice)
    goto error;

  outputParameters.channelCount = 1;         /* mono output */
  outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
  outputParameters.suggestedLatency =
      Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = 0;
  err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, sampleRate,
                      framesPerBuffer,
                      paClipOff, /* we won't output out of range samples so
                                    don't bother clipping them */
                      mscsound_callback, 0);
  if (err != paNoError)
    goto error;

  err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
  if (err != paNoError)
    goto error;

  err = Pa_StartStream(stream);
  if (err != paNoError)
    goto error;

  return (void *)stream;

error:
  Pa_Terminate();
  return 0;
}

static void mscsound_terminate(void *stream) {
  PaError err;
  stream = (PaStream *)stream;
  err = Pa_StopStream(stream);
  if (err != paNoError)
    goto error;

  err = Pa_CloseStream(stream);
  if (err != paNoError)
    goto error;

  Pa_Terminate();

  return;
error:
  Pa_Terminate();
}

#endif /* mosaic-sound.h */
