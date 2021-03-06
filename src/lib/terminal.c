#include <lib/terminal.h>

CIRCULAR_BUFFER_DEF(tdisp_cb, char, TERMINAL_BUFFER_SIZE);
CIRCULAR_BUFFER_DEF(wid_cb, int, MAX_WINDOWS);

void tdisp_cb_pushstr(tdisp_cb *buf, char *str) {
  while (*str != '\0')
    tdisp_cb_push(buf, *str++);
}

int tdisp_cb_pushui32(tdisp_cb *buf, unsigned int i) {
  int n;
  int dgt;
  char c;
  unsigned int d;

  d = 1;
  n = 0;
  while ((i / d) >= 10) d *= 10;
  while (d != 0) {
    dgt = i / d;
    i %= d;
    d /= 10;
    if (n || dgt > 0 || d == 0) {
      c = dgt + (dgt < 10 ? '0' : 'a' - 10);
      tdisp_cb_push(buf, c);
      ++n;
    }
  }
  return n;
}

void tcursor_init(TCursor *tc, int x, int y) {
  tc->x = x;
  tc->y = y;
}

void twindow_init(TWindow *tw, int wid, int x, int y, int w, int h, tid_t tid) {
  tcursor_init(&tw->cur, 1, 1);
  tw->wid = wid;
  tw->offsetx = x;
  tw->offsety = y;
  tw->w = w;
  tw->h = h;
  tw->tid = tid;
}

void tdisp_init(TDisplay *td) {
  int i;
  td->num_active_windows = 0;
  td->focused_window = 0;
  td->active_task = -1;
  tcursor_init(&td->tdcur, -1, -1);
  tdisp_cb_init(&td->buffer);
  wid_cb_init(&td->avail_wids);

  for (i = 0; i < MAX_WINDOWS; ++i) {
    twindow_init(&td->windows[i], -1, -1, -1, -1, -1, -1);
    td->active_windows[i] = 0;
    wid_cb_push(&td->avail_wids, i);
  }
}


char tdisp_pop(TDisplay *td) {
  int r;
  char c;

  r = tdisp_cb_pop(&td->buffer, &c);
  assert(r == 0);
  return c;
}

// set td cursor relative to focused window
void tdisp_set_cursor(TDisplay *td, int x, int y, bool wrap) {
  TWindow *window;
  int ret;
  tdisp_cb *buf;
  buf = &td->buffer;
  window = td->focused_window;

  x += window->offsetx;
  y += window->offsety;

  // if (/*wrap&&*/ x > window->offsetx + window->w /*- 1*/) {
  //   window->cur.x = 3; // wrap the cursor by default
  //   window->cur.y++;
  //   x = window->offsetx;
  //   y = window->offsety;
  // }
  // if (/*wrap &&*/ y > window->offsety + window->h /*- 1*/) {
  //   window->cur.x = 1;
  //   window->cur.y = 1;
  //   x = window->offsetx;
  //   y = window->offsety;
  // }
  // assert(x >= window->offsetx && x <= window->offsetx + window->w);
  // assert(y >= window->offsety && y <= window->offsety + window->h);

  if (td->tdcur.x != x || td->tdcur.y != y) {
    /*
    if (td->active_task == window->tid) {
      ret = tdisp_cb_push(buf, '\033');
      ret = tdisp_cb_push(buf, '[');
      ret = tdisp_cb_push(buf, '?');
      ret = tdisp_cb_push(buf, '2');
      ret = tdisp_cb_push(buf, '5');
      ret = tdisp_cb_push(buf, 'h');
    }
    else {
      ret = tdisp_cb_push(buf, '\033');
      ret = tdisp_cb_push(buf, '[');
      ret = tdisp_cb_push(buf, '?');
      ret = tdisp_cb_push(buf, '2');
      ret = tdisp_cb_push(buf, '5');
      ret = tdisp_cb_push(buf, 'l');
    }
    */
    ret = tdisp_cb_push(buf, '\033');
    ret = tdisp_cb_push(buf, '[');
    ret = tdisp_cb_pushui32(buf, y);
    ret = tdisp_cb_push(buf, ';');
    ret = tdisp_cb_pushui32(buf, x);
    ret = tdisp_cb_push(buf, 'H');
    assert(ret == 0);
  }

  td->tdcur.x = x;
  td->tdcur.y = y;
}

// resets the cursor to the focused window's cursor position
void tdisp_reset_cursor(TDisplay *td) {
  TWindow *window;
  window = td->focused_window;
  tdisp_set_cursor(td, window->cur.x, window->cur.y, true);
}

void tdisp_set_active_window(TDisplay *td, int wid) {
  td->focused_window = &td->windows[wid];
}

void tdisp_draw_window_outline(TDisplay *td) {
  int i, j, w, h;
  TWindow *window;
  window = td->focused_window;

  w = window->w;
  h = window->h;

  for (i = 0; i <= h; ++i) {
    if (i == 0 || i == h) {
      tdisp_set_cursor(td, 0, i, false);
      tdisp_cb_pushstr(&td->buffer, i == 0 ? TERM_TOP_L: TERM_BOT_L);
      if (i == 0) {
        int n;
        if (window->tid == -1)
          n = tdisp_cb_pushui32(&td->buffer, 0);
        else
          n = tdisp_cb_pushui32(&td->buffer, window->tid);
        for (j = n; j < w-2; ++j)
          tdisp_cb_pushstr(&td->buffer, TERM_HOR);
      }
      else {
        for (j = 0; j < w-2; ++j)
          tdisp_cb_pushstr(&td->buffer, TERM_HOR);
      }
      tdisp_cb_pushstr(&td->buffer, i == 0 ? TERM_TOP_R : TERM_BOT_R);
    }
    else {
      tdisp_set_cursor(td, 0, i, false);
      tdisp_cb_pushstr(&td->buffer, TERM_VER);
      tdisp_set_cursor(td, w-1, i, false);
      tdisp_cb_pushstr(&td->buffer, TERM_VER);
    }
  }
}

