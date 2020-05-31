#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

#include "maze.h"

/* Utility Function Macros */
#define MAX(a, b) ( ((a) > (b)) ? (a) : (b) )
#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )

/* Possible Options for Camera Movement */
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

/* Default Camera Values */
// World Vector Settings
#define WORLD_UP_DEFAULT	glm::vec3(0.0f, 1.0f, 0.0f)
// If There is a Target the Camera is Pointing to Initially
#define TARGET_POS_DEFAULT	glm::vec3(0.0f, 0.0f, -1.0f)
// Camera Vector Settings: At the origin, looking front,
// right is +x, up is the normal worldUp
#define CAM_POS_DEFAULT		glm::vec3(0.0f, 0.0f, 0.0f)
#define CAM_FRONT_DEFAULT	glm::vec3(0.0f, 0.0f, -1.0f)
#define CAM_RIGHT_DEFAULT	glm::vec3(1.0f, 0.0f, 0.0f)
#define CAM_UP_DEFAULT		glm::vec3(0.0f, 1.0f, 0.0f)
// Pitch: Looking straight forward, so value is 0.0f
#define PITCH_DEFAULT		0.0f
#define PITCH_MIN_DEFAULT	-89.0f
#define PITCH_MAX_DEFAULT	89.0f
// Yaw: Looking front, which is -90.0f degrees from +x
#define YAW_DEFAULT -90.0f
// Speed
#define SPEED_SLOW_DEFAULT		0.5f
#define SPEED_NORMAL_DEFAULT	3.6f
#define SPEED_FAST_DEFAULT		8.0f
// FOV (Field of View) / ZOOM
#define FOV_MIN_DEFAULT 1.0f
#define FOV_MAX_DEFAULT 45.0f
// Sensitivity
#define SENSITIVITY_DEFAULT 0.1f
// Clipping Plane
#define Z_NEAR_DEFAULT	0.1f
#define Z_FAR_DEFAULT	100.0f

/* Camera Class of OPENGL */
class Camera {
public:
    /* Camera Attributes */
    glm::vec3 position = CAM_POS_DEFAULT;	// Camera Position (Normalized)
    glm::vec3 front = CAM_FRONT_DEFAULT;	// Camera Front (Normalized)
    glm::vec3 right = CAM_RIGHT_DEFAULT;	// Camera Right (Normalized) *Note: When roll is considered, this should split up as well
    glm::vec3 up = CAM_UP_DEFAULT;			// Camera Up (Normalized)
    /* Applied World Attributes */
    glm::vec3 worldUp = WORLD_UP_DEFAULT;	// World Up (Normalized)
    /* Euler Angles: We do not care about roll for now. */
    float yaw = YAW_DEFAULT;			// Yaw on x-axis
    float pitch = PITCH_DEFAULT;		// Pitch on y-axis
    /* Camera Options */
    float speed = SPEED_NORMAL_DEFAULT;			// Moving Speed
    float sensitivity = SENSITIVITY_DEFAULT;	// Moving Sensitivity
    float fov = FOV_MAX_DEFAULT;				// FOV (Field of View) / ZOOM
    float zNear = Z_NEAR_DEFAULT;	// Near Clipping Plane
    float zFar = Z_FAR_DEFAULT;		// Far Clipping Plane

    bool fpv = true;    // First-Person View

    /* Constructors */
    // Direct Customization on Yaw & Pitch Values
    Camera(	// with vectors
            glm::vec3 _pos = CAM_POS_DEFAULT,
            glm::vec3 _worldUp = WORLD_UP_DEFAULT,
            float _yaw = YAW_DEFAULT,
            float _pitch = PITCH_DEFAULT,
            bool _fpv = true
    ) {
        fpv = _fpv;
        position = _pos;
        worldUp = glm::normalize(_worldUp);
        yaw = _yaw;
        pitch = _pitch;
        updateCameraVectors();	// yaw & pitch have changed, so update all vectors
    }
    Camera(	// with scalars
            float _posX = CAM_POS_DEFAULT.x, float _posY = CAM_POS_DEFAULT.y, float _posZ = CAM_POS_DEFAULT.z,
            float _worldUpX = WORLD_UP_DEFAULT.x, float _worldUpY = WORLD_UP_DEFAULT.y, float _worldUpZ = WORLD_UP_DEFAULT.z,
            float _yaw = YAW_DEFAULT,
            float _pitch = PITCH_DEFAULT,
            bool _fpv = true
    ) : Camera(
            glm::vec3(_posX, _posY, _posZ),
            glm::normalize(glm::vec3(_worldUpX, _worldUpY, _worldUpZ)),
            _yaw,
            _pitch,
            _fpv
    ) {}
    // Initial Focus On Certain Target
    Camera(
            glm::vec3 _pos = CAM_POS_DEFAULT,
            glm::vec3 _worldUp = WORLD_UP_DEFAULT,
            glm::vec3 _target = TARGET_POS_DEFAULT,
            bool _fpv = true
    ) {
        fpv = _fpv;
        position = _pos;
        worldUp = glm::normalize(_worldUp);
        locateTarget(_target);
    }

