#include <stdio.h>

#include <SDL2/SDL.h>

#define GRID_DIMENSION 27
#define CELL_SIZE 18
#define PADDING 1
#define EXTRA_WIDTH 250
#define WINDOW_HEIGHT (GRID_DIMENSION * CELL_SIZE) + PADDING
#define WINDOW_WIDTH  (GRID_DIMENSION * CELL_SIZE) + EXTRA_WIDTH + PADDING

#define BAR_WIDTH 15
#define BAR_PADDING 35
#define BAR_SEPERATION 5
#define MAX_BAR_HEIGHT 250
#define BASE_BAR_HEIGHT WINDOW_HEIGHT - 50

static int grid[GRID_DIMENSION][GRID_DIMENSION] = {0};

float relu_derivative(float z) {
    if (z > 0.0f) { return 1.0f; }
    return 0.0f;
}


void map_corresponding_rgb(float confidence, int rgb[]) {
    if (confidence <= 0.1) { rgb[0] = 229; rgb[1] = 31; rgb[2] = 31; }
    else if (confidence <= 0.2) { rgb[0] = 242; rgb[1] = 161; rgb[2] = 52; }
    else if (confidence <= 0.4) { rgb[0] = 247; rgb[1] = 227; rgb[2] = 121; }
    else if (confidence <= 0.6) { rgb[0] = 187; rgb[1] = 219; rgb[2] = 68; }
    else if (confidence <= 0.8) { rgb[0] = 68; rgb[1] = 206; rgb[2] = 27; }
    else { rgb[0] = 0; rgb[1] = 150; rgb[2] = 0;  }
}

void map_corresponding_digit(SDL_Renderer *r, int digit, int x, int y) {
    switch(digit) {
        case 0: SDL_RenderDrawRect(r, &(SDL_Rect){x, y, 6, 10}); break;
        case 1: SDL_RenderDrawLine(r, x+3, y, x+3, y+10); break;
        case 2: SDL_RenderDrawLine(r, x, y, x+6, y); SDL_RenderDrawLine(r, x+6, y, x+6, y+5); SDL_RenderDrawLine(r, x+6, y+5, x, y+5); SDL_RenderDrawLine(r, x, y+5, x, y+10); SDL_RenderDrawLine(r, x, y+10, x+6, y+10); break;
        case 3: SDL_RenderDrawLine(r, x, y, x+6, y); SDL_RenderDrawLine(r, x+6, y, x+6, y+10); SDL_RenderDrawLine(r, x, y+5, x+6, y+5); SDL_RenderDrawLine(r, x, y+10, x+6, y+10); break;
        case 4: SDL_RenderDrawLine(r, x, y, x, y+5); SDL_RenderDrawLine(r, x+5, y+5, x, y+5); SDL_RenderDrawLine(r, x+6, y, x+6, y+10); break;
        case 5: SDL_RenderDrawLine(r, x+6, y, x, y); SDL_RenderDrawLine(r, x, y, x, y+5); SDL_RenderDrawLine(r, x, y+5, x+6, y+5); SDL_RenderDrawLine(r, x+6, y+5, x+6, y+10); SDL_RenderDrawLine(r, x+6, y+10, x, y+10); break;
        case 6: SDL_RenderDrawRect(r, &(SDL_Rect){x, y+5, 6, 5}); SDL_RenderDrawLine(r, x, y, x+6, y); SDL_RenderDrawLine(r, x, y, x, y+5); break;
        case 7: SDL_RenderDrawLine(r, x, y, x+6, y); SDL_RenderDrawLine(r, x+6, y, x+2, y+10); break;
        case 8: SDL_RenderDrawRect(r, &(SDL_Rect){x, y, 6, 5}); SDL_RenderDrawRect(r, &(SDL_Rect){x, y+5, 6, 5}); break;
        case 9: SDL_RenderDrawLine(r, x, y, x+6, y); SDL_RenderDrawLine(r, x, y, x, y+5); SDL_RenderDrawLine(r, x, y+5, x+6, y+5); SDL_RenderDrawLine(r, x+6, y, x+6, y+10); break;
    }
}

