#include "include/midi.h"
#include <alsa/asoundlib.h>
#include <pthread.h>

typedef void (*mscsound_midi_t_event_callback_function)(snd_seq_event_t *event);

snd_seq_t *get_handle(mscsound_midi_t **self) {
  return (snd_seq_t *)((*self)->handle);
}

void *mscsound_midi_thread(void *data) {
  mscsound_midi_t *midi = *((mscsound_midi_t **)data);

  if (midi->callback == NULL) {
    pthread_exit((void *)NULL);
  }
  snd_seq_event_t *ev;
  while (1) {
    snd_seq_event_input(get_handle(&midi), &ev);
    printf("Chegou evento!!\n");
    (midi->callback)(ev);
  }
}

mscsound_midi_t *mscsound_create_midi(
    const char *device, int port_type,
    mscsound_midi_t_event_callback_function event_callback_function) {

  mscsound_midi_t *midi = malloc(sizeof(mscsound_midi_t));
  midi->callback = event_callback_function;

  midi->send_note = mscsound_midi_send_note;
  midi->send_control = mscsound_midi_send_control;

  int portid;

  snd_seq_t *handle;
  midi->handle = handle;
  if (snd_seq_open((snd_seq_t **)&(midi->handle), "hw", port_type, 0) < 0) {
    return NULL;
  }
  snd_seq_set_client_name(get_handle(&midi), device);

  if (port_type == SND_SEQ_OPEN_DUPLEX || port_type == SND_SEQ_OPEN_INPUT) {
    if ((portid = snd_seq_create_simple_port(
             get_handle(&midi), "input",
             SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
             SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      return NULL;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, mscsound_midi_thread, &midi);
  }

  if (port_type == SND_SEQ_OPEN_DUPLEX || port_type == SND_SEQ_OPEN_OUTPUT) {
    if ((portid = snd_seq_create_simple_port(
             get_handle(&midi), "output",
             SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
             SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      return NULL;
    }
  }
  return midi;
}

void mscsound_midi_send_note(void *self, int type, int channel,
                             unsigned char vel, unsigned char note) {
  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, 0);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_direct(&ev);
  snd_seq_ev_set_fixed(&ev);

  ev.type = type; // SND_SEQ_EVENT_NOTEOFF;
  ev.data.note.channel = channel;
  ev.data.note.velocity = vel;
  ev.data.note.note = note;
  snd_seq_event_output(get_handle(self), &ev);
  snd_seq_drain_output(get_handle(self));
}

void mscsound_midi_send_control(void *self, int channel, int control,
                                int value) {
  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, 0);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_direct(&ev);
  snd_seq_ev_set_fixed(&ev);

  ev.type = SND_SEQ_EVENT_CONTROLLER;
  ev.data.control.channel = channel;
  ev.data.control.param = control;
  ev.data.control.value = value;

  snd_seq_event_output(get_handle(self), &ev);
  snd_seq_drain_output(get_handle(self));
}
