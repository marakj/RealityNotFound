#include "QOSG/ProjectiveARViewer.h"
#include "QOSG/ProjectiveARWindow.h"

#include "Viewer/PickHandler.h"
#include "Viewer/CameraManipulator.h"
#include "Viewer/CoreGraph.h"
#include "Util/ApplicationConfig.h"
#include "Core/Core.h"

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/TexGenNode>
#include <osg/TexMat>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <math.h>

// used code from http://jotschi.de/2009/05/31/projective-textures-with-openscenegraph/
osg::StateSet* QOSG::ProjectiveARViewer::createProjectorState(osg::Texture2D* texture, osg::Vec3 projectorPos, osg::Vec3 projectorDirection, float projectorFOV) {

    osg::StateSet* stateset = new osg::StateSet;

    /* 1. Load the texture that will be projected */
    if (!texture){
        texture = new osg::Texture2D();
        osg::Image* foo = osgDB::readImageFile("foo.jpg");
        if (foo == NULL)
            printf("Error: Image \"foo.jpg\" has not been read!\n");
        texture->setImage(foo);
    }
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER);
    stateset->setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);


    // set up tex gens
    stateset->setTextureMode(1, GL_TEXTURE_GEN_S,
        osg::StateAttribute::ON);
    stateset->setTextureMode(1, GL_TEXTURE_GEN_T,
        osg::StateAttribute::ON);
    stateset->setTextureMode(1, GL_TEXTURE_GEN_R,
        osg::StateAttribute::ON);
    stateset->setTextureMode(1, GL_TEXTURE_GEN_Q,
        osg::StateAttribute::ON);

    /* 2. Load the Shaders */
    osg::ref_ptr<osg::Program> projProg(new osg::Program);
    const std::string vertexSource =
        "varying vec4 projCoord; \n"
        "void main() \n"
        "{ \n"
        "	projCoord = gl_TextureMatrix[1] * gl_Vertex; \n"
        "	gl_Position = ftransform(); \n"
        "	gl_FrontColor = gl_Color; \n"
        "} \n";
    osg::ref_ptr<osg::Shader> projvertexShader(new osg::Shader(
        osg::Shader::VERTEX, vertexSource));

    const std::string fragmentSource =
        "uniform sampler2D projectionMap; \n"
        "varying vec4 projCoord; \n"
        "void main() \n"
        "{ \n"
        "	vec4 dividedCoord = projCoord / projCoord.w; \n"
        "	vec2 texCoord = dividedCoord.st + vec2(0.5f, 0.5f); \n"
        "	vec4 color = texture2D(projectionMap, texCoord); \n"
        "	gl_FragColor = color * gl_Color; \n"
        "} \n";
    osg::ref_ptr<osg::Shader> projfragShader(new osg::Shader(
        osg::Shader::FRAGMENT, fragmentSource));

    projProg->addShader(projvertexShader.get());
    projProg->addShader(projfragShader.get());

    /* 3. Handover the texture to the fragment shader via uniform */
    osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D,
        "projectionMap");
    texUniform->set(1);
    stateset->addUniform(texUniform);

    /* 4. set Texture matrix*/
    osg::TexMat* texMat = new osg::TexMat;
    osg::Matrix mat;
    mat = osg::Matrixd::lookAt(projectorPos, projectorPos + projectorDirection, up)
        * osg::Matrixd::perspective(projectorFOV, 1.0, 0.1, SCENE_MAX_SIZE);
    texMat->setMatrix(mat);
    stateset->setTextureAttributeAndModes(1, texMat, osg::StateAttribute::ON);

    stateset->setAttribute(projProg.get());
    return stateset;
}


