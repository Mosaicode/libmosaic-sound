#include "include/adsr.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

void restart_adsr (mscsound_adsr_t **adsr) {
  (*adsr)->play = 0;
  (*adsr)->initialSample = 0;
  (*adsr)->currentSample = 0;
  (*adsr)->currentAmplitude = 0;
  (*adsr)->currentFrame = 0;
}

mscsound_adsr_t *mscsound_create_adsr(int sampleRate, int framesPerBuffer) {
  mscsound_adsr_t *adsr = malloc(sizeof(mscsound_adsr_t));

  adsr->output0 = calloc(1, sizeof(float*));
  adsr->framesPerBuffer = framesPerBuffer;
  adsr->sampleRate = sampleRate;
  adsr->attack = 0.0;
  adsr->decay = 0.0;
  adsr->sustain = 0.0;
  adsr->release = 0.0;
  adsr->gain = 0.8;
  adsr->play = 0;
  adsr->initialSample = 0;
  adsr->currentSample = 0;
  adsr->finalSample = 0;
  adsr->currentAmplitude = 0;
  adsr->currentFrame = 0;

  adsr->process = mscsound_process_silence;
  return adsr;
}

void mscsound_process_silence(mscsound_adsr_t **adsr){
  printf("Silence\n");
  *((*adsr)->output0) = *((*adsr)->input0);

  if (! (*adsr)->play) {
    for (int i = 0; i < (*adsr)->framesPerBuffer; i++)
      (*((*adsr)->output0))[i] = 0.0;
  } else {
    (*adsr)->process = mscsound_process_attack;
    (*adsr)->process(adsr);
  }
}

void mscsound_process_attack(mscsound_adsr_t **adsr) {
  printf("Attack\n");
  *((*adsr)->output0) = *((*adsr)->input0);

  if ((*adsr)->play)
    restart_adsr(adsr);

  (*adsr)->finalSample = ((*adsr)->sampleRate * (*adsr)->attack) / 1000;
  int amplitudeStep = 1 / ((*adsr)->finalSample - (*adsr)->initialSample - 1);

  int remainingSamples = (*adsr)->finalSample - (*adsr)->currentSample;
  if (remainingSamples > (*adsr)->framesPerBuffer) {
    for (int i = (*adsr)->currentFrame; i < (*adsr)->framesPerBuffer; i++) {
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
      ((*adsr)->currentAmplitude) += amplitudeStep;
    }

  } else {
    for (int i = (*adsr)->currentFrame; i < remainingSamples; i++) {
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
      ((*adsr)->currentAmplitude) += amplitudeStep;
    }

    (*adsr)->process = mscsound_process_decay;
    if (remainingSamples != (*adsr)->framesPerBuffer) {}
      (*adsr)->process(adsr);

    ((*adsr)->currentFrame) = 0;
  }
}


void mscsound_process_decay(mscsound_adsr_t **adsr) {
  printf("Decay\n");
  *((*adsr)->output0) = *((*adsr)->input0);

  (*adsr)->currentAmplitude = 1;
  if ((*adsr)->play) {
    restart_adsr(adsr);
    (*adsr)->process = mscsound_process_attack;
    (*adsr)->process(adsr);
    return;
  }

  (*adsr)->initialSample = (*adsr)->finalSample;
  (*adsr)->finalSample += ((*adsr)->sampleRate * (*adsr)->decay) / 1000;

  int amplitudeStep = ((*adsr)->currentAmplitude - (*adsr)->gain) / \
            ((*adsr)->finalSample - (*adsr)->initialSample);


  int remainingSamples = (*adsr)->finalSample - (*adsr)->currentSample;

  if (remainingSamples > (*adsr)->framesPerBuffer) {
    for (int i = (*adsr)->currentFrame; i < (*adsr)->framesPerBuffer; i++) {
      ((*adsr)->currentAmplitude) -= amplitudeStep;
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
    }

  } else {
    for (int i = (*adsr)->currentFrame; i < remainingSamples; i++) {
      ((*adsr)->currentAmplitude) -= amplitudeStep;
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
    }

    (*adsr)->process = mscsound_process_sustain;
    if (remainingSamples != (*adsr)->framesPerBuffer)
      (*adsr)->process(adsr);

    ((*adsr)->currentFrame) = 0;
  }
}

void mscsound_process_sustain(mscsound_adsr_t **adsr) {
  printf("Sustain\n");
  *((*adsr)->output0) = *((*adsr)->input0);

  if ((*adsr)->play) {
    restart_adsr(adsr);
    (*adsr)->process = mscsound_process_attack;
    (*adsr)->process(adsr);
    return;
  }

  (*adsr)->initialSample = (*adsr)->finalSample;
  (*adsr)->finalSample += ((*adsr)->sampleRate * (*adsr)->sustain) / 1000;

  int remainingSamples = (*adsr)->finalSample - (*adsr)->currentSample;

  if (remainingSamples > (*adsr)->framesPerBuffer) {
    for (int i = (*adsr)->currentFrame; i < (*adsr)->framesPerBuffer; i++) {
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
    }

  } else {
    for (int i = (*adsr)->currentFrame; i < remainingSamples; i++) {
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
    }

    (*adsr)->process = mscsound_process_release;
    if (remainingSamples != (*adsr)->framesPerBuffer)
      (*adsr)->process(adsr);
    ((*adsr)->currentFrame) = 0;
  }
}

void mscsound_process_release(mscsound_adsr_t **adsr) {
  printf("Release\n");
  *((*adsr)->output0) = *((*adsr)->input0);

  if ((*adsr)->play) {
    restart_adsr(adsr);
    (*adsr)->process = mscsound_process_attack;
    (*adsr)->process(adsr);
    return;
  }

  (*adsr)->initialSample = (*adsr)->finalSample;
  (*adsr)->finalSample += ((*adsr)->sampleRate * (*adsr)->release) / 1000;

  int amplitudeStep = (*adsr)->currentAmplitude / \
            ((*adsr)->finalSample - (*adsr)->initialSample);


  int remainingSamples = (*adsr)->finalSample - (*adsr)->currentSample;

  if (remainingSamples > (*adsr)->framesPerBuffer) {
    for (int i = (*adsr)->currentFrame; i < (*adsr)->framesPerBuffer; i++) {
      ((*adsr)->currentAmplitude) -= amplitudeStep;
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
    }

  } else {
    for (int i = (*adsr)->currentFrame; i < remainingSamples; i++) {
      ((*adsr)->currentAmplitude) -= amplitudeStep;
      (*((*adsr)->output0))[i] *= (*adsr)->currentAmplitude;
      ((*adsr)->currentFrame)++;
      ((*adsr)->currentSample)++;
    }

    (*adsr)->process = mscsound_process_silence;
    if (remainingSamples != (*adsr)->framesPerBuffer) {
      for (int i = (*adsr)->currentFrame; i < (*adsr)->framesPerBuffer; i++)
        (*((*adsr)->output0))[i] = 0.0;
    }

    restart_adsr(adsr);
  }
}