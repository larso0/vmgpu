#ifndef VMGPU_CAMERACONTROLLER_H
#define VMGPU_CAMERACONTROLLER_H

#include <bpScene/Node.h>

class CameraController
{
public:
	CameraController() :
		node{nullptr},
		sensitivity{0.01f},
		speed{2.f},
		yaw{0.f}, pitch{0.f},
		forwardPressed{false}, backwardPressed{false}, upPressed{false}, downPressed{false},
		leftPressed{false}, rightPressed{false} {}

	void setCameraNode(bpScene::Node& node) { CameraController::node = &node; }
	void update(float delta);
	void motion(float yaw, float pitch);
	bool isForwardPressed() const { return forwardPressed; }
	void setForwardPressed(bool fw) { forwardPressed = fw; }
	bool isBackwardPressed() const { return backwardPressed; }
	void setBackwardPressed(bool bw) { backwardPressed = bw; }
	bool isUpPressed() const { return upPressed; }
	void setUpPressed(bool up) { upPressed = up; }
	bool isDownPressed() const { return downPressed; }
	void setDownPressed(bool down) { downPressed = down; }
	bool isLeftPressed() const { return leftPressed; }
	void setLeftPressed(bool left) { leftPressed = left; }
	bool isRightPressed() const { return rightPressed; }
	void setRightPressed(bool right) { rightPressed = right; }

private:
	bpScene::Node* node;
	float sensitivity, speed;
	float yaw, pitch;

	bool forwardPressed, backwardPressed, upPressed, downPressed, leftPressed, rightPressed;
};

#endif