    /* Utility Functions */
    // Retrieve View Transformation Matrix
    glm::mat4 getViewMatrix() {
        return glm::lookAt(
                position,			// Camera at "position", in World Space
                position + front,	// Looks at "position + front"
                up					// Head is "up"
        );
    }
    // Quickly Locate a Specified Target in situ
    void locateTarget(glm::vec3 _target) {
        glm::vec3 camFront_unit = glm::normalize(_target - position);
        pitch = glm::degrees( asin(camFront_unit.y) );
        yaw = glm::degrees( atan2(camFront_unit.z, camFront_unit.x) );
        updateCameraVectors();	// yaw & pitch have changed, so update all vectors
    }
    void locateTarget(float _targetX, float _targetY, float _targetZ) {
        glm::vec3 _target(_targetX, _targetY, _targetZ);
        locateTarget(_target);
    }

    /* Input Feedback */
    // Keyboard Event
    void moveAround(CameraMovement dir, float deltaTime, const Maze* maze, double maze_blk_sz) {
        float velocity = speed * deltaTime;
        if (dir == CameraMovement::UP) {
            if (!fpv) {
                position += velocity * worldUp;    // flying up, not straight above
            }
        }
        else if (dir == CameraMovement::DOWN) {
            if (!fpv) {
                position -= velocity * worldUp; // flying down, not straight below
            }
        }
        else {
            glm::vec3 newPos = position;
            if (dir == CameraMovement::FORWARD) {
                newPos = position + velocity * glm::cross(worldUp, right);
            }
            if (dir == CameraMovement::LEFT) {
                newPos = position - velocity * right;
            }
            if (dir == CameraMovement::BACKWARD) {
                newPos = position - velocity * glm::cross(worldUp, right);
            }
            if (dir == CameraMovement::RIGHT) {
                newPos = position + velocity * right;
            }
            glm::vec3 moveDir = glm::normalize(newPos - position);
            position = fpv ? getMovedPos(maze, maze_blk_sz, moveDir, newPos, velocity): newPos;
        }
    }

    // Compare good point distance with wished
    glm::vec3 getMovedPos(const Maze* maze, double maze_blk_sz, glm::vec3 dir, glm::vec3 wishPos, float velocity) {
        glm::vec3 goodP = collideIfAny(maze, maze_blk_sz, dir);
        if (goodP.y < 0) {
            // no collision
            return wishPos;
        }
        // Collision!
        double dist_collision = glm::distance(position, goodP);
        double dist_wish = glm::distance(position, wishPos);
//        printf("collision = %.3f, wish = %.3f\n", dist_collision, dist_wish);
        return (dist_collision < dist_wish) ? goodP : wishPos;
    }

    // Collision Detection: returns good point
    glm::vec3 collideIfAny(const Maze* maze, double maze_blk_sz, glm::vec3 dir) {
        bool collided = false;
        double tmin = std::numeric_limits<double>::max();
        int imin = 999, jmin = 999;
        // A ray
        glm::vec3 ray_orig(position.x, 0, position.z);
        glm::vec3 ray_dir(dir.x, 0, dir.z); // only look in 2D dimension, to make searching faster
        double goOut = -1.0;
        // Search all cubes
        for (int i = 0; i < maze->get_row_num(); ++i) {
            for (int j = 0; j < maze->get_col_num(); ++j) {
                if (!maze->isWall(i, j)) continue;
                // A cube
                double half_maze_blk_sz = maze_blk_sz / 2;
                glm::vec3 pmin(i * maze_blk_sz - half_maze_blk_sz, -half_maze_blk_sz, j * maze_blk_sz - half_maze_blk_sz);
                glm::vec3 pmax(i * maze_blk_sz + half_maze_blk_sz, half_maze_blk_sz, j * maze_blk_sz + half_maze_blk_sz);
                // If point is inside the cube, rollback
                if (!(ray_orig.x < pmin.x || ray_orig.x > pmax.x ||
                    ray_orig.y < pmin.y || ray_orig.y > pmax.y ||
                    ray_orig.z < pmin.z || ray_orig.z > pmax.z)) {
                    goOut = 1.0;
                }
                // Whether they collide? p + td = X --> t = (X - p) / d
                double ts[6];
                ts[0] = (pmin.x - ray_orig.x) / ray_dir.x;
                ts[1] = (pmax.x - ray_orig.x) / ray_dir.x;
                ts[2] = (pmin.y - ray_orig.y) / ray_dir.y;
                ts[3] = (pmax.y - ray_orig.y) / ray_dir.y;
                ts[4] = (pmin.z - ray_orig.z) / ray_dir.z;
                ts[5] = (pmax.z - ray_orig.z) / ray_dir.z;
                for (double tcur : ts) {
                    // check if t is valid
                    if (tcur < 0) continue;
                    glm::vec3 res = ray_orig + ray_dir * (float)tcur;
                    if (res.x < pmin.x || res.x > pmax.x ||
                            res.y < pmin.y || res.y > pmax.y ||
                            res.z < pmin.z || res.z > pmax.z) {
                        // invalid
                        continue;
                    }
                    // valid, find smallest
                    collided = true;
                    if (tcur < tmin) {
                        tmin = tcur;
                        imin = i;
                        jmin = j;
                    }
                }
            }
        }
        if (!collided) {
            return glm::vec3(0, -10, 0);    // cannot collide underground
        }
        glm::vec3 crossPt = ray_orig + ray_dir * (float)(tmin + goOut * 0.32f);
        crossPt.y = position.y;
//        printf("Pointing at (%d, %d)!\n", imin, jmin);
        return crossPt;
    }

