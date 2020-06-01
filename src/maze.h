#ifndef HIM_MAZE_H
#define HIM_MAZE_H

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Thing {
    int xPos;
    int yPos;
};

// 0 represent road, 1 represent wall
// first, initialize the class
// and then, call get_maze can get the maze matrix
// the row and column only can be odd and >= 3
class Maze {
private:
    int **maze_map;
    int row;
    int col;

    Thing thingOne;
    Thing thingTwo;
    Thing thingThree;

    void make_maze();

    int searchPath(int, int);

public:
    glm::vec2 start;
    glm::vec2 end;


    Maze(int, int);

    int **get_maze() const;

    void print_maze() const;

    int get_row_num() const;

    int get_col_num() const;

    bool isWall(int i, int j) const;

    glm::vec3 Maze::getStartPoint(double);

    glm::vec3 Maze::getEndPoint(double);

    Thing Maze::getThingOne();

    Thing Maze::getThingTwo();

    Thing Maze::getThingThree();
};

#endif //HIM_MAZE_H
