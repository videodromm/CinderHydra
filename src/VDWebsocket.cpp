#include "VDWebsocket.h"

using namespace videodromm;

VDWebsocket::VDWebsocket() {

	shaderReceived = false;
	receivedFragString = "";
	shaderUniforms = false;
	receivedUniformsString = "";
	streamReceived = false;
	// WebSockets
	clientConnected = false;

	wsConnect();
	mPingTime = getElapsedSeconds();

}

void VDWebsocket::updateParams(int iarg0, float farg1) {

	if (iarg0 > 0 && iarg0 < 9) {
		// sliders 
		//mVDAnimation->setFloatUniformValueByIndex(iarg0, farg1);
	}
	if (iarg0 > 10 && iarg0 < 19) {
		// rotary 
		//mVDAnimation->setFloatUniformValueByIndex(iarg0, farg1);
		//		wsWrite("{\"params\" :[{\"name\":" + toString(iarg0) + ",\"value\":" + toString(mVDAnimation->getFloatUniformValueByIndex(iarg0)) + "}]}");

	}

	if (iarg0 > 40 && iarg0 < 49) {
		// low row 
		//mVDAnimation->setFloatUniformValueByIndex(iarg0, farg1);
	}
}

void VDWebsocket::wsPing() {
#if defined( CINDER_MSW )
	if (clientConnected) {

		mClient.ping();

	}
#endif
}
string * VDWebsocket::getBase64Image() {
	streamReceived = false;
	return &mBase64String;
}
//dreads(1, 0, 0.8).out(o0)
void VDWebsocket::parseMessage(string msg) {

	if (!msg.empty()) {

		string first = msg.substr(0, 1);
		if (first == "{") {
			// json
			JsonTree json;
			try {
				json = JsonTree(msg);
				// web controller
				if (json.hasChild("params")) {
					JsonTree jsonParams = json.getChild("params");
					for (JsonTree::ConstIter jsonElement = jsonParams.begin(); jsonElement != jsonParams.end(); ++jsonElement) {
						int name = jsonElement->getChild("name").getValue<int>();
						float value = jsonElement->getChild("value").getValue<float>();
						// basic name value 
						//mVDAnimation->setFloatUniformValueByIndex(name, value);

					}
				}

				if (json.hasChild("k2")) {
					JsonTree jsonParams = json.getChild("k2");
					for (JsonTree::ConstIter jsonElement = jsonParams.begin(); jsonElement != jsonParams.end(); ++jsonElement) {
						int name = jsonElement->getChild("name").getValue<int>();
						string value = jsonElement->getChild("value").getValue();
						vector<string> vs = split(value, ",");
						vec4 v = vec4(strtof((vs[0]).c_str(), 0), strtof((vs[1]).c_str(), 0), strtof((vs[2]).c_str(), 0), strtof((vs[3]).c_str(), 0));
						// basic name value 
						//mVDAnimation->setVec4UniformValueByIndex(name, v);
					}
				}

				if (json.hasChild("event")) {
					JsonTree jsonEvent = json.getChild("event");
					string val = jsonEvent.getValue();
					// check if message exists
					if (json.hasChild("message")) {
						if (val == "canvas") {
							// we received a jpeg base64
							mBase64String = json.getChild("message").getValue<string>();
							streamReceived = true;
						}
						else if (val == "params") {
							//{"event":"params","message":"{\"params\" :[{\"name\" : 12,\"value\" :0.132}]}"}
							JsonTree jsonParams = json.getChild("message");
							for (JsonTree::ConstIter jsonElement = jsonParams.begin(); jsonElement != jsonParams.end(); ++jsonElement) {
								int name = jsonElement->getChild("name").getValue<int>();
								float value = jsonElement->getChild("value").getValue<float>();

								// basic name value 
								//mVDAnimation->setFloatUniformValueByIndex(name, value);
							}
						}
						else if (val == "hydra") {
							receivedUniformsString = json.getChild("message").getValue<string>();
							shaderUniforms = true;
							// force to display
							//mVDAnimation->setIntUniformValueByIndex(mVDSettings->IFBOA, 0);
							//mVDAnimation->setIntUniformValueByIndex(mVDSettings->IFBOB, 1);
						}
						/*else if (val == "editortext") {
							// we received a fragment shader string from hydra
							receivedFragString = json.getChild("message").getValue<string>();
							shaderReceived = true;
							// force to display
							mVDAnimation->setIntUniformValueByIndex(mVDSettings->IFBOA, 0);
							mVDAnimation->setIntUniformValueByIndex(mVDSettings->IFBOB, 1);
						}*/
						else if (val == "frag") {
							// we received a fragment shader string
							receivedFragString = json.getChild("message").getValue<string>();
							shaderReceived = true;
							// force to display
							//mVDAnimation->setIntUniformValueByIndex(mVDSettings->IFBOA, 0);
							//mVDAnimation->setIntUniformValueByIndex(mVDSettings->IFBOB, 1);
						}
						else {
							// we received a fragment shader string
							//CI_LOG_V("VDWebsocket unknown event: " + val);
						}
						//string evt = json.getChild("event").getValue<string>();
					}
				}

			}
			catch (cinder::JsonTree::Exception exception) {
				/*mVDSettings->mWebSocketsMsg += " error jsonparser exception: ";
				mVDSettings->mWebSocketsMsg += exception.what();
				mVDSettings->mWebSocketsMsg += "  ";*/
			}
		}
		else if (msg.substr(0, 2) == "/*") {
			// shader with json info				
			unsigned closingCommentPosition = msg.find("*/");
			if (closingCommentPosition > 0) {
				JsonTree json;
				try {
					// create folders if they don't exist
					fs::path pathsToCheck = getAssetPath("") / "glsl";
					if (!fs::exists(pathsToCheck)) fs::create_directory(pathsToCheck);
					pathsToCheck = getAssetPath("") / "glsl" / "received";
					if (!fs::exists(pathsToCheck)) fs::create_directory(pathsToCheck);
					pathsToCheck = getAssetPath("") / "glsl" / "processed";
					if (!fs::exists(pathsToCheck)) fs::create_directory(pathsToCheck);
					// find commented header
					string jsonHeader = msg.substr(2, closingCommentPosition - 2);
					ci::JsonTree::ParseOptions parseOptions;
					parseOptions.ignoreErrors(false);
					json = JsonTree(jsonHeader, parseOptions);
					string title = json.getChild("title").getValue<string>();
					string fragFileName = title + ".frag"; // with uniforms
					string glslFileName = title + ".glsl"; // without uniforms, need to include shadertoy.inc
					string shader = msg.substr(closingCommentPosition + 2);

					string processedContent = "/*" + jsonHeader + "*/";
					// check uniforms presence
					std::size_t foundUniform = msg.find("uniform");

					if (foundUniform != std::string::npos) {
						// found uniforms
					}
					else {
						// save glsl file without uniforms as it was received
						fs::path currentFile = getAssetPath("") / "glsl" / "received" / glslFileName;
						/*ofstream mFrag(currentFile.string(), std::ofstream::binary);
						mFrag << msg;
						mFrag.close();*/


						// uniforms not found, add include
						processedContent += "#include shadertoy.inc";
					}
					processedContent += shader;

					// save processed file
					/*fs::path processedFile = getAssetPath("") / "glsl" / "processed" / fragFileName;
					ofstream mFragProcessed(processedFile.string(), std::ofstream::binary);
					mFragProcessed << processedContent;
					mFragProcessed.close();
					CI_LOG_V("processed file saved:" + processedFile.string());*/

				}
				catch (cinder::JsonTree::Exception exception) {
					/*mVDSettings->mWebSocketsMsg += " error jsonparser exception: ";
					mVDSettings->mWebSocketsMsg += exception.what();
					mVDSettings->mWebSocketsMsg += "  ";*/
				}
			}
		}

		else if (msg.substr(0, 7) == "#version") {
			// fragment shader from live coding
			// route it to websockets clients
			//if (mVDSettings->mIsRouter) {
			wsWrite(msg);
			//}
		}
		else if (first == "/")
		{
			// osc from videodromm-nodejs-router
			/*int f = msg.size();
			const char c = msg[9];
			int s = msg[12];
			int t = msg[13];
			int u = msg[14];*/

		}
		else if (first == "I") {

			if (msg == "ImInit") {
				// send ImInit OK
				/*if (!remoteClientActive) { remoteClientActive = true; ForceKeyFrame = true;
				// Send confirmation mServer.write("ImInit"); // Send font texture unsigned char* pixels; int width, height;
				ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height); PreparePacketTexFont(pixels, width, height);SendPacket();
				}*/
			}
			else if (msg.substr(0, 11) == "ImMouseMove") {
				/*string trail = msg.substr(12);
				unsigned commaPosition = trail.find(",");
				if (commaPosition > 0) { mouseX = atoi(trail.substr(0, commaPosition).c_str());
				mouseY = atoi(trail.substr(commaPosition + 1).c_str()); ImGuiIO& io = ImGui::GetIO();
				io.MousePos = toPixels(vec2(mouseX, mouseY)); }*/
			}
			else if (msg.substr(0, 12) == "ImMousePress") {
				/*ImGuiIO& io = ImGui::GetIO(); // 1,0 left click 1,1 right click
				io.MouseDown[0] = false; io.MouseDown[1] = false; int rightClick = atoi(msg.substr(15).c_str());
				if (rightClick == 1) { io.MouseDown[0] = false; io.MouseDown[1] = true; }
				else { io.MouseDown[0] = true; io.MouseDown[1] = false;
				}*/
			}
		}
	}
}
string VDWebsocket::getReceivedShader() {
	shaderReceived = false;
	return receivedFragString;
}
string VDWebsocket::getReceivedUniforms() {
	shaderUniforms = false;
	return receivedUniformsString;
}

