#include <vector>
#include <random>
#include <algorithm>

enum TileType {
    EMPTY,        // No building
    SMALL_BUILDING,
    MEDIUM_BUILDING,
    TALL_BUILDING,
    PAVEMENT
};

struct Tile {
    TileType type;
    bool collapsed;
    std::vector<TileType> possibleStates;
};

class WFCGrid {
public:
    int width, height;
    std::vector<std::vector<Tile>> grid;

    WFCGrid(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<Tile>(width, Tile{EMPTY, false, {SMALL_BUILDING, MEDIUM_BUILDING, TALL_BUILDING, PAVEMENT}}));
    }

    // Check if all cells are collapsed
    bool isFullyCollapsed() const {
        for (const auto& row : grid) {
            for (const auto& tile : row) {
                if (!tile.collapsed) {
                    return false;
                }
            }
        }
        return true;
    }

    // Randomly collapse a cell and propagate constraints
    void collapse() {
        // Find the cell with the fewest possible states
        Tile* cell = nullptr;
        for (auto& row : grid) {
            for (auto& t : row) {
                if (!t.collapsed && (cell == nullptr || t.possibleStates.size() < cell->possibleStates.size())) {
                    cell = &t;
                }
            }
        }

        if (!cell) return; // All cells collapsed

        // Randomly choose a state for this cell
        TileType chosen = cell->possibleStates[rand() % cell->possibleStates.size()];
        cell->type = chosen;
        cell->collapsed = true;

        // Propagate constraints to neighbors
        propagateConstraints();
    }

private:
    void propagateConstraints() {
        // For simplicity, define some constraints
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Tile& cell = grid[y][x];
                if (!cell.collapsed) continue;

                TileType t = cell.type;
                if (x > 0) applyNeighborConstraints(grid[y][x - 1], t); // Left neighbor
                if (x < width - 1) applyNeighborConstraints(grid[y][x + 1], t); // Right neighbor
                if (y > 0) applyNeighborConstraints(grid[y - 1][x], t); // Top neighbor
                if (y < height - 1) applyNeighborConstraints(grid[y + 1][x], t); // Bottom neighbor
            }
        }
    }

    void applyNeighborConstraints(Tile& neighbor, TileType current) {
        if (neighbor.collapsed) return;

        // Example constraints
        switch (current) {
        case TALL_BUILDING:
            neighbor.possibleStates.erase(std::remove(neighbor.possibleStates.begin(), neighbor.possibleStates.end(), EMPTY), neighbor.possibleStates.end());
            neighbor.possibleStates.erase(std::remove(neighbor.possibleStates.begin(), neighbor.possibleStates.end(), SMALL_BUILDING), neighbor.possibleStates.end());
            break;
        case EMPTY:
            break;
        case SMALL_BUILDING:
            neighbor.possibleStates.erase(std::remove(neighbor.possibleStates.begin(), neighbor.possibleStates.end(), TALL_BUILDING), neighbor.possibleStates.end());
            break;
        default:
            break;
        }
    }
};
