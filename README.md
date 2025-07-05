# FORK PURPOSE
Extend the original work into 3D.
Generate puzzles for Arthur Owl's Word Block game.


# Arthur Owl's Word Block (Free VR puzzle game)
(Quest 2/Pro/3) https://www.meta.com/en-gb/experiences/5821234647962627
(Rift/S/PC VR/Quest Link) https://www.meta.com/en-gb/experiences/pcvr/arthur-owls-word-block/6128272427233921/ 
(Steam) https://store.steampowered.com/app/3037000/Arthur_Owls_Word_Block/


# WordSquares
A simple C++ solver for dense word grids.

See this [YouTube Video](https://youtu.be/zWIsnrxL-Zc) for the best explanation.

**NOTE:** This repository is not actively maintained and is for reference only.

## Usage
To use this solver, you'll need a list of valid words and a word frequency list. If you'd like to use the same files as shown in the video, download links are provided below.

[Scrabble Words List](https://raw.githubusercontent.com/andrewchen3019/wordle/refs/heads/main/Collins%20Scrabble%20Words%20(2019).txt)

[NGram Viewer Frequencies](https://www.kaggle.com/datasets/wheelercode/dictionary-word-frequency)

You'll need to update the `DICTIONARY` and `FREQ_FILTER` paths to reference these files. These and other parameters are found on the top of `main.cpp`. Modify these to try different kinds of word grids. Basic documentation is provided in this file.
