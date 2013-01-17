#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "CircleFinder.h"
#include "ofxAutoControlPanel.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	CircleFinder circleFinder;
	ofxKinect kinect;
	ofMesh cloud;
	ofEasyCam easyCam;
	
	ofxAutoControlPanel gui;
	
	ofImage valid, background;
	vector<ofVec3f> trackedPositions;
};

