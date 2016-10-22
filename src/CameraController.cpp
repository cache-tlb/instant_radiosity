#include "CameraController.h"
#include "Vec4.h"
#include "common.h"
#include <QDebug>

CameraController::CameraController()
  : eye_distance_(1),
    eye_pos_(0, 0, 0), look_at_(0, 0, -1), eye_up_(0, 1, 0),
    rotate_speed_(1.0), zoom_speed_(1.2), pan_speed_(0.3),
    is_static_moving_(false), dynamic_damping_factor_(0.3),
    no_rotate_(false), no_roll_(false), no_zoom_(false), no_pan_(false),
    min_distance_(0), max_distance_(1e10),
    last_position_(0, 0, 0),
    state_(NONE), prev_state_(NONE),
    rotate_start_(0, 0, 0), rotate_end_(0, 0, 0),
    zoom_start_(0, 0, 0), zoom_end_(0, 0, 0),
    pan_start_(0, 0, 0), pan_end_(0, 0, 0)
{

}

CameraController::~CameraController()
{
}

void CameraController::HandleKeyPressEvent(const KeyEventArg &arg) {
    switch (arg.key_code) {
    case 'S':
    case 's':
        ToggleStaticMoving();
        break;
    default:
        break;
    }
}

void CameraController::HandleKeyReleaseEvent(const KeyEventArg &arg) {
    //
}

void CameraController::HandleMousePressEvent(const MouseEventArg &arg) {
    if (arg.mouse_type != MouseEventArg::PRESS) return;
    if (state_ == NONE) {
        switch (arg.mouse_button) {
        case MouseEventArg::LEFT:
            state_ = ROTATE; break;
        case MouseEventArg::MIDDLE:
            state_ = ZOOM; break;
        case MouseEventArg::RIGHT:
            state_ = PAN; break;
        default:
            break;
        }
    } else {
        return;
    }
    switch (state_) {
    case CameraController::ROTATE:
        rotate_start_ = GetMouseProjectionOnBall(arg.position.x, arg.position.y);
        rotate_end_ = rotate_start_;
        break;
    case CameraController::ZOOM:
        zoom_start_ = GetMouseOnScreen(arg.position.x, arg.position.y);
        zoom_end_ = zoom_start_;
        break;
    case CameraController::PAN:
        pan_start_ = GetMouseOnScreen(arg.position.x, arg.position.y);
        pan_end_ = pan_start_;
        break;
    default:
        break;
    }
}

void CameraController::HandleMouseMoveEvent(const MouseEventArg &arg) {
    switch (state_) {
    case CameraController::ROTATE:
        rotate_end_ = GetMouseProjectionOnBall(arg.position.x, arg.position.y);
        break;
    case CameraController::ZOOM:
        zoom_end_ = GetMouseOnScreen(arg.position.x, arg.position.y);
        break;
    case CameraController::PAN:
        pan_end_ = GetMouseOnScreen(arg.position.x, arg.position.y);
        break;
    default:
        break;
    }
    Update(0);
}

void CameraController::HandleMouseReleaseEvent(const MouseEventArg &arg) {
    state_ = NONE;
}

void CameraController::HandleWheelEvent(const MouseEventArg &arg) {
    if (state_ == NONE) {
        float delta = arg.degree / 10.f;
        zoom_start_.y += delta;
        Update(0);
    }
}

void CameraController::CheckDistances(Vec3f eye_dir) {
    if (!no_zoom_ || !no_pan_) {
        if (eye_distance_ > max_distance_) {
            eye_distance_ = max_distance_;
            eye_dir = eye_dir.unit().multiply(max_distance_);
            eye_pos_ = look_at_.add(eye_dir);
        }
        if (eye_distance_< min_distance_) {
            eye_distance_ = min_distance_;
            eye_dir = eye_dir.unit().multiply(min_distance_);
            eye_pos_ = look_at_.add(eye_dir);
        }
    }
}

void CameraController::Update(double delta) {
    if (!no_rotate_ && ((is_static_moving_ && state_ == ROTATE) || !is_static_moving_)) {
        RotateCamera();
    }
    if (!no_zoom_ && ((is_static_moving_ && state_ == ZOOM) || !is_static_moving_)) {
        ZoomCamera();
    }
    if (!no_pan_ && ((is_static_moving_ && state_ == PAN) || !is_static_moving_)) {
        PanCamera();
    }
    Vec3f eye_dir = eye_pos_.subtract(look_at_);
    CheckDistances(eye_dir);
}