void tdisp_clear_window(TDisplay *td) {
  int i, j, w, h;
  TWindow *window;
  window = td->focused_window;

  w = window->w;
  h = window->h;

  for (i = 1; i < h; ++i) {
    tdisp_set_cursor(td, 1, i, false);
    for (j = 0; j < w-2; ++j) {
      tdisp_cb_push(&td->buffer, ' ');
    }
  }
}

void tdisp_clear_whole_window(TDisplay *td) {
  int i, j, w, h;
  TWindow *window;
  window = td->focused_window;

  w = window->w;
  h = window->h;

  for (i = 0; i <= h; ++i) {
    tdisp_set_cursor(td, 0, i, false);
    for (j = 0; j < w; ++j) {
      tdisp_cb_push(&td->buffer, ' ');
    }
  }
}

int tdisp_add_window(TDisplay *td, int x, int y, int w, int h, tid_t tid) {
  int r, wid;
  TWindow *window;
  tid_id_t id;

  id = TID_ID(tid);

  if (td->avail_wids.size < 1 || id < 0 || id > MAX_TASK-1) {
    return -1;
  }

  r = wid_cb_pop(&td->avail_wids, &wid);
  assert(r == 0);
  assert(wid >= 0 && wid <= MAX_WINDOWS);
  td->active_windows[wid] = 1;
  td->task_window[TID_ID(tid)] = wid;
  window = &td->windows[wid];
  twindow_init(window, wid, x, y, w, h, tid);
  tdisp_set_active_window(td, wid);
  tdisp_draw_window_outline(td);
  tdisp_reset_cursor(td);
  return wid;
}

void tdisp_focus_window(TDisplay *td, int wid) {
  if (!td->active_windows[wid])
    return;
  assert(td->active_windows[wid]);
  TWindow *window;
  window = &td->windows[wid];
  td->focused_window = window;
  tdisp_reset_cursor(td);
}

void tdisp_set_active_task(TDisplay *td, tid_t tid) {
  td->active_task = tid;
  assert(td->task_window[tid] != -1);
  tdisp_focus_window(td, td->task_window[tid]);
}

void tdisp_delete_window(TDisplay *td) {
  TWindow *fwindow;
  int i, r;
  int wid;
  fwindow = td->focused_window;
  wid = fwindow->wid;

  tdisp_clear_whole_window(td);
  td->active_windows[wid] = 0;
  td->task_window[TID_ID(fwindow->tid)] = -1;
  r = wid_cb_push(&td->avail_wids, wid);
  assert(r == 0);
  td->focused_window = 0;

  if (td->active_task == fwindow->tid)
    td->active_task = -1;

  // focus another window if it exists
  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (td->active_windows[i]) {
      tdisp_focus_window(td, i);
      break;
    }
  }
}

void tdisp_move_window(TDisplay *td, int x, int y) {
  TWindow *fwindow;
  int wid;
  fwindow = td->focused_window;
  wid = fwindow->wid;

  tdisp_clear_whole_window(td);
  twindow_init(fwindow, wid, x, y, fwindow->w, fwindow->h, fwindow->tid);
  tdisp_draw_window_outline(td);
  tdisp_reset_cursor(td);
}

// writes a character to the active window
void tdisp_writec(TDisplay *td, char c) {
  TWindow *fwindow;
  fwindow = td->focused_window;
  if (fwindow == 0)
    return;

  tdisp_reset_cursor(td);

  switch (c) {
    case TERM_RESET:
      fwindow->cur.y = 1;
      fwindow->cur.x = 1;
      tdisp_reset_cursor(td);
      break;
    case TERM_RETURN:
      fwindow->cur.y = 1;
      fwindow->cur.x = 1;
      tdisp_clear_window(td);
      tdisp_reset_cursor(td);
      break;
    case TERM_NEWLINE:
      fwindow->cur.y++;
      fwindow->cur.x = 1;
      tdisp_reset_cursor(td);
      break;
    case BACKSPACE:
      fwindow->cur.x--;
      td->tdcur.x--;
      tdisp_cb_push(&td->buffer, c);
      break;
    default:
      fwindow->cur.x++;
      td->tdcur.x++;
      tdisp_cb_push(&td->buffer, c);
      break;
  }
}

int tdisp_get_task(TDisplay *td) {
  if (td->focused_window == 0)
    return -1;
  return td->focused_window->tid;
}

int tdisp_focus_task_window(TDisplay *td, tid_t tid) {
  tid_id_t id = TID_ID(tid);
  if (id < 0 || MAX_TASK < id)
    return -1;

  int wid = td->task_window[id];
  tdisp_focus_window(td, wid);

  return 0;
}

void tdisp_write_task(TDisplay *td, tid_t tid, char c) {
  assert(td->focused_window != 0);
  int wid, r;
  wid = td->focused_window->wid;
  r = tdisp_focus_task_window(td, tid);
  assert(r == 0);
  tdisp_writec(td, c);
  // tdisp_focus_window(td, wid);
}

void tdisp_writes_task(TDisplay *td, tid_t tid, const char *c, int len) {
  assert(td->focused_window != 0);
  int wid, r, i;
  wid = td->focused_window->wid;
  r = tdisp_focus_task_window(td, tid);
  assert(r == 0);
  for (i = 0; i < len; ++i) {
    tdisp_writec(td, c[i]);
  }
}