void VDWebsocket::wsConnect() {


	mClient.connectOpenEventHandler([&]() {
		clientConnected = true;
		//mVDSettings->mWebSocketsMsg = "Connected";
		//mVDSettings->mWebSocketsNewMsg = true;
	});
	mClient.connectCloseEventHandler([&]() {
		clientConnected = false;
		//mVDSettings->mWebSocketsMsg = "Disconnected";
		//mVDSettings->mWebSocketsNewMsg = true;
	});
	mClient.connectFailEventHandler([&](string err) {
		//mVDSettings->mWebSocketsMsg = "WS Error";
		//mVDSettings->mWebSocketsNewMsg = true;
		if (!err.empty()) {
			//mVDSettings->mWebSocketsMsg += ": " + err;
		}
	});
	mClient.connectInterruptEventHandler([&]() {
		//mVDSettings->mWebSocketsMsg = "WS Interrupted";
		//mVDSettings->mWebSocketsNewMsg = true;
	});
	mClient.connectPingEventHandler([&](string msg) {
		//mVDSettings->mWebSocketsMsg = "WS Ponged";
		//mVDSettings->mWebSocketsNewMsg = true;
		if (!msg.empty())
		{
			//mVDSettings->mWebSocketsMsg += ": " + msg;
		}
	});
	mClient.connectMessageEventHandler([&](string msg) {
		parseMessage(msg);
	});
	wsClientConnect();


	clientConnected = true;

}

