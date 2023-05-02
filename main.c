/**
 * @defgroup   MAIN main
 *
 * @brief      This file implements main. Please Comment you code
 *
 * @author     Khush Bakht Aliza - 21K-4714
 * @date       2023
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


// Function to print the grid
void printGeneration(int *grid, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", grid[i * n + j]);
        }
        printf("\n");
    }
}

// Function to update the generation for a single row
void updateRow(int *grid, int *newGrid, int row, int n) {
    int neighbors;
    for (int col = 0; col < n; col++) {
        neighbors = 0;
        // Count number of alive neighbors
        for (int r = row - 1; r <= row + 1; r++) {
            for (int c = col - 1; c <= col + 1; c++) {
                if (r >= 0 && r < n && c >= 0 && c < n && !(r == row && c == col) && grid[r * n + c] == 1) {
                    neighbors++;
                }
            }
        }

        // Update cell state based on neighbors
        if (grid[row * n + col] == 1) {
            if (neighbors == 2 || neighbors == 3) {
                newGrid[row * n + col] = 1;
            } else {
                newGrid[row * n + col] = 0;
            }
        } else {
            if (neighbors == 3) {
                newGrid[row * n + col] = 1;
            } else {
                newGrid[row * n + col] = 0;
            }
        }
    }
}

int main() {
    int n;
    printf("Enter the size of the grid (n x n): ");
    scanf("%d", &n);

    // Allocate shared memory for the grid
    int *grid = (int *)malloc(n * n * sizeof(int));
    int *newGrid = (int *)malloc(n * n * sizeof(int));

    printf("Enter the initial state of the grid (0 for dead, 1 for alive):\n");
    for (int i = 0; i < n * n; i++) {
        scanf("%d", &grid[i]);
    }

    int generations;
    printf("Enter the number of generations to simulate: ");
    scanf("%d", &generations);

    // Create pipes for communication
    int **pipes = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        pipes[i] = (int *)malloc(2 * sizeof(int));
        pipe(pipes[i]);
    }

    // Fork for each row and update generations concurrently
    for (int gen = 1; gen <= generations; gen++) {
        for (int row = 0; row < n; row++) {
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                close(pipes[row][0]); // Close read end of pipe

                // Update generation for the row
                updateRow(grid, newGrid, row, n);

                // Write updated row to pipe
                write(pipes[row][1], newGrid + row * n, n * sizeof(int));

                exit(0);
            } else if (pid== -1) {
            // Error in forking
                printf("Error forking process.\n");
                exit(1);
            }
    }    // Wait for all child processes to complete
        for (int row = 0; row < n; row++) {
            wait(NULL);
        }

        // Read updated grid from pipes
        for (int row = 0; row < n; row++) {
            read(pipes[row][0], newGrid + row * n, n * sizeof(int));
        }

        // Copy new grid to original grid for next generation
        memcpy(grid, newGrid, n * n * sizeof(int));
    }

    // Print the final generation
    printf("Final generation:\n");
    printGeneration(grid, n);

    // Free allocated memory
    free(grid);
    free(newGrid);
    for (int i = 0; i < n; i++) {
       free(pipes[i]);
    }
    free(pipes);

    return 0;
}