osg::Geode* QOSG::ProjectiveARViewer::createBase()
{
    osg::Geode* baseGeode = new osg::Geode();
    osg::Geometry* baseGeometry = new osg::Geometry();

    baseGeode->addDrawable(baseGeometry);

    //base
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(-0.75, -1.0, 2.0));    // lb   floor
    vertices->push_back(osg::Vec3( 0.75, -1.0, 2.0));    // rb   floor
    vertices->push_back(osg::Vec3( 0.75, -1.0, 0));      // rb   screen
    vertices->push_back(osg::Vec3(-0.75, -1.0, 0));      // lb   screen
    vertices->push_back(osg::Vec3( 0.75, 1.00, 0));   // rt   screen
    vertices->push_back(osg::Vec3(-0.75, 1.00, 0));   // lt   screen

    //right side
    vertices->push_back(osg::Vec3(0.77, -1.0, 0.08));       // fb   rack
    vertices->push_back(osg::Vec3(0.77, -0.98, 0.08));    // fm   rack
    vertices->push_back(osg::Vec3(0.77, -0.935, 0.015));    // ft   rack
    vertices->push_back(osg::Vec3(0.77, -1.00, 0.015));    // sb   rack
    //left side
    vertices->push_back(osg::Vec3(-0.77, -1.0, 0.08));       // fb   rack
    vertices->push_back(osg::Vec3(-0.77, -0.98, 0.08));    // fm   rack
    vertices->push_back(osg::Vec3(-0.77, -0.935, 0.015));    // ft   rack
    vertices->push_back(osg::Vec3(-0.77, -1.00, 0.015));    // sb   rack
    baseGeometry->setVertexArray(vertices);

    // floor
    osg::DrawElementsUInt* floor = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    floor->push_back(3);
    floor->push_back(2);
    floor->push_back(1);
    floor->push_back(0);

    baseGeometry->addPrimitiveSet(floor);

    // screen
    osg::DrawElementsUInt* screen = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    screen->push_back(5);
    screen->push_back(4);
    screen->push_back(2);
    screen->push_back(3);

    baseGeometry->addPrimitiveSet(screen);

    // rack
    osg::DrawElementsUInt* frontBotRack = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    frontBotRack->push_back(6);
    frontBotRack->push_back(7);
    frontBotRack->push_back(11);
    frontBotRack->push_back(10);

    baseGeometry->addPrimitiveSet(frontBotRack);

    osg::DrawElementsUInt* frontTopRack = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    frontTopRack->push_back(7);
    frontTopRack->push_back(8);
    frontTopRack->push_back(12);
    frontTopRack->push_back(11);

    baseGeometry->addPrimitiveSet(frontTopRack);

    osg::DrawElementsUInt* sideRightRack = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    sideRightRack->push_back(6);
    sideRightRack->push_back(7);
    sideRightRack->push_back(8);
    sideRightRack->push_back(9);

    baseGeometry->addPrimitiveSet(sideRightRack);

    osg::DrawElementsUInt* sideLeftRack = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    sideLeftRack->push_back(10);
    sideLeftRack->push_back(11);
    sideLeftRack->push_back(12);
    sideLeftRack->push_back(13);

    baseGeometry->addPrimitiveSet(sideLeftRack);



    return baseGeode;
}


