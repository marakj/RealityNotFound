#include "QOpenCV/OpenCVWindow.h"

#include "OpenCV/CamSelectCore.h"

#include "Util/ApplicationConfig.h"

QOpenCV::OpenCVWindow::OpenCVWindow( QWidget* parent, QApplication* app ) : QDialog( parent )
{
	mApp = app;
	configureWindow();
}

QLabel* QOpenCV::OpenCVWindow::getLabel() const
{
	return mWindowLabel;
}

QCheckBox*	QOpenCV::OpenCVWindow::getMarkerBehindCB() const
{
	return mMarkerBehindCB;
}

QCheckBox*	QOpenCV::OpenCVWindow::getMultiMarkerEnableCB() const
{
	return mMultiMarkerEnableCB;
}

QCheckBox*	QOpenCV::OpenCVWindow::getCorEnabledCB() const
{
	return mCorEnabledCB;
}

QPushButton*	QOpenCV::OpenCVWindow::getUpdateCorParPB() const
{
	return mUpdateCorParPB;
}

QPushButton* QOpenCV::OpenCVWindow::getInterchangeMarkersPB() const
{
	return mInterchangeMarkersPB;
}

void QOpenCV::OpenCVWindow::configureWindow()
{
	setModal( false );
	setWindowTitle( tr( "Kinect and Aruco Window" ) );

	mWindowLabel = new QLabel( "", this, 0 );

	mKinectRB = new QRadioButton( tr( "Kinect" ) );
	mArucoRB = new QRadioButton( tr( "Aruco" ) );
    mArInteractionRB = new QRadioButton( tr( "AR Interaction" ) );
	mFaceRecRB = new QRadioButton( tr( "Face Recognition" ) );
	mMarkerRB = new QRadioButton( tr( "Marker" ) );
	mMultiMarkerRB = new QRadioButton( tr( "Multi Marker" ) );

	mFaceRecPB = new QPushButton( tr( "Start Face Recognition" ) );
	mMarkerPB = new QPushButton( tr( "Start Marker Detection" ) );
	mMultiMarkerPB = new QPushButton( tr( "Start MultiMarker" ) );
	mKinectPB = new QPushButton( tr( "Start Kinect" ) );
	mKinectSnapshotPB = new QPushButton( tr( "Kinect Snapshot" ) );
	mUpdateCorParPB	= new QPushButton( tr( "Update cor. param." ) );
	mInterchangeMarkersPB = new QPushButton( tr( "Change Markers" ) );

	mNoVideo = new QCheckBox( tr( "NoVideo" ) );
	mMarkerBackgrCB	= new QCheckBox( tr( "Background" ) );
	mFaceDetBackgrCB = new QCheckBox( tr( "Background" ) );
	mMarkerBehindCB	= new QCheckBox( tr( "Marker is behind" ) );
	mCorEnabledCB = new QCheckBox( tr( "Correction" ) );
	mMultiMarkerEnableCB = new QCheckBox( tr( "Multiple markers" ) );
	mEnableMarkerlessKinectCB = new QCheckBox( tr( "Turn on Markerless Detection" ) );

	mDisableCursorCB = new QCheckBox( tr( "Turn off cursor" ) );
	mDisableZoomCursorCB = new QCheckBox( tr( "Turn off zoom" ) );
	mEnableMarkerDetectCB = new QCheckBox( tr( "Turn on Marker Detection" ) );
	mEnableMarkerlessCameraCB = new QCheckBox( tr( "Turn on Markerless Detection" ) );

	mSpeed =  new QSlider( Qt::Vertical );
	mSpeed->setRange( 5,20 );
	mSpeed->setValue( 10 );
	mSpeed->setPageStep( 1 );
	mSpeed->setFocusPolicy( Qt::NoFocus );
	mSpeed->setToolTip( "Modify speed of movement" );

	mModulesStackL = new QStackedLayout;
	mSubmodulesStackL = new QStackedLayout;

	QHBoxLayout* mainLayout		= new QHBoxLayout;
	QVBoxLayout* buttonLayout	= new QVBoxLayout;

#ifdef OPENNI2_FOUND
	mKinectRB->setChecked( true );
	buttonLayout->addWidget( mKinectRB );
#endif

	buttonLayout->addWidget( mArucoRB );
    buttonLayout->addWidget( mArInteractionRB );
	buttonLayout->addLayout( mModulesStackL );

	QWidget* kinectPageWid =  new QWidget;
	QWidget* arucoPageWid =  new QWidget;
	QWidget* arucoFaceRecPageWid = new QWidget;
	QWidget* arucoMarkerPageWid = new QWidget;
	QWidget* arucoMultiMarkerPageWid = new QWidget;
	QWidget* arucoSubPageWid = new QWidget;
    QWidget* arInteractionSubPageWid = new QWidget;

	QVBoxLayout*	kinectPageLayout	= new QVBoxLayout;
	QVBoxLayout*	arucoPageLayout	= new QVBoxLayout;
	QVBoxLayout* arucoFaceRecPageLayout = new QVBoxLayout;
	QVBoxLayout* arucoMarkerPageLayout = new QVBoxLayout;
	QVBoxLayout* arucoMultiMarkerPageLayout = new QVBoxLayout;
	QVBoxLayout* arucoSubPageLayout = new QVBoxLayout;
    QVBoxLayout* arInteractionSubPageLayout = new QVBoxLayout;

	mFaceRecRB->setChecked( true );
	arucoSubPageLayout->addWidget( mFaceRecRB );
	arucoSubPageLayout->addWidget( mMarkerRB );
	//arucoSubPageLayout->addWidget( mMultiMarkerRB );
	arucoSubPageLayout->addWidget( mNoVideo );
	arucoSubPageLayout->addLayout( mSubmodulesStackL );

//JMA interaction RB groups
    //selection
    arNSPosition = new QRadioButton(tr("Select node by position"));
        arNSPosition->setChecked(true);
    arNSMostEdges = new QRadioButton(tr("Select node with most edges"));
    arNSNearest = new QRadioButton(tr("Select nearest node automatically"));

    QVBoxLayout *arNodeSelectionVBox = new QVBoxLayout;
        arNodeSelectionVBox->addWidget(arNSPosition);
        arNodeSelectionVBox->addWidget(arNSMostEdges);
        arNodeSelectionVBox->addWidget(arNSNearest);
        arNodeSelectionVBox->addStretch(1);

    QGroupBox *arNodeSelectionGroupBox = new QGroupBox(tr("Node selection method"));
        arNodeSelectionGroupBox->setLayout(arNodeSelectionVBox);

    arInteractionSubPageLayout->addWidget( arNodeSelectionGroupBox );

    connect( arNSPosition, SIGNAL( clicked( bool ) ), this, SLOT( onArInteractionSelectionClicked( bool ) ) );
    connect( arNSMostEdges, SIGNAL( clicked( bool ) ), this, SLOT( onArInteractionSelectionClicked( bool ) ) );
    connect( arNSNearest, SIGNAL( clicked( bool ) ), this, SLOT( onArInteractionSelectionClicked( bool ) ) );

    //behaviour
    arNBSingle = new QRadioButton(tr("Single node movement"));
        arNBSingle->setChecked(true);
    arNBCluster = new QRadioButton(tr("Cluster movement"));

    QVBoxLayout *arNodeBehaviourVBox = new QVBoxLayout;
        arNodeBehaviourVBox->addWidget(arNBSingle);
        arNodeBehaviourVBox->addWidget(arNBCluster);
        arNodeBehaviourVBox->addStretch(1);

    QGroupBox *arNodeBehaviourGroupBox = new QGroupBox(tr("Node behaviour method"));
        arNodeBehaviourGroupBox->setLayout(arNodeBehaviourVBox);

    arInteractionSubPageLayout->addWidget( arNodeBehaviourGroupBox );

    connect( arNBSingle, SIGNAL( clicked( bool ) ), this, SLOT( onArInteractionBehaviourClicked( bool ) ) );
    connect( arNBCluster, SIGNAL( clicked( bool ) ), this, SLOT( onArInteractionBehaviourClicked( bool ) ) );


    QPushButton *mArInteractionGraphZoomIn = new QPushButton( tr( "+" ) );
    QPushButton *mArInteractionGraphZoomOut = new QPushButton( tr( "-" ) );
    arInteractionSubPageLayout->addWidget( mArInteractionGraphZoomIn );
    arInteractionSubPageLayout->addWidget( mArInteractionGraphZoomOut );

    connect( mArInteractionGraphZoomIn,  SIGNAL( clicked( bool ) ), this, SLOT( applyGraphZoomIn( bool ) ) );
    connect( mArInteractionGraphZoomOut,  SIGNAL( clicked( bool ) ), this, SLOT( applyGraphZoomOut( bool ) ) );


//JMA interaction RB groups

	kinectPageLayout->setAlignment( Qt::AlignBottom );
	arucoPageLayout->setAlignment( Qt::AlignBottom );
	arucoFaceRecPageLayout->setAlignment( Qt::AlignBottom );
	arucoMarkerPageLayout->setAlignment( Qt::AlignBottom );
	arucoMultiMarkerPageLayout->setAlignment( Qt::AlignBottom );

	mModulesStackL->addWidget( kinectPageWid );
	mModulesStackL->addWidget( arucoSubPageWid );
    mModulesStackL->addWidget( arInteractionSubPageWid );


	mSubmodulesStackL->addWidget( arucoFaceRecPageWid );
	mSubmodulesStackL->addWidget( arucoMarkerPageWid );
	//mSubmodulesStackL->addWidget( arucoMultiMarkerPageWid );

	kinectPageWid->setLayout( kinectPageLayout );
	arucoPageWid->setLayout( arucoPageLayout );
	arucoFaceRecPageWid->setLayout( arucoFaceRecPageLayout );
	arucoMarkerPageWid->setLayout( arucoMarkerPageLayout );
	arucoMultiMarkerPageWid->setLayout( arucoMultiMarkerPageLayout );
	arucoSubPageWid->setLayout( arucoSubPageLayout );
    arInteractionSubPageWid->setLayout( arInteractionSubPageLayout );

	//set up page layouts
	kinectPageLayout->addWidget( mDisableCursorCB );
	kinectPageLayout->addWidget( mDisableZoomCursorCB );
	kinectPageLayout->addWidget( mEnableMarkerDetectCB );
	kinectPageLayout->addWidget( mEnableMarkerlessKinectCB );
	kinectPageLayout->addWidget( mSpeed );
	kinectPageLayout->addWidget( mKinectSnapshotPB );
	kinectPageLayout->addWidget( mKinectPB );

	//arucoPageLayout->addWidget( mMultiMarkerPB );

	arucoFaceRecPageLayout->addWidget( mFaceDetBackgrCB );
	arucoFaceRecPageLayout->addWidget( mFaceRecPB );

	arucoMarkerPageLayout->addWidget( mMarkerBackgrCB );
	arucoMarkerPageLayout->addWidget( mMarkerBehindCB );
	arucoMarkerPageLayout->addWidget( mCorEnabledCB );
	arucoMarkerPageLayout->addWidget( mMultiMarkerEnableCB );
	arucoMarkerPageLayout->addWidget( mEnableMarkerlessCameraCB );
	arucoMarkerPageLayout->addWidget( mUpdateCorParPB );
	arucoMarkerPageLayout->addWidget( mInterchangeMarkersPB );
	arucoMarkerPageLayout->addWidget( mMarkerPB );

	//arucoMultiMarkerPageLayout->addWidget( mMultiMarkerPB );

	if ( Util::ApplicationConfig::get()->getValue( "Viewer.SkyBox.Noise" ).toInt() < 2 ) {
		mMarkerBackgrCB->setDisabled( true );
	}

	//set layout
	mainLayout->addLayout( buttonLayout );
	mainLayout->addWidget( mWindowLabel, Qt::AlignCenter );
	mainLayout->setSizeConstraint( QLayout::SetMinimumSize );
	setLayout( mainLayout );
	adjustSize();

	//set up Widgets
	mUpdateCorParPB->setEnabled( false );

	mFaceDetBackgrCB->setEnabled( false );
	mMarkerBackgrCB->setEnabled( false );
	mMarkerBehindCB->setEnabled( false );
	mCorEnabledCB->setEnabled( false );
	mMultiMarkerEnableCB->setEnabled( true );
	mEnableMarkerlessKinectCB->setEnabled( true );
	mEnableMarkerDetectCB->setEnabled( true );
	mEnableMarkerlessCameraCB->setEnabled( true );

	mMultiMarkerPB->setCheckable( true );
	mFaceRecPB->setCheckable( true );
	mMarkerPB->setCheckable( true );
	mKinectPB->setCheckable( true );

	//set up signals to slots
	connect( mKinectRB, SIGNAL( clicked() ), this, SLOT( onSelModulChange() ) );
	connect( mArucoRB, SIGNAL( clicked() ), this, SLOT( onSelModulChange() ) );
    connect( mArInteractionRB, SIGNAL( clicked() ), this, SLOT( onSelModulChange() ) );

	connect( mFaceRecRB, SIGNAL( clicked() ), this, SLOT( onSelSubModulChange() ) );
	connect( mMarkerRB, SIGNAL( clicked() ), this, SLOT( onSelSubModulChange() ) );
	connect( mMultiMarkerRB, SIGNAL( clicked() ), this, SLOT( onSelSubModulChange() ) );

	connect( mNoVideo,	 SIGNAL( clicked() ), this, SLOT( onSelSubModulChange() ) );

	connect( mUpdateCorParPB, SIGNAL( clicked() ), this, SLOT( onUpdateCorPar() ) );
	connect( mMarkerPB,  SIGNAL( clicked( bool ) ), this, SLOT( onMarkerStartCancel( bool ) ) );
	connect( mFaceRecPB, SIGNAL( clicked( bool ) ), this, SLOT( onFaceRecStartCancel( bool ) ) );
	connect( mMultiMarkerPB, SIGNAL( clicked( bool ) ), this, SLOT( onMultiMarkerStartCancel( bool ) ) );
	connect( mKinectPB, SIGNAL( clicked( bool ) ), this, SLOT( onKinectStartCancel( bool ) ) );
	connect( mKinectSnapshotPB, SIGNAL( clicked() ), this, SLOT( onKinectSnapshotPBClicked() ) );

	connect( mMarkerBackgrCB, SIGNAL( clicked( bool ) ), this, SLOT( onMarkerBackgrCBClicked( bool ) ) );
	connect( mFaceDetBackgrCB, SIGNAL( clicked( bool ) ), this, SLOT( onFaceDetBackgrCBClicked( bool ) ) );
	connect( mEnableMarkerDetectCB, SIGNAL( clicked( bool ) ), this, SLOT( setMarkerDetection( bool ) ) );
	connect( mEnableMarkerlessCameraCB, SIGNAL( clicked( bool ) ), this, SLOT( setMarkerlessDetectionCamera( bool ) ) );

	connect( mDisableCursorCB, SIGNAL( clicked() ), this, SLOT( stopMovingCursor() ) );
	connect( mDisableZoomCursorCB, SIGNAL( clicked( bool ) ), this, SLOT( stopZoom() ) );
	connect( mEnableMarkerlessKinectCB, SIGNAL( clicked( bool ) ), this, SLOT( setMarkerlessDetectionKinect( bool ) ) );

}

