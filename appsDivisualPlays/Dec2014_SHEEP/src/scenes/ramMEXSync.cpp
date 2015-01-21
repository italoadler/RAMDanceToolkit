//
//  ramMEXSync.cpp
//  RAMDanceToolkit
//
//  Created by Ovis aries on 2015/01/19.
//
//

#include "ramMEXSync.h"

ramMEXSync::ramMEXSync(){

}

void ramMEXSync::setupControlPanel(){

	gui = ramGetGUI().getCurrentUIContext();
	targScene = gui->addTextInput("TargScene", "dpHPLink_Laser");
	targScene->setAutoClear(false);

	scenes.push_back("dpHServoPendulum");	scenes_pair.push_back("dpVisServoPendulum");
	scenes.push_back("dpVisStage");			scenes_pair.push_back("");
	scenes.push_back("dpHWorm");			scenes_pair.push_back("dpVisWorm");
	scenes.push_back("dpHfrozenIce");		scenes_pair.push_back("dpVisIce");
	scenes.push_back("dpHStruggle");		scenes_pair.push_back("dpVisStruggle");
	scenes.push_back("dpHSandStorm");		scenes_pair.push_back("dpVisSandStorm");
	scenes.push_back("dpHMagPendulum");		scenes_pair.push_back("dpVisMagPendulum");
 	scenes.push_back("dpVisTheta");			scenes_pair.push_back("");
	scenes.push_back("dpHGearMove");		scenes_pair.push_back("");
	scenes.push_back("dpHTornado");			scenes_pair.push_back("dpVisTornado");
	scenes.push_back("dpHPLink_Laser");		scenes_pair.push_back("dpVisPLink_Laser");
	scenes.push_back("dpHPLink_Prism");		scenes_pair.push_back("dpVisPLink_Prism");
	scenes.push_back("dpHPLink_Oil");		scenes_pair.push_back("dpVisPLink_Oil");
	
	gui->addLabel("SceneSelect",OFX_UI_FONT_LARGE);
	gui->addRadio("SceneSelector", scenes);
	gui->addSpacer();
	gui->addLabel("SceneControl",OFX_UI_FONT_LARGE);
	gui->addToggle("Enable", &mSignal_Enable);
	gui->addToggle("Disp1", &mSignal_DispA);
	gui->addToggle("Disp2", &mSignal_DispB);
	gui->addLabel("SendSignal",OFX_UI_FONT_LARGE);
	gui->addButton("Send", false);
	gui->addButton("AllClear", false);
	
	gui->addSpacer();
	gui->addLabel("Extractor",OFX_UI_FONT_LARGE);
	gui->addButton("Sync", false);
	gui->addButton("Get", false);
	gui->addSpacer();
	gui->addLabel("Remote UI",OFX_UI_FONT_LARGE);
	uiName = gui->addTextInput("uiName", "UIName");
	uiName->setAutoClear(false);
	gui->addSlider("Remote:Float", 0.0, 1.0, &mUIRemote_float);
	gui->addSlider("Order", 1.0, 1000.0, &mUIRemote_fOrder);
	gui->addToggle("Remote:Toggle", &mUIRemote_toggle);
	
	mex.setupControlPanel(this);

	ofAddListener(gui->newGUIEvent, this, &ramMEXSync::onPanelChanged);
	
	receiver.addAddress("/Debug/");
	ramOscManager::instance().addReceiverTag(&receiver);
	
}

void ramMEXSync::update(){

	mSignal_DispA = mSignal_DispA & mSignal_Enable;
	mSignal_DispB = mSignal_DispB & mSignal_Enable;
	
	while (receiver.hasWaitingMessages()){
		
		ofxOscMessage m;
		receiver.getNextMessage(&m);
		
		int idx = -1;
		for (int i = 0;i < previews.size();i++){
			if (previews[i]->msg.getAddress() == m.getAddress()){
				idx = i;
				break;
			}
		}
		
		if (idx == -1){

			oscPreview* pv = new oscPreview();
			pv->msg = m;
			pv->valueClamp.assign(pv->msg.getNumArgs(), 0.0);
			pv->numValue.assign(pv->msg.getNumArgs(), 0.0);

			for (int i = 0;i < m.getNumArgs();i++){
				if (m.getArgType(i) == OFXOSC_TYPE_FLOAT)
					pv->numValue[i] = m.getArgAsFloat(i);
				else if (m.getArgType(i) == OFXOSC_TYPE_INT32)
					pv->numValue[i] = m.getArgAsInt32(i);
				else if (m.getArgType(i) == OFXOSC_TYPE_INT64)
					pv->numValue[i] = m.getArgAsInt64(i);
			}
			
			previews.push_back(pv);
			
			
		}else{
			
			for (int i = 0;i < previews[idx]->msg.getNumArgs();i++){
				float v;
				if (m.getArgType(i) == OFXOSC_TYPE_FLOAT) v = m.getArgAsFloat(i);
				else if (m.getArgType(i) == OFXOSC_TYPE_INT32) v = m.getArgAsInt32(i);
				else if (m.getArgType(i) == OFXOSC_TYPE_INT64) v = m.getArgAsInt64(i);
				else{
					v = 0.0;
				}
				previews[idx]->numValue[i] = v;
				previews[idx]->valueClamp[i] = MAX(previews[idx]->valueClamp[i],v);
			}
			
			previews[idx]->msg = m;
			
		}
		
		while (previews.size() > 10){
			oscPreview* p = previews[0];
			previews.erase(previews.begin());
			delete p;
		}
	}
	
	mex.update();
}