void VDWebsocket::wsClientConnect()
{

	stringstream s;
	/*if (mVDSettings->mWebSocketsPort == 80) {
		s << mVDSettings->mWebSocketsProtocol << mVDSettings->mWebSocketsHost;
	}
	else {
		s << mVDSettings->mWebSocketsProtocol << mVDSettings->mWebSocketsHost << ":" << mVDSettings->mWebSocketsPort;
	}*/
	// BL TEMP 
	s << "ws://127.0.0.1:8088";
	mClient.connect(s.str());

}
void VDWebsocket::wsClientDisconnect()
{

	if (clientConnected)
	{
		mClient.disconnect();
	}

}
void VDWebsocket::wsWrite(string msg)
{

	if (clientConnected) mClient.write(msg);

}

void VDWebsocket::sendJSON(string params) {

	wsWrite(params);

}
void VDWebsocket::toggleAuto(unsigned int aIndex) {
	// toggle
	//mVDAnimation->toggleAuto(aIndex);
	// TODO send json	
}
void VDWebsocket::toggleTempo(unsigned int aIndex) {
	// toggle
	//mVDAnimation->toggleTempo(aIndex);
	// TODO send json	
}
void VDWebsocket::toggleValue(unsigned int aIndex) {
	// toggle
	//mVDAnimation->toggleValue(aIndex);
	/*stringstream sParams;
	// TODO check boolean value:
	sParams << "{\"params\" :[{\"name\" : " << aIndex << ",\"value\" : " << (int)mVDAnimation->getBoolUniformValueByIndex(aIndex) << "}]}";
	string strParams = sParams.str();
	sendJSON(strParams);*/
}
void VDWebsocket::resetAutoAnimation(unsigned int aIndex) {
	// reset
	//mVDAnimation->resetAutoAnimation(aIndex);
	// TODO: send json	
}

