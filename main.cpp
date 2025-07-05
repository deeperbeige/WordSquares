#include "trie.h"
#include <fstream>
#include <iostream>
#include <array>
#include <unordered_set>
#include <unordered_map>

//Path to the dictionary file
//Recommended source: https://raw.githubusercontent.com/andrewchen3019/wordle/refs/heads/main/Collins%20Scrabble%20Words%20(2019).txt
#define DICTIONARY "dict/collins_scrabble_words_2019.txt"
//Path to the word frequency file
//Recommended source: https://www.kaggle.com/datasets/wheelercode/dictionary-word-frequency
#define FREQ_FILTER "dict/ngram_freq_dict.csv"
//Width of the word grid
#define SIZE_W 4 // width (X)
//Height of the word grid
#define SIZE_H 4 // height (Y)
//Depth of the word grid
#define SIZE_D 3 // depth (Z)
//Filter horizontal words to be in the top-N (or 0 for all words)
#define MIN_FREQ_W 20000
//Filter vertical words to be in the top-N (or 0 for all words)
#define MIN_FREQ_H 20000
//Only print solutions with all unique words (only for square grids)
#define UNIQUE true
//Diagonals must also be words (only for square grids)
#define DIAGONALS false

static const int VTRIE_SIZE = (DIAGONALS ? SIZE_W + 2 : SIZE_W);
static const std::unordered_set<std::string> banned = {
  //Feel free to add words you don't want to see here
};

//Using global variables makes the recursive calls more compact
std::unordered_map<std::string, uint32_t> g_freqs;
Trie g_trie_w;
Trie g_trie_h;
Trie g_trie_x; // width
Trie g_trie_y; // height
Trie g_trie_z; // depth
char g_words[SIZE_W * SIZE_H * SIZE_D] = { 0 };

// Helper to index into the 3D array
inline int idx(int x, int y, int z) {
    return y * SIZE_W * SIZE_D + x * SIZE_D + z;
}

//Dictionary should be list of words separated by newlines
void LoadDictionary(const char* fname, int length, Trie& trie, int min_freq) {
  std::cout << "Loading Dictionary " << fname << "..." << std::endl;
  int num_words = 0;
  std::ifstream fin(fname);
  std::string line;
  while (std::getline(fin, line)) {
    if (line.size() != length) { continue; }
    for (auto& c : line) c = toupper(c);
    if (g_freqs.size() > 0 && min_freq > 0) {
      const auto& freq = g_freqs.find(line);
      if (freq == g_freqs.end() || freq->second > min_freq) { continue; }
    }
    if (banned.count(line) != 0) { continue; }
    trie.add(line);
    num_words += 1;
  }
  std::cout << "Loaded " << num_words << " words." << std::endl;
}

//Frequency list is expecting a sorted 2-column CSV with header
//First column is the word, second column is the frequency
void LoadFreq(const char* fname) {
  std::cout << "Loading Frequency List " << fname << "..." << std::endl;
  int num_words = 0;
  std::ifstream fin(fname);
  std::string line;
  bool first = false;
  while (std::getline(fin, line)) {
    if (first) { first = false; continue; }
    std::string str = line.substr(0, line.find_first_of(','));
    for (auto& c : str) c = toupper(c);
    g_freqs[str] = num_words;
    num_words += 1;
  }
  std::cout << "Loaded " << num_words << " words." << std::endl;
}