osg::Group* QOSG::ProjectiveARViewer::createProjectorScene() {

    unsigned int tex_width = 2048;
    unsigned int tex_height = 2048;
    unsigned int samples = 0;
    unsigned int colorSamples = 0;

    osg::Camera::RenderTargetImplementation renderImplementation = osg::Camera::FRAME_BUFFER_OBJECT;

    // load model to view
    osg::Node* model = viewerPerspective->getSceneData();
    if(!model)
        model = osgDB::readNodeFile("cessna.osgt");

    if (!model){
        return NULL;
    }

    // texture to render to and to use for rendering of graph.
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

    // create the camera node to do the render to texture
    if (!renderCamera)
        renderCamera = new osg::Camera;

    // set up the background color and clear mask.
    renderCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    renderCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double fovy, aspectRatio, zNear, zFar;
    viewerPerspective->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    if(useGraph){
        const osg::BoundingSphere& bs = coreGraph->getNodesGroup()->getGroup()->getBound();/*viewerPerspective->getSceneData()->getBound();*/
        if (!bs.valid()){
            qDebug() << "[ProjectiveARViewer::createProjectorScene] Bounding sphere is not valid!";
            return NULL;
        }

        // setting View Matrix
        osg::Vec3d viewerRelPos(viewerPos - graphPos);
        osg::Vec3d renderCameraRelPos(viewerRelPos * (bs.radius() / graphRadius));
        renderCamera->setViewMatrixAsLookAt(renderCameraRelPos + bs.center(), bs.center(), osg::Vec3(0.0f, 1.0f, 0.0f));

        // setting Projection Matrix (compute frustum parameters)
        double dist = (double) (renderCameraRelPos).length();
        renderCamera->setProjectionMatrix(createFrustumForSphere(bs.radius(), dist));
    }
    else{
        renderCamera->setProjectionMatrixAsPerspective(viewerFOV, 1.0, zNear, zFar);
        renderCamera->setViewMatrix(viewerPerspective->getCamera()->getViewMatrix());
    }
    renderCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    // set viewport
    renderCamera->setViewport(0, 0, tex_width, tex_height);

    // set the camera to render before the main camera.
    renderCamera->setRenderOrder(osg::Camera::PRE_RENDER);

    // tell the camera to use OpenGL frame buffer object where supported.
    renderCamera->setRenderTargetImplementation(renderImplementation);

    // attach the texture and use it as the color buffer.
    renderCamera->attach(osg::Camera::COLOR_BUFFER, texture,
        0, 0, false,
        samples, colorSamples);

    // add subgraph to render
    renderCamera->addChild(model);

    osg::Group* projectorScene = new osg::Group;
    base = createBase();

    /* Enable projective texturing for all objects of base */
    if(useGraph){
        base->setStateSet(createProjectorState(texture, viewerPos, graphPos - viewerPos, viewerFOV));
    }
    else{
        base->setStateSet(createProjectorState(texture, viewerPos, viewerDir, viewerFOV));
    }

    projectorScene->addChild(base);
    projectorScene->addChild(renderCamera);

    return projectorScene;
}

QOSG::ProjectiveARViewer::ProjectiveARViewer( QWidget* parent , const char* name , const QGLWidget* shareWidget , WindowFlags f , QOSG::ProjectiveARWindow* window, osgViewer::Viewer* viewerPerspective, Vwr::CoreGraph* coreGraph ):
    AdapterWidget( parent, name, shareWidget, f )
{
    this->window = window;
    this->coreGraph = coreGraph;
    this->viewerPerspective = viewerPerspective;

    appConf = Util::ApplicationConfig::get();

    renderCamera = new osg::Camera;
    useGraph = false;

    // setup Graph (radius 0.5m)
    graphPos = osg::Vec3d(0.0, 0.00, 0.50);
    graphRadius = 0.5;

    // setup Viewer
    viewerPos = osg::Vec3d(0.0, 0.00, 1.50);
    viewerDir = osg::Vec3d(0.0, 0.00, 0.00) - viewerPos;
    viewerFOV = 90;

    this->setSceneData(createProjectorScene());

    // setup Projector
    projectorPos = osg::Vec3d(0.0, 0.00, 1.50);
    projectorDir = osg::Vec3d(0.0, 0.00, 0.00) - projectorPos;
    projectorFOV = 30;

    getCamera()->setViewport( new osg::Viewport( 0,0,width(),height() ) );
    getCamera()->setProjectionMatrixAsPerspective( projectorFOV, static_cast<double>( width() )/static_cast<double>( height() ), 0.01, appConf->getValue( "Viewer.Display.ViewDistance" ).toFloat() );
    getCamera()->setGraphicsContext( getGraphicsWindow() );
    getCamera()->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );

    getCamera()->setClearColor( osg::Vec4( 0, 0, 0, 1 ) );
    getCamera()->setViewMatrixAsLookAt( projectorPos, projectorDir + projectorPos, osg::Vec3d( 0, 1, 0 ) );


    addEventHandler( new osgViewer::StatsHandler );

    setThreadingModel( osgViewer::ViewerBase::SingleThreaded );

    connect( &_timer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );

    _timer.start( 10 );
}

void QOSG::ProjectiveARViewer::reloadConfig()
{

}

