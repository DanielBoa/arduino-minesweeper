# 💣 Arduino Minesweeper

![Minesweeper gameplay on 8x8 grid](example-video.gif)

A physical Minesweeper implementation built for [Adafruit NeoTrellis](https://www.adafruit.com/product/3954) powered by an Arduino (tested using Uno).

## How To Play

### Bombs

Instead of numbers being used to represent the number of bombs in neighbouring cells, colours are used. The colours are as follows:

1) Blue
2) Green
3) Pink
4) Cyan
5) Brown(ish)
6) Blue (less on)
7) Yellow
8) Red

### Controls

* Tap a cell to reveal it
* Hold a cell and release to flag/unflag
* ~Tap a coloured cell to clear all it's neighbours~ _(TODO)_

## Dependencies

* Built using the [Adafruit seesaw library.](https://github.com/adafruit/Adafruit_Seesaw)

## To Do

* Refactor to use OOP?
  * Code is currently a bit of a mess
  * Would be nice to have rendering logic etc. contained within a class so that more complex visual state could be held. I'm thinking pulsing colours.
* Difficulty selection
* Restart without hard reset
* Sound
* Choose better colours
  * Colours for 1, 2, & 3 are okay, but the others aren't great.
* Press a coloured square to clear neighbours
  * This would result in a gameover if a hidden neighbour is a bomb
* Explosion animation?
* Timed mode?
  * Have the board (or maybe just flags) flash faster to indicate time left
  * Would make the smaller game board a little more fun
* Test with larger NeoTrellis configuration (only tested with 2x2 NeoTrellis')