void QOpenCV::OpenCVWindow::stopMovingCursor()
{
	if ( mDisableCursorCB->isChecked() ) {
		emit setMovementCursor( false );
	}
	else {
		emit setMovementCursor( true );
	}
}

void QOpenCV::OpenCVWindow::stopZoom()
{
	if ( mDisableZoomCursorCB->isChecked() ) {
		emit setZoom( false );
	}
	else {
		emit setZoom( true );
	}
}

void QOpenCV::OpenCVWindow::setMarkerDetection( bool set )
{
	emit setKinectMarkerDetection( set );
}

void QOpenCV::OpenCVWindow::setMarkerlessDetectionCamera( bool set )
{
	emit setCameraMarkerlessDetection( set );
}

void QOpenCV::OpenCVWindow::setMarkerlessDetectionKinect( bool set )
{
	emit setKinectMarkerlessDetection( set );
}

void QOpenCV::OpenCVWindow::setSpeedKinect( int speed )
{
	double _speed= speed/10.0 ;
	emit sendSpeedKinect( _speed );
}

void QOpenCV::OpenCVWindow::onKinectStartCancel( bool checked )
{
	if ( checked ) {
		qDebug() << "Startujem Kinect Thread";
		mKinectPB->setText( tr( "Pause Kinect" ) );
		emit startKinect();
		emit sendImageKinect( true );
	}
	else {
		qDebug() << "Pausujem Kinect Thread";
		mKinectPB->setText( tr( "Start Kinect" ) );
		emit sendImageKinect( false );
	}
}

