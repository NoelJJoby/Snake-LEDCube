#include <stdio.h>
#include <stdint.h>

#define MAX_SNAKE_LENGTH 512

typedef struct
{
    int x;
    int y;
    int z;
} Vector3;

int reset_grid(void);
int print_grid(void);
int draw_snake_to_grid(void);
int set_led_in_grid(Vector3 pos);
int randomize_food_loc();

struct Snake
{
    int len;
     Vector3 parts[MAX_SNAKE_LENGTH];
};

struct Food
{
    int ate;
     Vector3 pos;
};

uint8_t grid[8][8];
struct Snake snake;
struct Food food;

int reset_grid(void)
{
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            grid[x][y] = 0;
        }
    }
}

int print_grid(void)
{
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            printf("%d ", grid[y][x]);
        }
        printf("\n");
    }
    printf("\n\n");
}

// Requires Send grid to Cube func

int set_led_in_grid(Vector3 pos)
{
    grid[pos.y][pos.x] += (1 << pos.z);
}

int draw_snake_to_grid(void)
{
    for (int i = 0; i < snake.len; i++)
    {
        set_led_in_grid(snake.parts[i]);
    }
}

int move_snake(int x, int y, int z)
{
    for (int i = snake.len - 1; i > 0; i--)
    {
        snake.parts[i] = snake.parts[i - 1];
    }
    snake.parts[0].x += x;
    snake.parts[0].y += y;
    snake.parts[0].z -= z;
}

int randomize_food_loc()
{
    srand(time(NULL));
    food.pos.x = rand() % 8;
    food.pos.y = rand() % 8;
    food.pos.z = rand() % 8;
}

int main(void)
// {
//     snake.len = 3;
//     snake.parts[0].x=3;
//     snake.parts[0].y=3;
//     snake.parts[0].z=0;

//     snake.parts[1].x=3;
//     snake.parts[1].y=4;
//     snake.parts[1].z=0;

//     snake.parts[2].x=3;
//     snake.parts[2].y=4;
//     snake.parts[2].z=1;

//     reset_grid();
//     draw_snake_to_grid();
//     print_grid();

//     move_snake(0,0,-1);
//     reset_grid();
//     draw_snake_to_grid();
//     print_grid();

//     move_snake(0,0,-1);
//     reset_grid();
//     draw_snake_to_grid();
//     print_grid();

//     move_snake(1,0,0);
//     reset_grid();
//     draw_snake_to_grid();
//     print_grid();

{
    randomize_food_loc();
    printf("%d %d %d", food.pos.x, food.pos.y, food.pos.z);
}