//Print a solution
void PrintBox(char* words) {
  //Do a uniqueness check if requested
  if (UNIQUE && SIZE_H == SIZE_W) {
    for (int i = 0; i < SIZE_H; ++i) {
      int num_same = 0;
      for (int j = 0; j < SIZE_W; ++j) {
        if (words[i * SIZE_W + j] == words[j * SIZE_W + i]) {
          num_same += 1;
        }
      }
      if (num_same == SIZE_W) { return; }
    }
  }
  //Print the grid
  for (int h = 0; h < SIZE_H; ++h) {
    for (int w = 0; w < SIZE_W; ++w) {
      std::cout << words[h * SIZE_W + w];
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void PrintCube(char* words) {
    for (int z = 0; z < SIZE_D; ++z) {
        std::cout << "Layer z=" << z << ":\n";
        for (int y = 0; y < SIZE_H; ++y) {
            for (int x = 0; x < SIZE_W; ++x) {
                std::cout << words[idx(x, y, z)];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void BoxSearch(Trie* trie, Trie* vtries[VTRIE_SIZE], int pos) {
  //Reset when coming back to first letter
  const int v_ix = pos % SIZE_W;
#if DIAGONALS
  const int h_ix = pos / SIZE_W;
#endif
  //Check if this is the beginning of a row
  if (v_ix == 0) {
    //If the entire grid is filled, we're done, print the solution
    if (pos == SIZE_H * SIZE_W) {
      PrintBox(g_words);
      return;
    }
    //Reset the horizontal trie position to the beginning
    trie = &g_trie_w;
  }
  Trie::Iter iter = trie->iter();
  while (iter.next()) {
    //Try next letter if vertical trie fails
    if (!vtries[v_ix]->hasIx(iter.getIx())) { continue; }
    //Show progress bar
    if (pos == 0) { std::cout << "=== [" << iter.getLetter() << "] ===" << std::endl; }
#if DIAGONALS
    if (h_ix == v_ix) {
      if (!vtries[VTRIE_SIZE - 2]->hasIx(iter.getIx())) { continue; }
    }
    if (h_ix == SIZE_W - v_ix - 1) {
      if (!vtries[VTRIE_SIZE - 1]->hasIx(iter.getIx())) { continue; }
    }
#endif
    //Letter is valid, update the solution
    g_words[pos] = iter.getLetter();
    //Make a backup of the vertical trie position in the stack for backtracking
    Trie* backup_vtrie = vtries[v_ix];
    //Update the vertical trie position
    vtries[v_ix] = vtries[v_ix]->decend(iter.getIx());
#if DIAGONALS
    Trie* backup_dtrie1 = vtries[VTRIE_SIZE - 2];
    Trie* backup_dtrie2 = vtries[VTRIE_SIZE - 1];
    if (h_ix == v_ix) {
      vtries[VTRIE_SIZE - 2] = vtries[VTRIE_SIZE - 2]->decend(iter.getIx());
    }
    if (h_ix == SIZE_W - v_ix - 1) {
      vtries[VTRIE_SIZE - 1] = vtries[VTRIE_SIZE - 1]->decend(iter.getIx());
    }
#endif
    //Make the recursive call
    BoxSearch(iter.get(), vtries, pos + 1);
    //After returning, restore the vertical trie position from the stack
    vtries[v_ix] = backup_vtrie;
#if DIAGONALS
    vtries[VTRIE_SIZE - 2] = backup_dtrie1;
    vtries[VTRIE_SIZE - 1] = backup_dtrie2;
#endif
  }
}

void CubeSearch(
    Trie* x_iters[SIZE_H][SIZE_D],
    Trie* y_iters[SIZE_W][SIZE_D],
    Trie* z_iters[SIZE_W][SIZE_H],
    int pos
) {
    int total = SIZE_W * SIZE_H * SIZE_D;
    if (pos == total) {
        PrintCube(g_words);
        return;
    }
    int x = (pos / (SIZE_D * SIZE_H)) % SIZE_W;
    int y = (pos / SIZE_D) % SIZE_H;
    int z = pos % SIZE_D;

    Trie* x_trie = x_iters[y][z];
    Trie* y_trie = y_iters[x][z];
    Trie* z_trie = z_iters[x][y];

    Trie::Iter iter = x_trie->iter();
    while (iter.next()) {
        int ix = iter.getIx();
        char letter = iter.getLetter();
        if (!y_trie->hasIx(ix)) continue;
        if (!z_trie->hasIx(ix)) continue;

        g_words[idx(x, y, z)] = letter;

        // Backup
        Trie* x_bak = x_iters[y][z];
        Trie* y_bak = y_iters[x][z];
        Trie* z_bak = z_iters[x][y];

        // Advance
        x_iters[y][z] = x_iters[y][z]->decend(ix);
        y_iters[x][z] = y_iters[x][z]->decend(ix);
        z_iters[x][y] = z_iters[x][y]->decend(ix);

        CubeSearch(x_iters, y_iters, z_iters, pos + 1);

        // Restore
        x_iters[y][z] = x_bak;
        y_iters[x][z] = y_bak;
        z_iters[x][y] = z_bak;
    }
}

int main(int argc, char* argv[]) {
    LoadFreq(FREQ_FILTER);

    LoadDictionary(DICTIONARY, SIZE_W, g_trie_x, MIN_FREQ_W);
    LoadDictionary(DICTIONARY, SIZE_H, g_trie_y, MIN_FREQ_H);
    LoadDictionary(DICTIONARY, SIZE_D, g_trie_z, MIN_FREQ_H);

    Trie* x_iters[SIZE_H][SIZE_D];
    Trie* y_iters[SIZE_W][SIZE_D];
    Trie* z_iters[SIZE_W][SIZE_H];

    for (int y = 0; y < SIZE_H; ++y)
        for (int z = 0; z < SIZE_D; ++z)
            x_iters[y][z] = &g_trie_x;
    for (int x = 0; x < SIZE_W; ++x)
        for (int z = 0; z < SIZE_D; ++z)
            y_iters[x][z] = &g_trie_y;
    for (int x = 0; x < SIZE_W; ++x)
        for (int y = 0; y < SIZE_H; ++y)
            z_iters[x][y] = &g_trie_z;

    CubeSearch(x_iters, y_iters, z_iters, 0);

    std::cout << "Done." << std::endl;
    return 0;
}
