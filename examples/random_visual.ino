// -------------------------------- НАСТРОЙКИ --------------------------------
// матрица
#define MTRX_BRIGHT 0        // яркость матрицы (0 - 15)
#define MTRX_COUNT_IN_ROW 4  // количество матриц горизонтально
#define MTRX_COUNT_IN_COL 1  // количество матриц вертикально
#define MTRX_PIXELS_IN_COLUMN 8
#define MTRX_PIXELS_IN_ROW (MTRX_PIXELS_IN_COLUMN * MTRX_COUNT_IN_ROW)
#define MTRX_PIXELS_IN_COL (MTRX_PIXELS_IN_COLUMN * MTRX_COUNT_IN_COL)
#include <GyverMAX7219.h>
MAX7219<MTRX_COUNT_IN_ROW, MTRX_COUNT_IN_COL, 12, A0, 13> matrix;

#include <SpectrumCalculator.h>

SpectrumCalculator spectrum_calculator(10, MTRX_PIXELS_IN_ROW, MTRX_PIXELS_IN_COLUMN);

// #define FILLING_UNDERLINE
const byte DRAW_OFFSET = ((MTRX_PIXELS_IN_ROW - spectrum_calculator.get_used_columns()) >> 1);

#define draw_line_v(x, y1, y2) matrix.fastLineV((x) + DRAW_OFFSET, MTRX_PIXELS_IN_COL - (y1), MTRX_PIXELS_IN_COL - (y2), GFX_FILL)
#define draw_line_up(x, h) draw_line_v(x, 0, h)
#define draw_dot(x, y) matrix.dot((x) + DRAW_OFFSET, MTRX_PIXELS_IN_COL - (y), GFX_FILL)

uint32_t data_refresh_time;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  matrix.setBright(MTRX_BRIGHT);
  matrix.textDisplayMode(GFX_REPLACE);
  matrix.clear();
  matrix.update();
  data_refresh_time = 0;
  spectrum_calculator.set_round_range(1);
}

void loop() {
  if (millis() - data_refresh_time > 2000) {
    data_refresh_time = millis();
    byte new_signals[spectrum_calculator.get_signals_count()];
    for (byte i = 0; i < spectrum_calculator.get_signals_count(); ++i) {
      int8_t value = random(-25, 125);
      new_signals[i] = constrain(value, 0, 100);
    }
    spectrum_calculator.put_signals(new_signals);
    byte signals[spectrum_calculator.get_signals_count()];
    spectrum_calculator.pull_signals(signals);
  }
  spectrum_calculator.tick();
  if (spectrum_calculator.is_changed()) {
    byte values_a[spectrum_calculator.get_used_columns()];
    byte values_b[spectrum_calculator.get_used_columns()];
    spectrum_calculator.pull_max_positions(values_a, values_b);
    matrix.clear();
    for (byte i = 0; i < spectrum_calculator.get_used_columns(); ++i) {
      draw_line_v(i, values_a[i], values_b[i]);
#ifdef FILLING_UNDERLINE
      draw_line_up(i, max(max(values_a[i], values_b[i]), 1) - 1);
#endif
    }

    matrix.update();
  }
}