void QOpenCV::OpenCVWindow::onKinectSnapshotPBClicked()
{
	qDebug() << "Sending signal to Kinect to take snapshot";
	emit setKinectCaptureImage( true );
}

void QOpenCV::OpenCVWindow::onFaceDetBackgrCBClicked( bool checked )
{
	qDebug() << "onFaceDetBackgrCBClicked " << checked;
	emit sendBackgrImgFaceRec( checked );
	if ( checked ) {
		emit sendBackgrImgMarker( false );
		mMarkerBackgrCB->setChecked( false );
	}
}
void QOpenCV::OpenCVWindow::onMarkerBackgrCBClicked( bool checked )
{
	qDebug() << "onMarkerBackgrCBClicked " << checked;
	emit sendBackgrImgMarker( checked );
	if ( checked ) {
		emit sendBackgrImgFaceRec( false );
		mFaceDetBackgrCB->setChecked( false );
	}
}

void QOpenCV::OpenCVWindow::onSelModulChange()
{
	if ( mKinectRB->isChecked() ) {
		mModulesStackL->setCurrentIndex( 0 );
	}

	if ( mArucoRB->isChecked() ) {
		mModulesStackL->setCurrentIndex( 1 );
	}
    if ( mArInteractionRB->isChecked() ) {
        mModulesStackL->setCurrentIndex( 2 );
    }
}

