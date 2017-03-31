#include <easylogging++.h>

#include "LeapLib/LeapGestureHandler.h"
#include "LeapLib/LeapActions.h"
#include "LeapLib/DirectionDetector.h"
#include "LeapLib/FingerPositionDetector.h"

Leap::LeapGestureHandler::LeapGestureHandler( LeapManager* leapManager )
{
    leapActions = new Leap::LeapActions( leapManager );
	this->leapManager = leapManager;
    LOG( INFO ) << "LeapGestureHandler() Constructor";
}

Leap::LeapGestureHandler::~LeapGestureHandler()
{
	if ( leapManager != NULL ) {
		delete( leapManager );
	}
    LOG( INFO ) << "~LeapGestureHandler() Destructor";
}

void Leap::LeapGestureHandler::handleGestures( Frame frame )
{

    HandList hands = frame.hands();
    Leap::DirectionDetector::Direction leftHandDirection ;
    Leap::DirectionDetector::Direction rightHandDirection ;
    Vector *leftHandVelocity = new Vector(0,0,0);
    Vector *rightHandVelocity = new Vector(0,0,0);
    Vector *deltaVelocity = new Vector(0,0,0) ;
    bool leftHandExtended = false;
    bool rightHandExtended = false;

    for ( int i=0; i< hands.count(); ++i ) {
        if ( hands[i].isRight() ) {
            rightHandDirection = Leap::DirectionDetector::getPalmDirection( hands[i] );
            *rightHandVelocity = hands[i].palmVelocity();

            rightHandExtended = Leap::FingerPositionDetector::isHandExtended( hands[i] );
        }
        else {
            leftHandDirection = Leap::DirectionDetector::getPalmDirection( hands[i] );
            *leftHandVelocity = hands[i].palmVelocity();
            //leapActions->scaleEdges( direction );

            leftHandExtended = Leap::FingerPositionDetector::isHandExtended( hands[i] );
        }
    }

    if(leftHandVelocity->x !=0 && rightHandVelocity->x != 0 ){
        if(!leftHandExtended && !rightHandExtended){
            deltaVelocity = new Vector(rightHandVelocity->x - leftHandVelocity->x,rightHandVelocity->y - leftHandVelocity->y,rightHandVelocity->z - leftHandVelocity->z);

            leapActions->scaleGraph( deltaVelocity );

        }
    }


    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];

        HandList hands = gesture.hands();
        Hand firstHand = hands[0];
        switch (gesture.type()) {
        case Gesture::TYPE_CIRCLE:
        {
            LOG( INFO ) << "GESTO CIRCLE....zoomGraph().";
            leapActions->zoomGraph(gesture);
            break;
        }
        case Gesture::TYPE_SWIPE:
        {

            LOG( INFO ) << "GESTO swipe ....rotateGraph().";
            leapActions->rotateGraph(gesture);
            break;
        }
        case Gesture::TYPE_KEY_TAP:
        {
            LOG( INFO ) << "GESTO KEY_TAP....onKeyTap().";
            leapActions->onKeyTap(gesture);
            break;
        }
        case Gesture::TYPE_SCREEN_TAP:
        {
            LOG( INFO ) << "GESTO SCREEN_TAP....onScreenTap().";
            leapActions->onScreenTap(gesture);
            break;
        }
        default:
            LOG( INFO ) << "Unknown gesture type.";
            break;
        }
    }
}
