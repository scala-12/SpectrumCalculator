#pragma once

#include <Arduino.h>

class SpectrumCalculator {
 public:
  SpectrumCalculator(const byte signals_count, const byte columns_count, const byte rows_count);

  bool tick();
  bool is_changed();

  byte get_columns_for_signal();
  byte get_signals_count();
  byte get_used_columns();
  byte get_rows_count();

  void set_move_period(byte period);
  void set_round_range(byte range_value);

  void put_signals(byte* signals);

  void pull_signals(byte* signals);
  void pull_max_positions(byte* values_a, byte* values_b);

 private:
  byte _signals_count;
  byte _rows_count;
  byte _columns_for_signal;
  byte _used_columns;

  byte _move_period;
  byte _round_range;

  bool _is_moved;
  uint32_t _move_time;

  struct _SignalInfo {
    byte now_value;
    bool is_up_direction;
    byte* next_levels;
  };

  _SignalInfo* _signals_info;
};

bool _is_success_delay(uint32_t& last_time, uint32_t timeout) {
  uint32_t current = millis();

  uint32_t before_over = -1 - last_time;
  if (before_over > timeout) {
    return ((max(current, last_time) - min(current, last_time)) >= timeout);
  }

  return before_over + timeout <= current;
}

const byte _DEFAUL_MOVE_PERIOD = 70;
const byte _REFRESH_DELAY = 4;

SpectrumCalculator::SpectrumCalculator(const byte signals_count, const byte columns_count, const byte rows_count) {
  _signals_count = signals_count;
  _rows_count = rows_count;
  _columns_for_signal = columns_count / _signals_count;
  _used_columns = _signals_count * _columns_for_signal;
  _move_period = _DEFAUL_MOVE_PERIOD;
  _round_range = 1;

  _is_moved = true;
  _move_time = 0;
  _signals_info = new _SignalInfo[signals_count];
  for (byte i = 0; i < _signals_count; ++i) {
    _signals_info[i].next_levels = new byte[_columns_for_signal];
    memset(_signals_info[i].next_levels, 1, _columns_for_signal);
    _signals_info[i].now_value = 0;
    _signals_info[i].is_up_direction = true;
  }
}

bool SpectrumCalculator::tick() {
  bool refreshed = _is_success_delay(_move_time, _move_period);
  if (refreshed) {
    _move_time = millis();

    for (byte i = 0; i < _signals_count; ++i) {
      byte level = map(_signals_info[i].now_value, 0, 100, 1, _rows_count);
      for (byte j = 0; j < _columns_for_signal; ++j) {
        if ((_signals_info[i].next_levels[j] != level && _signals_info[i].is_up_direction ^ _signals_info[i].next_levels[j] > level) && (max(_signals_info[i].next_levels[j], level) - min(_signals_info[i].next_levels[j], level) > _round_range)) {
          _signals_info[i].next_levels[j] += random(1, max(_signals_info[i].next_levels[j], level) - min(_signals_info[i].next_levels[j], level)) * ((_signals_info[i].next_levels[j] > level) ? -1 : 1);
          if (!_is_moved) {
            _is_moved = true;
          }
        }
      }
    }
  }

  return refreshed;
}

bool SpectrumCalculator::is_changed() {
  if (_is_moved) {
    _is_moved = false;
    return true;
  }
  return false;
}

byte SpectrumCalculator::get_columns_for_signal() {
  return _columns_for_signal;
}

byte SpectrumCalculator::get_signals_count() {
  return _signals_count;
}

byte SpectrumCalculator::get_used_columns() {
  return _used_columns;
}

byte SpectrumCalculator::get_rows_count() {
  return _rows_count;
}

void SpectrumCalculator::set_move_period(byte period) {
  _move_period = period;
}

void SpectrumCalculator::set_round_range(byte range_value) {
  _round_range = constrain(range_value, 1, _rows_count);
}

void SpectrumCalculator::put_signals(byte* signals) {
  for (byte i = 0; i < _signals_count; ++i) {
    if (_signals_info[i].now_value != signals[i]) {
      _signals_info[i].is_up_direction = _signals_info[i].now_value < signals[i];
    }
    _signals_info[i].now_value = signals[i];
  }
}

void SpectrumCalculator::pull_signals(byte* signals) {
  for (byte i = 0; i < _signals_count; ++i) {
    signals[i] = _signals_info[i].now_value;
  }
}

void SpectrumCalculator::pull_max_positions(byte* values_a, byte* values_b) {
  for (byte i = 0; i < _used_columns; ++i) {
    values_a[i] = _signals_info[i / _columns_for_signal].next_levels[i % _columns_for_signal];
    values_b[i] = values_a[i];
  }

  for (byte i = 0; i < _used_columns; ++i) {
    byte min_value = _rows_count;
    if (i > 0) {
      min_value = values_a[i - 1];
    }
    if (i < _used_columns - 1) {
      if (min_value > values_a[i + 1]) {
        min_value = values_a[i + 1];
      }
    }

    if (values_a[i] > min_value && values_a[i] - min_value > 1) {
      if (_columns_for_signal > 1) {
        --values_a[i];
      }
      values_b[i] = min_value + 1;
    }
  }
}