void QOpenCV::OpenCVWindow::onSelSubModulChange()
{
	if ( mFaceRecRB->isChecked() ) {
		mSubmodulesStackL->setCurrentIndex( 0 );
	}

	if ( mMarkerRB->isChecked() ) {
		mSubmodulesStackL->setCurrentIndex( 1 );
	}
	/*
	if ( mMultiMarkerRB->isChecked() ) {
		mSubmodulesStackL->setCurrentIndex( 2 );
	}
	*/
	if ( mNoVideo->isChecked() ) {
		emit sendImgFaceRec( false );
		emit sendImgMarker( false );
		emit setMultiMarker( false );
	}
	else {
		// face recognition
		if ( mFaceRecRB->isChecked() ) {
			emit sendImgMarker( false );
			emit sendImgFaceRec( true );
		}
		// marker
		if ( mMarkerRB->isChecked() ) {
			emit sendImgFaceRec( false );
			emit sendImgMarker( true );
		}
	}
}

void QOpenCV::OpenCVWindow::onUpdateCorPar()
{
	mUpdateCorParPB->setEnabled( false );
}

void QOpenCV::OpenCVWindow::onCorParUpdated()
{
	mUpdateCorParPB->setEnabled( true );
	mCorEnabledCB->setEnabled( true );
}

