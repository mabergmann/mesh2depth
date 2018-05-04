#include <vtkOBJReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWorldPointPicker.h>
#include <vtkMatrix4x4.h>
#include <vtkPerspectiveTransform.h>
#include <vtkCamera.h>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <vtkJPEGWriter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageShiftScale.h>
#include <vtkTransform.h>
#include <vtkPNGReader.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <random>

#define PI 3.1415926

using namespace cv;
using namespace std;
using namespace vtk;

static int height = 320;
static int width = 480;

static bool add_gaussian_noise = true;
static double gaussian_noise_standard_deviation = 0.2;
 
int main(int argc, char* argv[])
{
  // Parse command line arguments
  if(argc < 2){
      cout << "Usage: " << argv[0] << " mesh.obj [texture.png]" << endl;
      return -1;
    }
    string filename = argv[1];
    string texture_filename = "";
    if(argc >= 3){
      texture_filename = argv[2];
    }

  namedWindow( "window_name", CV_WINDOW_AUTOSIZE );

  ofstream association, groundtruth;
  association.open("association.txt");
  groundtruth.open("groundtruth.txt");


  srand (time(NULL));

  float rotation = 0;
    

    vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetSize(width, height);
 
    vtkSmartPointer<vtkOBJReader> reader = 
      vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update(); 
    vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort()); 
    vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->AddPosition(0,0,2);

    vtkSmartPointer<vtkPNGReader> pngReader = 
      vtkSmartPointer<vtkPNGReader>::New();
    pngReader->SetFileName(texture_filename.c_str());
    pngReader->Update();

    vtkSmartPointer<vtkTexture> texture = 
      vtkSmartPointer<vtkTexture>::New();

    texture->SetInputConnection(pngReader->GetOutputPort());
    texture->InterpolateOn();

    actor->SetTexture(texture);

    double depth_min = 0.1, depth_max = 4.0;
    double view_angle = 360 * (2.0 * std::atan( height / (2*300.0) ))/(2*M_PI);
    cout << "Fov: " << view_angle << endl;

  // Visualize
  for(int i = 0; i<360; i++){
    cout << i << endl;

    actor->AddPosition(0,0,-2);
    actor->RotateWXYZ(1,0,1,0);
    actor->AddPosition(0,0,2);

    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();

    // the camera can stay at the origin because we are transforming the scene objects
    camera->SetPosition(0, 0, 0);
    // look in the +Z direction of the camera coordinate system
    camera->SetFocalPoint(0, 0, 1);
    // the camera Y axis points down
    camera->SetViewUp(0,1,0);

    // ensure the relevant range of depths are rendered
    camera->SetClippingRange(depth_min, depth_max);
    camera->SetViewAngle(view_angle);

    vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();
    renderer->SetActiveCamera(camera);
    renderer->AddActor(actor);
    renderer->SetBackground(.0, .0, .0);

    renderWindow->AddRenderer(renderer);
    renderWindow->Render();
    
    vtkSmartPointer<vtkWindowToImageFilter> filter = 
      vtkSmartPointer<vtkWindowToImageFilter>::New(); 
    vtkSmartPointer<vtkJPEGWriter> imageWriter = 
      vtkSmartPointer<vtkJPEGWriter>::New(); 
    
    filter->SetInput(renderWindow); 
    filter->SetMagnification(1); 
    filter->SetInputBufferTypeToRGBA();
    filter->ReadFrontBufferOff();
    filter->Update();

    stringstream colorFilename;
    colorFilename << "output/c-" << i << ".jpg";

    imageWriter->SetFileName(colorFilename.str().c_str());
    imageWriter->SetInputConnection(filter->GetOutputPort());
    imageWriter->Write();

    filter = NULL;
    imageWriter = NULL;

    vtkSmartPointer<vtkWorldPointPicker> worldPicker = 
      vtkSmartPointer<vtkWorldPointPicker>::New();

    Mat img = Mat::zeros(Size(width,height), CV_32F);
    //Mat img = imread("white.exr", CV_LOAD_IMAGE_COLOR);

    for(int x=0; x<width; x++){
      for(int y=0; y<height; y++){
        double z = renderWindow->GetZbufferDataAtPoint(x,y);
        double euclidean_distance;

        if(z==1){
          euclidean_distance = 0;
        }else{
          double coords[3];
          worldPicker->Pick(x, y, z, renderer);
          worldPicker->GetPickPosition(coords);
          euclidean_distance = coords[2];
        }

        if(add_gaussian_noise){
          normal_distribution<double> distribution(euclidean_distance, gaussian_noise_standard_deviation);
          default_random_engine generator;
          euclidean_distance = distribution(generator);
        }

        img.at<float>(y,x) = euclidean_distance; 
      }
    }
    stringstream output;
    output << "output/d-" << i << ".exr";
    imwrite(output.str(), img);
    association << i << " " << output.str() << " " << i << " " << colorFilename.str() << endl;
    groundtruth << i << " 0 0 0 0 1 0 " << i*PI/365 << endl;
  }
  association.close();
  return EXIT_SUCCESS;
}
