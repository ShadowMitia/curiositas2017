#include "ofMain.h"
#include "ofApp.h"
#include "Constant.h"
//========================================================================
int main( ){
    ofSetupOpenGL(win_width,win_height,OF_WINDOW);

    // this kicks off the running of my app
  // can be OF_WINDOW or OF_FULLSCREEN
  // pass in width and height too:
  ofRunApp(new ofApp());

}