int main() {

    SDL_Color gridBackground = {22, 22, 22, 255};
    SDL_Color gridLineColor = {44, 44, 44, 255};
    SDL_Color gridPlacedColor = {255, 255, 255, 255}; 

    if (SDL_Init(SDL_INIT_VIDEO) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialize SDL: %s", SDL_GetError()); return 1; }

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer) < 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Create window and renderer: %s", SDL_GetError()); return 1; }

    SDL_SetWindowTitle(window, "Digit Recogniser");

    SDL_bool quit = SDL_FALSE;
    SDL_bool mouseDrawing = SDL_FALSE;
    int drawMode = 1; 
    
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseDrawing = SDL_TRUE;
                    int gridX = event.button.x / CELL_SIZE;
                    int gridY = event.button.y / CELL_SIZE;
                    
                    if (gridX >= 0 && gridX < GRID_DIMENSION && gridY >= 0 && gridY < GRID_DIMENSION) {
                        drawMode = !grid[gridX][gridY];
                        
                        for (int x = 0; x < 2; x++) {
                            for (int y = 0; y < 2; y++) {
                                if (gridX + x < GRID_DIMENSION && gridY + y < GRID_DIMENSION) {
                                    grid[gridX + x][gridY + y] = drawMode;
                                }
                            }
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseDrawing = SDL_FALSE;
                }
                break;
            case SDL_MOUSEMOTION:
                if (mouseDrawing) {
                    int gridX = event.motion.x / CELL_SIZE;
                    int gridY = event.motion.y / CELL_SIZE;

                    for (int x = 0; x < 2; x++) {
                        for (int y = 0; y < 2; y++) {
                            if (gridX + x >= 0 && gridX + x < GRID_DIMENSION &&
                                gridY + y >= 0 && gridY + y < GRID_DIMENSION) {
                                grid[gridX + x][gridY + y] = drawMode;
                            }
                        }
                    }
                }
                break;
            case SDL_QUIT:
                quit = SDL_TRUE;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, gridBackground.r, gridBackground.g, gridBackground.b, gridBackground.a);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, gridLineColor.r, gridLineColor.g, gridLineColor.b, gridLineColor.a);
        for (int x = 0; x <= GRID_DIMENSION * CELL_SIZE; x += CELL_SIZE) {
            SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT - 1);
        }
        for (int y = 0; y <= GRID_DIMENSION * CELL_SIZE; y += CELL_SIZE) {
            SDL_RenderDrawLine(renderer, 0, y, GRID_DIMENSION * CELL_SIZE, y);
        }

        SDL_SetRenderDrawColor(renderer, gridPlacedColor.r, gridPlacedColor.g, gridPlacedColor.b, gridPlacedColor.a);
        for (int x = 0; x < GRID_DIMENSION; x++) {
            for (int y = 0; y < GRID_DIMENSION; y++) {
                if (grid[x][y] == 1) {
                    SDL_Rect cellRect = { (x * CELL_SIZE) + 1, (y * CELL_SIZE) + 1, CELL_SIZE - 1, CELL_SIZE - 1 };
                    SDL_RenderFillRect(renderer, &cellRect);
                }
            }
        }

        float confidence[10] = {0.0302f, 0.2494f, 0.7076f, 1.0000f, 0.7076f, 0.2494f, 0.0302f, 0.0014f, 0.0000f, 0.0000f};

        for (int i = 0; i < 10; i++){
            int rgb[3];
            map_corresponding_rgb(confidence[i], rgb);
            SDL_SetRenderDrawColor(renderer, rgb[0], rgb[1], rgb[2], gridPlacedColor.a);
 
            int barHeight = (int)(confidence[i] * MAX_BAR_HEIGHT);
            if (barHeight < 4) { barHeight = 4;}

            

            SDL_Rect barRect = {(GRID_DIMENSION * CELL_SIZE) + (i * (BAR_WIDTH + BAR_SEPERATION)) + BAR_PADDING,BASE_BAR_HEIGHT - barHeight,BAR_WIDTH,barHeight};
            SDL_RenderFillRect(renderer, &barRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
            int textX = (GRID_DIMENSION * CELL_SIZE) + (i * (BAR_WIDTH + BAR_SEPERATION)) + BAR_PADDING + (BAR_WIDTH / 2) - 3; 
            int textY = BASE_BAR_HEIGHT + 15;

            map_corresponding_digit(renderer, i, textX, textY);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}