void ramMEXSync::draw(){

	ramBeginCamera();
	mex.draw();
	ramEndCamera();
	
	for (int i = 0;i < previews.size();i++){
		ofPushMatrix();
		ofTranslate(700, i*130);
		ofNoFill();
		ofRect(0, 10, 300, 115);
		ofFill();
		
		string info = previews[i]->msg.getAddress() + "\n";
		info += previews[i]->msg.getRemoteIp() + "\n";
		ofSetColor(130,30,80);
		for (int j = 0;j < previews[i]->msg.getNumArgs();j++){
			if (previews[i]->msg.getArgType(j) == OFXOSC_TYPE_STRING)
				info += previews[i]->msg.getArgAsString(j) + "\n";
			else
				info += ofToString(previews[i]->numValue[j]) + "\n";
			ofRect(0, 30+j*13, 300 * previews[i]->numValue[j] / previews[i]->valueClamp[j], 8);
		}
		
		ofSetColor(255);
		ofDrawBitmapString(info, 10, 25);
		
		ofPopMatrix();
	}

}

void ramMEXSync::onPanelChanged(ofxUIEventArgs &e){
	ofxUIWidget* w = e.widget;

	if (w->getName() == "AllClear"){
		for (int i = 0;i < scenes.size();i++){
			ofxOscMessage scn,pair;
			scn.setAddress("/ram/set_scene");
			scn.addStringArg(scenes[i]);
			scn.addIntArg(0);
			scn.addIntArg(0);
			scn.addIntArg(0);

			pair.setAddress("/ram/set_scene");
			pair.addStringArg(scenes_pair[i]);
			pair.addIntArg(0);
			pair.addIntArg(0);
			pair.addIntArg(0);

			for (int j = 0;j < 2;j++){
				sender.setup(j == 0 ? ip_1 : ip_2, 10000);
				sender.sendMessage(scn);
				sender.sendMessage(pair);
			}
		}
	}

	if (w->getName() == "Remote:Float"){
		for (int i = 0;i < 2;i++){
			if (i == 0) sender.setup(ip_1, 10000);
			if (i == 1) sender.setup(ip_2, 10000);
			ofxOscMessage msg;
			msg.setAddress("/ram/MEX/"+targScene->getTextString()+"/UI");
			msg.addStringArg(uiName->getTextString());
			msg.addFloatArg(mUIRemote_float * mUIRemote_fOrder);
			
			sender.sendMessage(msg);
		}
	}
	
	if (w->getName() == "Remote:Toggle"){
		for (int i = 0;i < 2;i++){
			if (i == 0) sender.setup(ip_1, 10000);
			if (i == 1) sender.setup(ip_2, 10000);
			ofxOscMessage msg;
			msg.setAddress("/ram/MEX/"+targScene->getTextString()+"/UI");
			msg.addStringArg(uiName->getTextString());
			msg.addFloatArg(((ofxUIToggle*)(w))->getValue());
			
			sender.sendMessage(msg);
		}
	}

	if (w->getName() == "Send" && w->getState() == OFX_UI_STATE_DOWN){
		for (int i = 0;i < 2;i++){
			if (i == 0) sender.setup(ip_1, 10000);
			if (i == 1) sender.setup(ip_2, 10000);
			ofxOscMessage setScene;
			setScene.setAddress("/ram/set_scene");
			setScene.addStringArg(targScene->getTextString());
			setScene.addIntArg(mSignal_Enable);
			setScene.addIntArg(mSignal_DispA);
			setScene.addIntArg(mSignal_DispB);
			
			sender.sendMessage(setScene);

			int pId = -1;
			for (int j = 0;j < scenes.size();j++){
				if (targScene->getTextString() == scenes[j]){
					pId = j;
					break;
				}
			}

			if (pId != -1){
				ofxOscMessage pairScene;
				pairScene.setAddress("/ram/set_scene");
				pairScene.addStringArg(scenes_pair[pId]);
				pairScene.addIntArg(mSignal_Enable);
				pairScene.addIntArg(mSignal_DispA);
				pairScene.addIntArg(mSignal_DispB);

				sender.sendMessage(pairScene);
			}
		}
	}
	
	if (w->getName() == "SceneSelector"){
		ofxUIRadio* r = (ofxUIRadio*)(w);
		targScene->setTextString(r->getActiveName());
	}
	
	if (w->getName() == "Sync" && w->getState() == OFX_UI_STATE_DOWN){
		string addr = "/ram/MEX/"+targScene->getTextString();

		for (int i = 0;i < 2;i++){
			if (i == 0) sender.setup(ip_1, 10000);
			if (i == 1) sender.setup(ip_2, 10000);

			ofxOscMessage mCls;
			mCls.setAddress(addr+"/clear");

			sender.sendMessage(mCls);

			for (int i = 0;i < mex.mMotionPort.size();i++){
				ofxOscMessage mPsh;
				mPsh.setAddress(addr+"/push");
				mPsh.addIntArg(mex.mMotionPort[i]->mActorIndex);
				mPsh.addIntArg(mex.mMotionPort[i]->mFinder.index);
				sender.sendMessage(mPsh);
			}

			ofxOscMessage mLs;
			mLs.setAddress(addr+"/actorList");
			for (int i = 0;i < mex.actorList->getListItems().size();i++){
				mLs.addStringArg(mex.actorList->getListItems()[i]->getName());
			}
			sender.sendMessage(mLs);
		}

	}

	if (w->getName() == "Get" && w->getState() == OFX_UI_STATE_DOWN){

		for (int i = 0;i < 2;i++){

			if (i == 0) sender.setup(ip_1, 10000);
			if (i == 1) sender.setup(ip_2, 10000);

			ofxOscMessage req;
			req.setAddress("/ram/MEX/"+targScene->getTextString()+"/request");
			req.addStringArg(getName());

			sender.sendMessage(req);

		}

	}
}