void QOpenCV::OpenCVWindow::onFaceRecStartCancel( bool checked )
{
	qDebug() << "OpenCVWindow::onFaceRecStartCancel:" << checked;

	if ( checked ) {
		mFaceRecPB->setEnabled( false );
		mFaceRecPB->setText( tr( "Stop FaceRec" ) );
		emit setCapVideoFaceRec( OpenCV::CamSelectCore::getInstance()->selectCamera() );
		emit startFaceRec();
		mFaceRecPB->setEnabled( true );

		if ( Util::ApplicationConfig::get()->getValue( "Viewer.SkyBox.Noise" ).toInt() >1 ) {
			mFaceDetBackgrCB->setEnabled( true );

		}
	}
	else {
		mFaceRecPB->setEnabled( false );
		mFaceDetBackgrCB->setEnabled( false );
		emit stopFaceRec( true );

	}
}

void QOpenCV::OpenCVWindow::onMarkerStartCancel( bool checked )
{
	if ( checked ) {
		mMarkerPB->setEnabled( false );
		mMarkerPB->setText( tr( "Stop Marker" ) );
		mMarkerBehindCB->setEnabled( true );
		mUpdateCorParPB->setEnabled( true );
		emit setCapVideoMarker( OpenCV::CamSelectCore::getInstance()->selectCamera() );
		emit startMarker();
		mMarkerPB->setEnabled( true );
		emit arucoRunning( true );

		if ( Util::ApplicationConfig::get()->getValue( "Viewer.SkyBox.Noise" ).toInt() ==0 ) {
			mMarkerBackgrCB->setEnabled( true );
		}
	}
	else {
		mMarkerPB->setEnabled( false );
		mMarkerBehindCB->setEnabled( false );
		mCorEnabledCB->setEnabled( false );
		mUpdateCorParPB->setEnabled( false );
		mMarkerBackgrCB->setEnabled( false );
		emit stopMarker( true );
		emit arucoRunning( false );
	}
}

