#include "Adafruit_NeoTrellis.h"

#define INT_PIN 10
#define Y_DIM 8
#define X_DIM 8
#define NUM_OF_MINES 7
#define TAP_THRES 400

const int NUM_OF_CELLS = Y_DIM * X_DIM;
const uint32_t HIDDEN_COLOR = seesaw_NeoPixel::Color(50, 50, 50);
const uint32_t MINE_COLOR = seesaw_NeoPixel::Color(255, 0, 0);
const uint32_t FLAG_COLOR = seesaw_NeoPixel::Color(255, 0, 0);
const uint32_t CLEARED_COLOR = seesaw_NeoPixel::Color(0, 0, 0);
const uint32_t BLUE_COLOR = seesaw_NeoPixel::Color(0, 0, 255);
const uint32_t GREEN_COLOR = seesaw_NeoPixel::Color(0, 255, 0);
const uint32_t YELLOW_COLOR = seesaw_NeoPixel::Color(240, 240, 25);
const uint32_t DARK_BLUE_COLOR = seesaw_NeoPixel::Color(25, 25, 125);
const uint32_t BROWN_COLOR = seesaw_NeoPixel::Color(100, 60, 20);
const uint32_t CYAN_COLOR = seesaw_NeoPixel::Color(10, 240, 240);
const uint32_t PINK_COLOR = seesaw_NeoPixel::Color(240, 10, 240);
const uint32_t RED_COLOR = seesaw_NeoPixel::Color(255, 0, 0);

struct CellState {
  int x;
  int y;
  int count;
  bool is_mine;
  bool is_flagged;
  bool is_hidden;
};

struct CellState cells[NUM_OF_CELLS];
bool is_pressing_down = false;
struct CellState *cell_being_pressed;
long pressed_down_time;
long time_started = millis();

// create a matrix of trellis panels
Adafruit_NeoTrellis t_array[Y_DIM/4][X_DIM/4] = {
  { Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x2E) },
  { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x2F) }
};

// pass this matrix to the multitrellis object
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)t_array, Y_DIM/4, X_DIM/4);

// Takes x/y coord and returns index in flat cell array,
// assuming array is populated from the top left.
int coord_to_index(int x, int y) {
  return (y * X_DIM) + x;
}

//define a callback for key presses
TrellisCallback trellis_callback(keyEvent evt){
  struct CellState *cell = &cells[evt.bit.NUM];

  if (is_pressing_down && evt.bit.NUM != coord_to_index(cell_being_pressed->x, cell_being_pressed->y)) return 0;
  
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    // down
    is_pressing_down = true;
    pressed_down_time = millis();
    cell_being_pressed = cell;
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    // up
    is_pressing_down = false;
    cell_being_pressed = NULL;
    cell_release_callback(cell, millis() - pressed_down_time); 
  }
  
  return 0;
}

uint32_t get_cell_color(struct CellState *cell) {
  if (cell->is_flagged) {
    return FLAG_COLOR;
  } else if (cell->is_hidden) {
    return HIDDEN_COLOR;
  } else if (cell->is_mine) {
    return MINE_COLOR;
  } else if (cell->count == 1) {
    return BLUE_COLOR;
  } else if (cell->count == 2) {
    return GREEN_COLOR;
  } else if (cell->count == 3) {
    return YELLOW_COLOR;
  } else if (cell->count == 4) {
    return DARK_BLUE_COLOR;
  } else if (cell->count == 5) {
    return BROWN_COLOR;
  } else if (cell->count == 6) {
    return CYAN_COLOR;
  } else if (cell->count == 7) {
    return PINK_COLOR;
  } else if (cell->count == 8) {
    return RED_COLOR;
  } else {
    return CLEARED_COLOR;
  }
}

bool cells_equal(struct CellState *a, struct CellState *b) {
  return (a->x == b->x) && (a->y == b->y);
}

void cell_hold_callback(struct CellState *cell, long length_of_press_ms) {
  if (!cell->is_hidden) return;

  long seconds = length_of_press_ms / 1000;
  int seconds_rounded = abs(seconds);

  if (seconds_rounded % 2 == 0) {
    cell->is_flagged = false;
  } else {
    cell->is_flagged = true;
  }
}

