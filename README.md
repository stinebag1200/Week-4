# Conway’s Game of Life (wxWidgets)

This project is an implementation of **Conway’s Game of Life** using **C++ and wxWidgets**.  
It provides a graphical interface to simulate cellular automata with additional features such as saving/loading, importing patterns, a status bar, and a customizable heads-up display (HUD).

---

## 🚀 Features

### ✅ Basic Features
- **Grid Rendering**: The universe grid is drawn with toggleable cell states.  
- **Game Controls**:  
  - Start → Run continuous generations.  
  - Pause → Stop at current generation.  
  - Next → Advance one generation (when paused).  
- **Randomize Universe**: Populate the universe randomly.  
- **Clear/New Universe**: Reset the universe to empty cells.  
- **Save/Load**:  
  - Save the current universe to a text file.  
  - Open a saved universe (resizes grid to match file).  
- **Import Pattern**: Load a pattern into the existing universe **without resizing the grid** (optionally centered).  
- **Status Bar**: Displays current information such as generation count, living cells, etc.

### 🌟 Advanced Features
- **Heads-Up Display (HUD)**:  
  - Shows stats such as generation, cell count, boundary type, and universe size.  
  - Displayed at the bottom-left corner of the drawing panel.  
  - Toggle visibility from the *View → Show HUD* menu.  
- **Customizable Colors**: Select colors for grid, background, and live cells.  
- **Persistent Settings**:  
  - Universe size, colors, and HUD visibility are saved and restored between sessions.  
- **Reset Settings**: Restore application defaults.  

---

## 🖥️ Requirements

- **C++17 or newer**  
- **wxWidgets 3.2+** installed on your system  

---

## ⚙️ Build & Run

1. Clone or download the project.  
2. Ensure wxWidgets is installed and available in your environment.  
3. Compile the program:
   ```bash
   g++ -std=c++17 -o GameOfLife main.cpp `wx-config --cxxflags --libs`
