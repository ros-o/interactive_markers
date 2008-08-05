/*
 * Copyright (c) 2008, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <wx/wx.h>

#include "../ogre_visualizer/visualization_panel.h"
#include "../ogre_visualizer/visualizers/grid_visualizer.h"
#include "../ogre_visualizer/visualizers/axes_visualizer.h"
#include "../ogre_visualizer/visualizers/point_cloud_visualizer.h"
#include "../ogre_visualizer/visualizers/laser_scan_visualizer.h"
#include "../ogre_visualizer/visualizers/robot_model_visualizer.h"

#include "Ogre.h"

#include "ros/node.h"
#include "ros/common.h"

using namespace ogre_vis;

class MyFrame : public wxFrame
{
public:
  MyFrame(wxWindow* parent) : wxFrame(parent, -1, _("Ogre Visualizer Test App"),
                      wxDefaultPosition, wxSize(800,600),
                      wxDEFAULT_FRAME_STYLE)
  {
    m_Root = new Ogre::Root();
    m_Root->loadPlugin( "RenderSystem_GL" );
    m_Root->loadPlugin( "Plugin_OctreeSceneManager" );
    m_Root->loadPlugin( "Plugin_CgProgramManager" );

    std::vector<std::string> paths;
    std::string mediaPath = ros::get_package_path( "gazebo_robot_description" );
    mediaPath += "/world/Media/";
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath, "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "fonts", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "materials", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "materials/scripts", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "materials/programs", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "materials/textures", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "models", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( mediaPath + "models/pr2", "FileSystem", "General" );

    std::vector<std::string>::iterator dirIt = paths.begin();
    std::vector<std::string>::iterator dirEnd = paths.end();
    for ( ; dirIt != dirEnd; ++dirIt )
    {
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation( *dirIt, "FileSystem", "General" );
    }

    // Taken from gazebo
    Ogre::RenderSystemList *rsList = m_Root->getAvailableRenderers();

    Ogre::RenderSystem* renderSystem = NULL;
    Ogre::RenderSystemList::iterator renderIt = rsList->begin();
    Ogre::RenderSystemList::iterator renderEnd = rsList->end();
    for ( ; renderIt != renderEnd; ++renderIt )
    {
        renderSystem = *renderIt;

        if ( renderSystem->getName() == "OpenGL Rendering Subsystem" )
        {
            break;
        }
    }

    if ( renderSystem == NULL )
    {
      printf( "Could not find the opengl rendering subsystem!\n" );
      exit(1);
    }

    renderSystem->setConfigOption("Full Screen","No");
    renderSystem->setConfigOption("FSAA","2");
    renderSystem->setConfigOption("RTT Preferred Mode", "PBuffer");

    m_Root->setRenderSystem( renderSystem );

    try
    {
        m_Root->initialise( false );
    }
    catch ( Ogre::Exception& e )
    {
        printf( "Failed to initialize Ogre::Root: %s\n", e.what() );
        exit(1);
    }

    m_VisualizationPanel = new VisualizationPanel( this, m_Root );

    // TODO: figure out how we can move this back to before panel (renderwindow) creation (manually creating a GpuProgramManager?)
    try
    {
      Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }
    catch ( Ogre::Exception& e )
    {
      printf( "Error initializing resource groups: %s", e.what() );
      exit( 1 );
    }

    m_VisualizationPanel->CreateVisualizer<GridVisualizer>( "Grid 1", true );
    m_VisualizationPanel->CreateVisualizer<GridVisualizer>( "Grid 2", true );

    PointCloudVisualizer* pointCloud = m_VisualizationPanel->CreateVisualizer<PointCloudVisualizer>( "Head Full Cloud", true );
    pointCloud->SetTopic( "full_cloud" );
    pointCloud->SetColor( 1.0, 1.0, 0.0 );

    LaserScanVisualizer* laserScan = m_VisualizationPanel->CreateVisualizer<LaserScanVisualizer>( "Head Scan", true );
    laserScan->SetScanTopic( "cloud" );
    laserScan->SetColor( 0.0, 1.0, 0.0 );

    laserScan = m_VisualizationPanel->CreateVisualizer<LaserScanVisualizer>( "Floor Scan", true );
    laserScan->SetScanTopic( "scan" );

    m_VisualizationPanel->CreateVisualizer<AxesVisualizer>( "Origin Axes", true );

    RobotModelVisualizer* model = m_VisualizationPanel->CreateVisualizer<RobotModelVisualizer>( "Robot Model", true );
    model->Initialize( "robotdesc/pr2", "transform" );
  }

  ~MyFrame()
  {
    m_VisualizationPanel->Destroy();

    delete m_Root;
  }

private:

  Ogre::Root* m_Root;
  Ogre::Camera* m_Camera;
  Ogre::SceneManager* m_SceneManager;

  VisualizationPanel* m_VisualizationPanel;
};

// our normal wxApp-derived class, as usual
class MyApp : public wxApp
{
public:

  bool OnInit()
  {
    ros::init(argc, (char**)argv);

    wxFrame* frame = new MyFrame(NULL);
    SetTopWindow(frame);
    frame->Show();
    return true;
  }

  int OnExit()
  {
      return 0;
  }
};

DECLARE_APP(MyApp);
IMPLEMENT_APP(MyApp);