void QOSG::ProjectiveARViewer::paintGL()
{
    frame();
}

void QOSG::ProjectiveARViewer::updateRenderCamera()
{
    double fovy, aspectRatio, zNear, zFar;
    viewerPerspective->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    if(useGraph){
        const osg::BoundingSphere& bs = coreGraph->getNodesGroup()->getGroup()->getBound();/*viewerPerspective->getSceneData()->getBound();*/
        if (!bs.valid()){
            qDebug() << "[ProjectiveARViewer::paintGL] Bounding sphere is not valid!";
            return ;
        }

        // setting View Matrix
        osg::Vec3d viewerRelPos(viewerPos - graphPos);
        osg::Vec3d renderCameraRelPos(viewerRelPos * (bs.radius() / graphRadius));
        renderCamera->setViewMatrixAsLookAt(renderCameraRelPos + bs.center(), bs.center(), up);

        // setting Projection Matrix (compute frustum parameters)
        double dist = (double) (renderCameraRelPos).length();
        renderCamera->setProjectionMatrix(createFrustumForSphere(bs.radius(), dist));
    }
    else{
        renderCamera->setProjectionMatrixAsPerspective(viewerFOV, 1.0, zNear, zFar);
        renderCamera->setViewMatrix(viewerPerspective->getCamera()->getViewMatrix());
    }
}

void QOSG::ProjectiveARViewer::updateViewer()
{
    osg::TexMat* texMat = dynamic_cast<osg::TexMat*>(base->getStateSet()->getTextureAttribute(1,osg::StateAttribute::TEXMAT));
    osg::Matrix mat;

    if(useGraph){
        double dist = (double) (viewerPos - graphPos).length();
        mat = osg::Matrixd::lookAt(viewerPos, graphPos, up)
            * createFrustumForSphere(graphRadius, dist);
    }
    else{
        mat = osg::Matrixd::lookAt(viewerPos, viewerDir + viewerPos, up)
            * renderCamera->getProjectionMatrix();
    }
    texMat->setMatrix(mat);
}

void QOSG::ProjectiveARViewer::updateProjector()
{
    getCamera()->setProjectionMatrixAsPerspective( projectorFOV, static_cast<double>( width() )/static_cast<double>( height() ), 0.01, SCENE_MAX_SIZE );
    getCamera()->setViewMatrixAsLookAt( projectorPos, projectorDir + projectorPos, up );
}

void QOSG::ProjectiveARViewer::updateScene()
{
    updateRenderCamera();
    updateViewer();
    updateProjector();
}


void QOSG::ProjectiveARViewer::setViewerPosByFaceDetection(float x, float y, float z)
{
    //qDebug() << "[ProjectiveARViewer::setViewerPosByFaceDetection] Passed arguments: x=\"" << x << "\", y=\"" << y << "\", z=\"" << z << "\"";
    viewerPos = osg::Vec3d(x,y,z);
    window->setViewerPos(viewerPos.x(), viewerPos.y(), viewerPos.z());
    updateScene();
}

osg::Matrixd QOSG::ProjectiveARViewer::createFrustumForSphere(float radius, float distance)
{
    // 0.1(m) is a distance where the sphere will start to disappear
    double zNear = (distance - radius > 0.1) ? distance - radius : 0.1;
    double zFar = distance + radius;
    // sin(67.5°) ~ 0.92388  -> 135° = angle of horizontal and vertical field of view
    double a = (radius / distance < 0.92388) ? zNear * ( radius / distance ) : zNear * 0.92388;

    /*qDebug() << "[ProjectiveARViewer::createFrustumForSphere] dist  = " << distance;
    qDebug() << "[ProjectiveARViewer::createFrustumForSphere] zNear = " << zNear;
    qDebug() << "[ProjectiveARViewer::createFrustumForSphere] zFar  = " << zFar;
    qDebug() << "[ProjectiveARViewer::createFrustumForSphere] a     = " << a;*/

    return osg::Matrixd::frustum(-a, a, -a, a, zNear, zFar);
}
