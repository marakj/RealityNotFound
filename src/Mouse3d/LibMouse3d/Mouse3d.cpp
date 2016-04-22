#include "Mouse3d/LibMouse3d/Mouse3d.h"

#include <QtGlobal>
#include <QDebug>

#include "QOSG/CoreWindow.h"

namespace LibMouse3d{

	Mouse3dDevice::Mouse3dDevice( QOSG::CoreWindow *window ) {
#if defined(Q_OS_WIN)

        this->mouse = new Mouse3DInput(window);
        QObject::connect(mouse, SIGNAL(Move3d(std::vector<float>&)), window, SLOT(OnMove(std::vector<float>&)));

#elif defined(Q_OS_LINUX)

        Mouse3DLinux(window);

#elif defined(Q_OS_MAC)
#endif

    }

    Mouse3dDevice::~Mouse3dDevice(){
    }

#if defined(Q_OS_WIN)
#elif defined(Q_OS_LINUX)
    /*!
		Called with the processed motion data when a 3D mouse event is received

		The default implementation emits a Move3d signal with the motion data
	*/
	void Mouse3dDevice::SendSignal(std::vector<float>& motionData) {
        //emit Mouse3dDevice::Move3d(motionData);
    }

	void Mouse3dDevice::Mouse3DLinux(QOSG::CoreWindow* window) {
        Display *display;
        Window xwindow = window->winId();
        std::vector<float> motionData;

        XEvent report;
        MagellanFloatEvent MagellanEvent;

        bool MagellanLoop = true;

        float normAxis;

        display = QX11Info::display();

        /************************* Create 3D Event Types ***************************/
        if ( !MagellanInit( display, xwindow ) ) {
            qDebug() << "No driver is running. Exit ... " << endl;
            exit(EXIT_FAILURE);
        }
        else {
            qDebug() << "Mouse3dDevice::Mouse3DLinux: MagellanInit() = Success!" << endl;
        }
		qDebug() << "Mouse3dDevice::Mouse3DLinux: winId() = " << xwindow << endl;

        /************************* Main Loop ***************************************/
        XSelectInput( display, xwindow, KeyPressMask | KeyReleaseMask );

        while( MagellanLoop ) {
            //if(XCheckWindowEvent(display, xwindow, KeyPressMask | KeyReleaseMask,  &report ))
            //if( ! window->x11Event( &report ) )
            if ( !x11EventFilter( & report) )
                //XNextEvent( display, &report );
                //if (XCheckMaskEvent(display, KeyPressMask | KeyReleaseMask, &report))
                //if (report.type)
                    //qDebug() << "EventType = " << report.type << endl;
                if ( report.type == ClientMessage ) {
                    if ( MagellanTranslateEvent( display, &report, &MagellanEvent, 1.0, 1.0 ) == MagellanInputMotionEvent) {
                        MagellanRemoveMotionEvents( display );

                        motionData.clear();
                        motionData.push_back( MagellanEvent.MagellanData[ MagellanX ] );
                        motionData.push_back( MagellanEvent.MagellanData[ MagellanY ] );
                        motionData.push_back( MagellanEvent.MagellanData[ MagellanZ ] * -1.0);
                        motionData.push_back( MagellanEvent.MagellanData[ MagellanA ] );
                        motionData.push_back( MagellanEvent.MagellanData[ MagellanC ] * -1.0);
                        motionData.push_back( MagellanEvent.MagellanData[ MagellanB ] * -1.0);




                        motionData[0] /= 100.0;
                        motionData[1] /= 100.0;
                        motionData[2] /= 100.0;

                        /*
                        normAxis = sqrt(motionData[3] * motionData[3] + motionData[4] * motionData[4] + motionData[5] * motionData[5]);
                        if (normAxis == 0.0){
                            motionData[3] = 0.0;
                            motionData[4] = 0.0;
                            motionData[5] = 1.0;
                        }
                        else{<<
                            motionData[3] /= normAxis;
                            motionData[4] /= normAxis;
                            motionData[5] /= normAxis;
                        }

                        motionData[3] /= 10.0;
                        motionData[4] /= 10.0;
                        motionData[5] /= 10.0;
                        */

                        motionData[3] /= 7000.0;
                        motionData[4] /= 7000.0;
                        motionData[5] /= 7000.0;

                        qDebug() <<  "Mouse3d x=" << motionData[0] <<
                                    "y=" << motionData[1] <<
                                    "z=" << motionData[2] <<
                                    "a=" << motionData[3] <<
                                    "b=" << motionData[4] <<
                                    "c=" << motionData[5] << endl;

                        //emit Mouse3dDevice::Move3d(motionData);
                        window->OnMove(motionData);
                        EventThread::msleep(50);
                    }
                }

        }
   }

#elif defined(Q_OS_MAC)
#endif
}
