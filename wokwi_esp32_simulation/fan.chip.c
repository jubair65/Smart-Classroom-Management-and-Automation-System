#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 80
#define HEIGHT 80
#define CX 40
#define CY 40

typedef struct {
  pin_t in_pos;
  pin_t in_neg;
  timer_t anim_timer;
  buffer_t fb;
  uint32_t width;
  uint32_t height;
  float angle;
  bool is_on;
  uint32_t pixels[WIDTH * HEIGHT];
} fan_chip_t;

static inline uint32_t make_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
}

static void draw_fan(fan_chip_t *chip) {
  uint32_t col_bg      = make_rgba(20, 24, 30, 255);
  uint32_t col_chamber = make_rgba(10, 14, 18, 255);
  uint32_t col_ring    = make_rgba(50, 60, 70, 255);
  uint32_t col_hub     = make_rgba(35, 42, 50, 255);
  uint32_t col_blade   = chip->is_on ? make_rgba(0, 230, 255, 255) : make_rgba(85, 95, 110, 255);

  float ang = chip->angle;
  const float TWO_PI = 6.2831853f;
  const float HALF_PI = 1.5707963f;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int dx = x - CX;
      int dy = y - CY;
      int d2 = dx * dx + dy * dy;
      int idx = y * WIDTH + x;

      if (d2 > 38 * 38) {
        chip->pixels[idx] = col_bg;
      } else if (d2 > 34 * 34) {
        chip->pixels[idx] = col_ring;
      } else if (d2 <= 7 * 7) {
        chip->pixels[idx] = col_hub;
      } else {
        float pt_ang = atan2f((float)dy, (float)dx) - ang;
        while (pt_ang < 0.0f) pt_ang += TWO_PI;
        while (pt_ang >= TWO_PI) pt_ang -= TWO_PI;

        bool in_blade = false;
        for (int k = 0; k < 4; k++) {
          float blade_center = k * HALF_PI;
          float diff = fabsf(pt_ang - blade_center);
          if (diff > 3.14159f) diff = TWO_PI - diff;
          if (diff < 0.28f) {
            in_blade = true;
            break;
          }
        }
        chip->pixels[idx] = in_blade ? col_blade : col_chamber;
      }
    }
  }

  buffer_write(chip->fb, 0, (uint8_t *)chip->pixels, sizeof(chip->pixels));
}

static void on_timer(void *user_data) {
  fan_chip_t *chip = (fan_chip_t *)user_data;
  if (chip->is_on) {
    chip->angle += 0.35f;
    if (chip->angle >= 6.2831853f) chip->angle -= 6.2831853f;
    draw_fan(chip);
  }
}

static void on_pin_change(void *user_data, pin_t pin, uint32_t value) {
  fan_chip_t *chip = (fan_chip_t *)user_data;
  uint32_t pos = pin_read(chip->in_pos);
  uint32_t neg = pin_read(chip->in_neg);

  bool now_on = (pos == HIGH && neg == LOW);
  if (now_on != chip->is_on) {
    chip->is_on = now_on;
    if (chip->is_on) {
      timer_start(chip->anim_timer, 40000, true); // 40ms frame rate (25 FPS)
    } else {
      timer_stop(chip->anim_timer);
      draw_fan(chip);
    }
  }
}

void chip_init(void) {
  fan_chip_t *chip = (fan_chip_t *)calloc(1, sizeof(fan_chip_t));
  chip->in_pos = pin_init("IN+", INPUT_PULLDOWN);
  chip->in_neg = pin_init("IN-", INPUT_PULLDOWN);

  chip->fb = framebuffer_init(&chip->width, &chip->height);

  const pin_watch_config_t watch_cfg = {
    .edge = BOTH,
    .pin_change = on_pin_change,
    .user_data = chip,
  };
  pin_watch(chip->in_pos, &watch_cfg);
  pin_watch(chip->in_neg, &watch_cfg);

  const timer_config_t timer_cfg = {
    .callback = on_timer,
    .user_data = chip,
  };
  chip->anim_timer = timer_init(&timer_cfg);

  uint32_t pos = pin_read(chip->in_pos);
  uint32_t neg = pin_read(chip->in_neg);
  chip->is_on = (pos == HIGH && neg == LOW);
  chip->angle = 0.0f;
  if (chip->is_on) {
    timer_start(chip->anim_timer, 40000, true);
  }
  draw_fan(chip);
}