void QOpenCV::OpenCVWindow::onFaceRecThrFinished()
{
	mFaceRecPB->setText( tr( "Start FaceRec" ) );
	mFaceRecPB->setEnabled( true );
}

void QOpenCV::OpenCVWindow::onMarkerThrFinished()
{
	mMarkerPB->setText( tr( "Start Marker" ) );
	mMarkerPB->setEnabled( true );
}

void QOpenCV::OpenCVWindow::closeEvent( QCloseEvent* event )
{
	qDebug() << "Closing OpenCV Window";

	disconnect();

	event->accept();
}

void QOpenCV::OpenCVWindow::onMultiMarkerStartCancel( bool checked )
{
	qDebug() << "checked = " << checked;
	if ( checked ) {
		mMultiMarkerPB->setText( tr( "Stop Aruco" ) );
		emit setMultiMarker( true );
		emit setCapVideoMarker( OpenCV::CamSelectCore::getInstance()->selectCamera() );
		emit startMultiMarker();
	}
	else {
		mMultiMarkerPB->setText( tr( "Start Aruco" ) );
		emit stopMultiMarker( true );
		emit setMultiMarker( false );
	}
}

void QOpenCV::OpenCVWindow::setLabel( cv::Mat image )
{
	if ( image.empty() ) {
		mWindowLabel->setText( tr( "Image empty" ) );
		return;
	}

	QImage qimage( image.data, image.cols, image.rows,static_cast<int>( image.step ), QImage::Format_RGB888 );

	mWindowLabel->setPixmap( QPixmap::fromImage( qimage ) );

	image.~Mat();
}

void QOpenCV::OpenCVWindow::onArInteractionSelectionClicked(bool state)
{
    if(arNSPosition->isChecked()){
        emit setArInteractionSelection(0);
    }
    else if(arNSMostEdges->isChecked()){
        emit setArInteractionSelection(1);
    }
    else if(arNSNearest->isChecked()){
        emit setArInteractionSelection(2);
    }
}

void QOpenCV::OpenCVWindow::onArInteractionBehaviourClicked(bool state)
{
    if(arNBSingle->isChecked()){
        emit setArInteractionBehaviour(0);
    }
    else if(arNBCluster->isChecked()){
        emit setArInteractionBehaviour(1);
    }
}

void QOpenCV::OpenCVWindow::applyGraphZoomIn(bool state)
{
    emit setArGraphZoom(2);
}

void QOpenCV::OpenCVWindow::applyGraphZoomOut(bool state)
{
    emit setArGraphZoom(1);
}
