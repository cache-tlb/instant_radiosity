#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H


#include "Vec3.h"

struct MouseEventArg {
    enum MouseEventButton {
        LEFT = 0,
        MIDDLE,
        RIGHT
    };
    enum MouseEventType {
        PRESS = 0,
        MOVE,
        RELEASE,
        WHEEL
    };
    MouseEventButton mouse_button;
    MouseEventType mouse_type;
    Vec3f position;
    int degree;
};

struct KeyEventArg {
    enum KeyEventType {
        PRESS = 0,
        RELEASE
    };
    KeyEventType key_type;
    int key_code;
};

class CameraController {
public:
    CameraController();
    ~CameraController();

    inline void SetScreenParameter(int width, int height);
    inline void ToggleStaticMoving();

    void HandleWheelEvent (const MouseEventArg &arg);
    void HandleMousePressEvent(const MouseEventArg &arg);
    void HandleMouseMoveEvent(const MouseEventArg &arg);
    void HandleMouseReleaseEvent(const MouseEventArg &arg);

    void HandleKeyPressEvent(const KeyEventArg &arg);
    void HandleKeyReleaseEvent(const KeyEventArg &arg);

    void Update(double delta);

    Vec3f eye_pos_, look_at_, eye_up_;

    double eye_distance_;
    double rotate_speed_, zoom_speed_, pan_speed_;
    bool is_static_moving_;
    double dynamic_damping_factor_;
    bool no_roll_, no_zoom_, no_rotate_, no_pan_;
    double min_distance_, max_distance_;
    int screen_width_, screen_height_;
protected:
    enum State {
        NONE = 0,
        ROTATE,
        ZOOM,
        PAN
    };
    State state_, prev_state_;
    Vec3f last_position_;
    Vec3f rotate_start_, rotate_end_;
    Vec3f zoom_start_, zoom_end_;
    Vec3f pan_start_, pan_end_;
    void RotateCamera();
    void ZoomCamera();
    void PanCamera();
    void CheckDistances(Vec3f eye_dir);
    Vec3f GetMouseOnScreen(double clientX, double clientY);
    Vec3f GetMouseProjectionOnBall(double clientX, double clientY);
private:
};

void CameraController::SetScreenParameter(int width, int height) {
    screen_width_ = width;
    screen_height_ = height;
}

void CameraController::ToggleStaticMoving() {
    is_static_moving_ = !is_static_moving_;
}

#endif // CAMERACONTROLLER_H