Vec3f CameraController::GetMouseOnScreen(double clientX, double clientY) {
    return Vec3f(clientX / screen_width_, clientY / screen_height_, 0);
}

inline static Vec3f applyQuaternion(const Vec3f &v, const Vec4f &q) {
    float x = v.x, y = v.y, z = v.z;
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;

    float ix = qw*x + qy*z - qz*y;
    float iy = qw*y + qz*x - qx*z;
    float iz = qw*z + qx*y - qy*x;
    float iw = -qx*x - qy*y - qz*z;

    float rx = ix*qw + iw*(-qx) + iy*(-qz) - iz*(-qy);
    float ry = iy*qw + iw*(-qy) + iz*(-qx) - ix*(-qz);
    float rz = iz*qw + iw*(-qz) + ix*(-qy) - iy*(-qx);

    return Vec3f(rx, ry, rz);
}


Vec3f CameraController::GetMouseProjectionOnBall(double clientX, double clientY) {
    Vec3f mouseOnBall = Vec3f(
        (clientX - screen_width_/2) / (screen_width_ / 2),
        -(clientY - screen_height_/2)/(screen_height_ / 2),
        0);
    float length = mouseOnBall.length();
    if (no_roll_) {
        if (length*length < 0.5) {
            mouseOnBall.z = sqrt(1 - length * length);
        } else {
            mouseOnBall.z = .5 / length;
        }
    } else if (length >= 1) {
        mouseOnBall = mouseOnBall.unit();
    } else {
        mouseOnBall.z = sqrt(1 - length * length);
    }

    Vec3f eye_dir = eye_pos_.subtract(look_at_);
    Vec3f projection = eye_up_.unit().multiply(mouseOnBall.y);
    projection = projection.add(eye_up_.cross(eye_dir).unit().multiply(mouseOnBall.x));
    projection = projection.add(eye_dir.unit().multiply(mouseOnBall.z));
    return projection;
}

void CameraController::RotateCamera() {
    Vec3f eye_dir = eye_pos_.subtract(look_at_);
    float l1 = rotate_start_.length(), l2 = rotate_end_.length();
    if (l1*l2 == 0) return;
    float angle  = acos(clamp<float>(rotate_start_.dot(rotate_end_) / (l1*l2), -1, 1));
    if (fabs(angle) > 1e-6) {
        Vec3f axis = rotate_start_.cross(rotate_end_).unit().multiply(rotate_speed_);
        Vec4f quat = Vec4f::fromAxisAngle(axis.x, axis.y, axis.z, -angle);
        eye_dir = applyQuaternion(eye_dir, quat);
        eye_pos_ = look_at_.add(eye_dir.unit().multiply(eye_distance_));
        eye_up_ = applyQuaternion(eye_up_, quat);
        rotate_end_ = applyQuaternion(rotate_end_, quat);
        if (is_static_moving_) {
            rotate_start_ = rotate_end_;
        } else {
            quat = Vec4f::fromAxisAngle(axis.x, axis.y, axis.z, angle*(1-dynamic_damping_factor_));
            rotate_start_ = applyQuaternion(rotate_start_, quat);
        }
    }
}

void CameraController::ZoomCamera() {
    float factor = 1 + (zoom_end_.y - zoom_start_.y)*zoom_speed_;
    if (factor != 1 && factor > 0) {
        Vec3f eye_dir = eye_pos_.subtract(look_at_);
        eye_distance_ *= factor;
        eye_pos_ = look_at_.add(eye_dir.unit().multiply(eye_distance_));
        if (is_static_moving_) zoom_start_ = zoom_end_;
        else zoom_start_.y += (zoom_end_.y - zoom_start_.y)*dynamic_damping_factor_;
    }
}

void CameraController::PanCamera() {
    Vec3f eye_dir = eye_pos_.subtract(look_at_);
    Vec3f mouse_change = pan_end_.subtract(pan_start_);
    if (mouse_change.length() > 0) {
        mouse_change = mouse_change.multiply(eye_distance_*pan_speed_);
        Vec3f pan = eye_dir.cross(eye_up_).unit().multiply(mouse_change.x);
        pan = pan.add(eye_up_.unit().multiply(mouse_change.y));
        eye_pos_ = eye_pos_.add(pan);
        look_at_ = look_at_.add(pan);
        if (is_static_moving_) {
            pan_start_ = pan_end_;
        } else {
            pan_start_ = pan_start_.add(pan_end_.subtract(pan_start_).multiply(dynamic_damping_factor_));
        }
    }
}