void VDWebsocket::changeBoolValue(unsigned int aControl, bool aValue) {
	// check if changed
	//mVDAnimation->setBoolUniformValueByIndex(aControl, aValue);
	//stringstream sParams;
	//// TODO: check boolean value:
	//sParams << "{\"params\" :[{\"name\" : " << aControl << ",\"value\" : " << (int)aValue << "}]}";
	//string strParams = sParams.str();
	//sendJSON(strParams);
}
void VDWebsocket::changeIntValue(unsigned int aControl, int aValue) {
	//mVDAnimation->setIntUniformValueByIndex(aControl, aValue);
}
void VDWebsocket::changeFloatValue(unsigned int aControl, float aValue, bool forceSend, bool toggle, bool increase, bool decrease) {
	/*if (aControl == 31) {
		CI_LOG_V("old value " + toString(mVDAnimation->getFloatUniformValueByIndex(aControl)) + " newvalue " + toString(aValue));
	}*/
	/*float newValue;
	if (increase) {
		// increase
		newValue = mVDAnimation->getFloatUniformValueByIndex(aControl) + 0.1f;
		if (newValue > 1.0f) newValue = 1.0f;
		aValue = newValue;
	}
	else {
		// decrease
		if (decrease) {
			newValue = mVDAnimation->getFloatUniformValueByIndex(aControl) - 0.1f;
			if (newValue < 0.0f) newValue = 0.0f;
			aValue = newValue;
		}
		else {
			// toggle
			if (toggle) {
				newValue = mVDAnimation->getFloatUniformValueByIndex(aControl);
				if (newValue > 0.0f) { newValue = 0.0f; }
				else { newValue = 1.0f; } // Check for max instead?
				aValue = newValue;
			}
		}
	}
	// check if changed
	if ((mVDAnimation->setFloatUniformValueByIndex(aControl, aValue) && aControl != mVDSettings->IFPS) || forceSend) {
		stringstream sParams;
		// update color vec3
		if (aControl > 0 && aControl < 4) {
			mVDAnimation->setVec3UniformValueByIndex(61, vec3(mVDAnimation->getFloatUniformValueByIndex(1), mVDAnimation->getFloatUniformValueByIndex(2), mVDAnimation->getFloatUniformValueByIndex(3)));
			colorWrite(); //lights4events
		}
		if (aControl == 29 || aControl == 30) {
			mVDAnimation->setVec3UniformValueByIndex(60, vec3(mVDAnimation->getFloatUniformValueByIndex(29), mVDAnimation->getFloatUniformValueByIndex(30), 1.0));
		}
		sParams << "{\"params\" :[{\"name\" : " << aControl << ",\"value\" : " << mVDAnimation->getFloatUniformValueByIndex(aControl) << "}]}";
		string strParams = sParams.str();

		sendJSON(strParams);
	}*/
}
void VDWebsocket::changeShaderIndex(unsigned int aWarpIndex, unsigned int aWarpShaderIndex, unsigned int aSlot) {
	//aSlot 0 = A, 1 = B,...
	stringstream sParams;
	sParams << "{\"cmd\" :[{\"type\" : 1,\"warp\" : " << aWarpIndex << ",\"shader\" : " << aWarpShaderIndex << ",\"slot\" : " << aSlot << "}]}";
	string strParams = sParams.str();
	sendJSON(strParams);
}
void VDWebsocket::changeWarpFboIndex(unsigned int aWarpIndex, unsigned int aWarpFboIndex, unsigned int aSlot) {
	//aSlot 0 = A, 1 = B,...
	stringstream sParams;
	sParams << "{\"cmd\" :[{\"type\" : 0,\"warp\" : " << aWarpIndex << ",\"fbo\" : " << aWarpFboIndex << ",\"slot\" : " << aSlot << "}]}";
	string strParams = sParams.str();
	sendJSON(strParams);
}
void VDWebsocket::changeFragmentShader(string aFragmentShaderText) {

	stringstream sParams;
	sParams << "{\"event\" : \"frag\",\"message\" : \"" << aFragmentShaderText << "\"}";
	string strParams = sParams.str();
	sendJSON(strParams);
}
void VDWebsocket::colorWrite()
{
	/* remove apache untick proxy mode in nginx
	location /ws {
	proxy_pass http://domain;
	proxy_http_version 1.1;
	proxy_set_header Upgrade $http_upgrade;
	proxy_set_header Connection "Upgrade";
	}
	*/
	// lights4events
	/*char col[97];
	int r = (int)(mVDAnimation->getFloatUniformValueByIndex(1) * 255);
	int g = (int)(mVDAnimation->getFloatUniformValueByIndex(2) * 255);
	int b = (int)(mVDAnimation->getFloatUniformValueByIndex(3) * 255);
	int a = (int)(mVDAnimation->getFloatUniformValueByIndex(4) * 255);
	//sprintf(col, "#%02X%02X%02X", r, g, b);
	sprintf(col, "{\"type\":\"action\", \"parameters\":{\"name\":\"FC\",\"parameters\":{\"color\":\"#%02X%02X%02X%02X\",\"fading\":\"NONE\"}}}", a, r, g, b);
	wsWrite(col);*/
	//mL4EClient.write(col);
}

void VDWebsocket::update() {
	if (clientConnected)
	{
		mClient.poll();
	}

}