    // Retrieve the maze block pointed at
    int* getPointAt(const Maze* maze, double maze_blk_sz) {
        bool collided = false;
        double tmin = std::numeric_limits<double>::max();
        int collPt[3] = {-999, -999, -999};
        // A ray
        glm::vec3 ray_orig = position;
        glm::vec3 ray_dir = front;
        // Search all cubes
        for (int i = 0; i < maze->get_row_num(); ++i) {
            for (int j = 0; j < maze->get_col_num(); ++j) {
                for (int _ = 0; _ < 5; ++_) {
                    if (!maze->isWall(i, j)) continue;
                    // A cube
                    double half_maze_blk_sz = maze_blk_sz / 2;
                    glm::vec3 pmin(i * maze_blk_sz - half_maze_blk_sz, maze_blk_sz * _ - half_maze_blk_sz, j * maze_blk_sz - half_maze_blk_sz);
                    glm::vec3 pmax(i * maze_blk_sz + half_maze_blk_sz, maze_blk_sz * _ + half_maze_blk_sz, j * maze_blk_sz + half_maze_blk_sz);
                    // Whether they collide? p + td = X --> t = (X - p) / d
                    double ts[6];
                    ts[0] = (pmin.x - ray_orig.x) / ray_dir.x;
                    ts[1] = (pmax.x - ray_orig.x) / ray_dir.x;
                    ts[2] = (pmin.y - ray_orig.y) / ray_dir.y;
                    ts[3] = (pmax.y - ray_orig.y) / ray_dir.y;
                    ts[4] = (pmin.z - ray_orig.z) / ray_dir.z;
                    ts[5] = (pmax.z - ray_orig.z) / ray_dir.z;
                    for (double tcur : ts) {
                        // check if t is valid
                        if (tcur < 0) continue;
                        glm::vec3 res = ray_orig + ray_dir * (float)tcur;
                        if (res.x < pmin.x || res.x > pmax.x ||
                            res.y < pmin.y || res.y > pmax.y ||
                            res.z < pmin.z || res.z > pmax.z) {
                            // invalid
                            continue;
                        }
                        // valid, find smallest
                        collided = true;
                        if (tcur < tmin) {
                            tmin = tcur;
                            collPt[0] = i;
                            collPt[1] = _;
                            collPt[2] = j;
                        }
                    }
                }
            }
        }
        if (!collided) {
            return new int[3]{0, -10, 0};    // no collision
        }
//        printf("Pointing at (%d, %d, %d)!\n", collPt[0], collPt[1], collPt[2]);
        return new int[3]{collPt[0], collPt[1], collPt[2]};
    }

    void changeSpeed(float _speed) {
        speed = _speed;
        // yaw/pitch has not changed, so no need to update
    }

    // Mouse Movement
    void lookAround(float deltaX, float deltaY, GLboolean constrainPitch = true) {
        deltaX *= sensitivity;
        deltaY *= sensitivity;

        yaw += deltaX;		// Look Right
        pitch += deltaY;	// Look Up

        // bound pitch if needed
        if (constrainPitch) {
            pitch = MIN(pitch, PITCH_MAX_DEFAULT);
            pitch = MAX(pitch, PITCH_MIN_DEFAULT);
        }

        // yaw & pitch have changed, so update all the vectors
        updateCameraVectors();
    }

    // Mouse Scrolling
    void zoom(float yoffset) {
        // currently we only consider y-axis scrolling
        // There is indeed x-axis scrolling out there though...
        if (fov >= FOV_MIN_DEFAULT && fov <= FOV_MAX_DEFAULT) {
            fov -= yoffset;	// Scroll down
        }
        fov = MAX(fov, FOV_MIN_DEFAULT);
        fov = MIN(fov, FOV_MAX_DEFAULT);
    }

private:
    /* Update Camera's Vectors */
    // Front Vector
    void updateFront() {
        // "front" is changed by mouse inputs (pitch & yaw)
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        front = glm::normalize(newFront);
    }

    // Right Vector
    void updateRight() {
        // "right" is changed as soon as "front" is changed
        // Note that roll is normally not considered, so we can use
        //	"worldUp" to compute "right"
        right = glm::normalize(glm::cross(front, worldUp));
    }

    // Up Vector
    void updateUp() {
        // "up" is changed as soon as "front" is changed
        // Note that roll is normally not considered, so we can use
        //	"worldUp" to compute "right", then use "right" to compute "up"
        up = glm::normalize(glm::cross(right, front));
    }

    // Above is why everytime yaw/pitch changes, we need to update everything!
    void updateCameraVectors() {
        updateFront();
        updateRight();
        updateUp();
    }
};

#endif // !CAMERA_H

