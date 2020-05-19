#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
#define SPEED_FAST_DEFAULT		16.0f
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

    /* Constructors */
    // Direct Customization on Yaw & Pitch Values
    Camera(	// with vectors
            glm::vec3 _pos = CAM_POS_DEFAULT,
            glm::vec3 _worldUp = WORLD_UP_DEFAULT,
            float _yaw = YAW_DEFAULT,
            float _pitch = PITCH_DEFAULT
    ) {
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
            float _pitch = PITCH_DEFAULT
    ) : Camera(
            glm::vec3(_posX, _posY, _posZ),
            glm::normalize(glm::vec3(_worldUpX, _worldUpY, _worldUpZ)),
            _yaw,
            _pitch
    ) {}
    // Initial Focus On Certain Target
    Camera(
            glm::vec3 _pos = CAM_POS_DEFAULT,
            glm::vec3 _worldUp = WORLD_UP_DEFAULT,
            glm::vec3 _target = TARGET_POS_DEFAULT
    ) {
        position = _pos;
        worldUp = glm::normalize(_worldUp);
        locateTarget(_target);
        updateCameraVectors();	// yaw & pitch have changed, so update all vectors
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
    }
    void locateTarget(float _targetX, float _targetY, float _targetZ) {
        glm::vec3 _target(_targetX, _targetY, _targetZ);
        locateTarget(_target);
    }

    /* Input Feedback */
    // Keyboard Event
    void moveAround(CameraMovement dir, float deltaTime) {
        float velocity = speed * deltaTime;
        if (dir == CameraMovement::FORWARD) {
//            position += velocity * front;
            position += velocity * glm::cross(worldUp, right);
        }
        if (dir == CameraMovement::LEFT) {
            position -= velocity * right;
        }
        if (dir == CameraMovement::BACKWARD) {
//            position -= velocity * front;
            position -= velocity * glm::cross(worldUp, right);
        }
        if (dir == CameraMovement::RIGHT) {
            position += velocity * right;
        }
        if (dir == CameraMovement::UP) {
            position += velocity * worldUp;	// flying up, not straight above
        }
        if (dir == CameraMovement::DOWN) {
            position -= velocity * worldUp; // flying down, not straight below
        }
        // yaw/pitch has not changed, so no need to update
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