void cell_release_callback(struct CellState *cell, long length_of_press_ms) {
  if (!cell->is_hidden) return;
  if (length_of_press_ms > TAP_THRES) return;

  if (cell->is_mine) {
    for (int i = 0; i < NUM_OF_CELLS; i++) {
      trellis.setPixelColor(i, MINE_COLOR);
    }

    trellis.show();

    delay(1000);

    return;
  }

  recursively_reveal_cells(cell);
}

void recursively_reveal_cells(struct CellState *cell) {
  if (!cell->is_hidden || cell->is_mine) return;

  cell->is_hidden = false;

  if (cell->count > 0) return;

  for (int y_offset = -1; y_offset < 2; y_offset++) {
    int y = cell->y + y_offset;

    if ((y < 0) || (y > (Y_DIM - 1))) continue;

    for (int x_offset = -1; x_offset < 2; x_offset++) {
      int x = cell->x + x_offset;

      if ((x < 0) || (x > (X_DIM - 1))) continue;
      if ((y == cell->y) && (x == cell->x)) continue;

      struct CellState *neighbour = &cells[coord_to_index(x, y)];

      if (!neighbour->is_hidden) continue;
      if (neighbour->is_mine) continue;
   
      if (neighbour->count == 0) recursively_reveal_cells(neighbour);

      neighbour->is_hidden = false;
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(INT_PIN, INPUT);

  if (!trellis.begin()) {
    Serial.println("failed to begin trellis");
    while(1);
  }

  // init cells
  for (int y = 0; y < Y_DIM; y++) {
    for (int x = 0; x < X_DIM; x++) {
      struct CellState cell;

      cell.x = x;
      cell.y = y;
      cell.is_mine = false;
      cell.is_flagged = false;
      cell.is_hidden = false; // set after init

      cells[coord_to_index(x, y)] = cell;
    }
  }

  // place mines
  int num_of_mines_to_place = NUM_OF_MINES;  
  while(num_of_mines_to_place > 0) {
    int x = random(0, X_DIM);
    int y = random(0, Y_DIM);
    CellState *cell = &cells[coord_to_index(x, y)];

    if (!cell->is_mine) {
      cell->is_mine = true;
      num_of_mines_to_place--;
    }
  }
  
  // count mine neighbours for each cell
  for (int i = 0; i < NUM_OF_CELLS; i++) {
    struct CellState *cell = &cells[i];
    int mines = 0;

    for (int y_offset = -1; y_offset < 2; y_offset++) {
      int y = cell->y + y_offset;

      if ((y < 0) || (y > (Y_DIM - 1))) continue;

      for (int x_offset = -1; x_offset < 2; x_offset++) {
        int x = cell->x + x_offset;

        if ((x < 0) || (x > (X_DIM - 1))) continue;
        if ((y == cell->y) && (x == cell->x)) continue;

        struct CellState *neighbour = &cells[coord_to_index(x, y)];

        if (neighbour->is_mine) mines++;
      }
    }

    cell->count = mines;
  }

  // enable callbacks etc.
  for (int i = 0; i < NUM_OF_CELLS; i++) {
    struct CellState *cell = &cells[i];
    trellis.activateKey(cell->x, cell->y, SEESAW_KEYPAD_EDGE_RISING, true);
    trellis.activateKey(cell->x, cell->y, SEESAW_KEYPAD_EDGE_FALLING, true);
    trellis.registerCallback(cell->x, cell->y, trellis_callback);
    trellis.setPixelColor(cell->x, cell->y, get_cell_color(cell));

    trellis.show();

    delay(5);
  }

  delay(1000);

  for (int i = 0; i < NUM_OF_CELLS; i++) {
    struct CellState *cell = &cells[i];

    cell->is_hidden = true;
  }
}

void loop() {
  long ms_running = millis() - time_started;
  long seconds_running = ms_running / 250;
  int seconds_rounded = abs(seconds_running);
  bool flash_mine = (seconds_rounded % 2) == 0;

  if(!digitalRead(INT_PIN)){
    trellis.read();
  }

  if (is_pressing_down) {
    cell_hold_callback(cell_being_pressed, millis() - pressed_down_time);
  }
  
  for (int i = 0; i < NUM_OF_CELLS; i++) {
    if (cells[i].is_flagged && flash_mine && !cells_equal(&cells[i], cell_being_pressed)) {
      trellis.setPixelColor(cells[i].x, cells[i].y, HIDDEN_COLOR);
    } else {
      trellis.setPixelColor(cells[i].x, cells[i].y, get_cell_color(&cells[i]));
    }
  }

  trellis.show();

  delay(20);
}
