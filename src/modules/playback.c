#include "include/playback.h"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mscsound_playback_t *mscsound_create_playback(char *filename,
                                                    int framesPerBuffer) {
  mscsound_playback_t *playback = malloc(sizeof(mscsound_playback_t));
  playback->readCount = 0;
  playback->paused = 0;
  playback->loop = 0;
  playback->framesPerBuffer = framesPerBuffer;

  SNDFILE *sf;
  SF_INFO info;

  playback->filename = malloc(strlen(filename) + 1);
  strcpy(playback->filename, filename);

  sf = sf_open(playback->filename, SFM_READ, &info);
  if (!sf) {
    printf("Not able to open input file %s.\n", playback->filename);
    puts(sf_strerror(NULL));
    return NULL;
  }

  playback->file = malloc(info.frames * sizeof(float));
  playback->channels = info.channels;

  if (info.channels == 1) {
    playback->process = mscsound_playback_process_mono;
    playback->output0 = playback->file;
    playback->output1 = NULL;
  } else if (info.channels == 2) {
    playback->process = mscsound_playback_process_stereo;
    playback->output0 = calloc(framesPerBuffer, sizeof(float));
    playback->output1 = calloc(framesPerBuffer, sizeof(float));
  } else {
    printf("Support just Mono and Stereo Sound File\n");
    return NULL;
  }

  sf_read_float(sf, playback->file, info.frames);

  playback->fileFrames = info.frames;
  playback->fileSampleRate = info.samplerate;
  sf_close(sf);

  return playback;
}

void mscsound_playback_process_mono(mscsound_playback_t *playback) {
  int i = 0;

  if (!playback->paused) {
    for (i = 0; i < playback->framesPerBuffer &&
                playback->readCount < playback->fileFrames;
         i++) {
      playback->output0[i] = playback->file[playback->readCount];
      playback->readCount++;
      if (playback->loop && playback->readCount >= playback->fileFrames)
        playback->readCount = 0;
    }
    for (; i < playback->framesPerBuffer &&
           playback->readCount >= playback->fileFrames;
         i++) {
      playback->output0[i] = 0;
    }
    return;
  } else {
    for (i = 0; i < playback->framesPerBuffer; i++) {
      playback->output0[i] = 0;
    }
  }
}

void mscsound_playback_process_stereo(mscsound_playback_t *playback) {
  int i = 0;

  if (!playback->paused) {
    for (i = 0; i < playback->framesPerBuffer &&
                playback->readCount < playback->fileFrames;
         i++) {
      playback->output0[i] = playback->file[playback->readCount];
      playback->output1[i] = playback->file[playback->readCount + 1];
      playback->readCount += 2;
      if (playback->loop && playback->readCount >= playback->fileFrames)
        playback->readCount = 0;
    }
    for (; i < playback->framesPerBuffer &&
           playback->readCount >= playback->fileFrames;
         i++) {
      playback->output0[i] = 0;
      playback->output1[i] = 0;
    }
    return;
  } else {
    for (i = 0; i < playback->framesPerBuffer; i++) {
      playback->output0[i] = 0;
      playback->output1[i] = 0;
    }
  }
}
