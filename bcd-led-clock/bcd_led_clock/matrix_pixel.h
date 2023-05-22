#ifndef MATRIX_PIXEL_H
#define MATRIX_PIXEL_H

/**
 * Class expects the matrix is wired row-wise in a serpentine fasion. 
 * Class to represent a pixel position, either defined by
 * (led position (index)) or (row, column). Once defined, you can use either 
 * the (position) or (row, column) interchangably.
 * Can optionally can hold a Color.
 */
class MatrixPixel {
    public:
    int row = -1;
    int column = -1;
    int position = -1;
    bool onMatrix = false;
    Color color;

    /**
     * Find the column for a position.
     */
    static int rowForPosition(int position) {
        return (int) floor(position / NUM_COLUMNS);
    }

    /**
     * Find the column for a position.
     * We must know the row it's on to
     * compensate for the serpentine LED path.
     */
    static int columnForPosition(int row, int position) {
        int column = position % NUM_COLUMNS;
        if (row % 2 == 1) {
            // Compoensate for serpentine matrix
            column = (NUM_COLUMNS - 1) - column;
        }
        return column;
    }

    /**
     * Find the position for a known row and column.
     * Compensate for the serpentine LED path.
     */
    static int positionForRowAndColumn(int row, int column) {
        int actualColumn = (row % 2 == 1) ? (NUM_COLUMNS - 1 - column) : column;
        return NUM_COLUMNS * row + actualColumn;
    }

    /**
     * Constructor.
     * Create a MatrixPixel based on a known led position (linear).
     * The row and column will be calculated.
     */
    MatrixPixel(int position_) {     // Constructor
        position = position_;
        onMatrix = (position >= 0) && (position < (NUM_ROWS * NUM_COLUMNS));
        if (onMatrix) {
            row = rowForPosition(position);
            column = columnForPosition(row, position);
            // ESP_LOGD("MatrixPixel for position", "p=%d -> (r=%d, c=%d)", position, row, column);
        }
    }

    MatrixPixel(esphome::light::AddressableLight &lights, int position_) {     // Constructor
        position = position_;
        onMatrix = (position >= 0) && (position < (NUM_ROWS * NUM_COLUMNS));
        if (onMatrix) {
            row = rowForPosition(position);
            column = columnForPosition(row, position);
            color = lights[position].get();
            // ESP_LOGD("MatrixPixel for position", "p=%d -> (r=%d, c=%d)", position, row, column);
        }
    }

    /**
     * Constructor.
     * Create a MatrixPixel based on a known row and column.
     * The (linear) position will be calculated.
     */
    MatrixPixel(int row_, int column_) {     // Constructor
        row = row_;
        column = column_;
        onMatrix = (row >= 0 && row < NUM_ROWS && column >= 0 && column < NUM_COLUMNS);
        if (onMatrix) {
            position = positionForRowAndColumn(row, column);
            // ESP_LOGD("MatrixPixel for (r,c)", "(r=%d, c=%d) -> p=%d", row, column, position);
        }
    }

    MatrixPixel(esphome::light::AddressableLight &lights, int row_, int column_) {     // Constructor
        row = row_;
        column = column_;
        onMatrix = (row >= 0 && row < NUM_ROWS && column >= 0 && column < NUM_COLUMNS);
        if (onMatrix) {
            position = positionForRowAndColumn(row, column);
            color = lights[position].get();
            // ESP_LOGD("MatrixPixel for (r,c)", "(r=%d, c=%d) -> p=%d", row, column, position);
        }
    }
};

#endif
