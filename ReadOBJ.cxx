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

#define PI 3.1415926

using namespace cv;
using namespace std;
using namespace vtk;
 
int main(int argc, char* argv[])
{
  // Parse command line arguments
  if(argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " Filename(.obj)" << std::endl;
    return EXIT_FAILURE;
  }

  namedWindow( "window_name", CV_WINDOW_AUTOSIZE );

  ofstream association, groundtruth;
  association.open("association.txt");
  groundtruth.open("groundtruth.txt");


  srand (time(NULL));

  float rotation = 0;


   
    string filename = argv[1];

    vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetSize(320, 320);
 
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

  // Visualize
  for(int i = 0; i<360; i++){
    cout << i << endl;

    vtkSmartPointer<vtkPNGReader> pngReader = 
      vtkSmartPointer<vtkPNGReader>::New();
    pngReader->SetFileName("Rabbit_D.png");
    pngReader->Update();

    vtkSmartPointer<vtkTexture> texture = 
      vtkSmartPointer<vtkTexture>::New();

    texture->SetInputConnection(pngReader->GetOutputPort());
    texture->InterpolateOn();

    actor->SetTexture(texture);
    /*int random = rand() % 100 - 50;
    rotation += random;
    actor->RotateX(rotation/25);*/

/*    vtkSmartPointer<vtkTransform> transform = vtkTransform::New();
    transform->Scale(1.0/2000.0,1.0/2000.0,1.0/2000.0);

    actor->SetUserTransform(transform);*/


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

    double depth_min = 0.1, depth_max = 4000.0;
    double view_angle = 360 * (2.0 * std::atan( 320.0 / (2*300.0) ))/(2*M_PI);
    cout << view_angle << endl;
    // ensure the relevant range of depths are rendered
    camera->SetClippingRange(depth_min, depth_max);
    camera->SetViewAngle(view_angle);

    vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();
    renderer->SetActiveCamera(camera);
    renderer->AddActor(actor);
    renderer->SetBackground(.0, .0, .0);

    /*vtkSmartPointer<vtkMatrix4x4> matrix = camera->GetProjectionTransformMatrix(renderer);
    cout << matrix[0][0] << " " << matrix[1][0] << endl; */

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

    Mat img = Mat::zeros(Size(320,320), CV_32F);
    //Mat img = imread("white.exr", CV_LOAD_IMAGE_COLOR);

    for(int x=0; x<320; x++){
      for(int y=0; y<320; y++){
        double z = renderWindow->GetZbufferDataAtPoint(x,y);
        double euclidean_distance;

        if(z==1){
          euclidean_distance = 0;
        }else{
          double coords[3];
          worldPicker->Pick(x, y, z, renderer);
          worldPicker->GetPickPosition(coords);

          /*euclidean_distance = sqrt(
            pow(coords[0], 2) + 
            pow(coords[1], 2) + 
            pow(coords[2], 2));

          double max_euclidean_distance = sqrt(
            pow(2, 2) + 
            pow(2, 2) + 
            pow(depth_max, 2));*/

          //euclidean_distance = euclidean_distance*255/max_euclidean_distance;
          //euclidean_distance = (coords[2])*255/(depth_max);
          euclidean_distance = coords[2];
        }

        //cout << coords[0] << " " << coords[1] << " " << z << " " << endl;
        img.at<float>(y,x) = euclidean_distance;
        //img.at<uchar>(y,x) = 255-euclidean_distance;
        //img.at<uchar>(y,x) = 255-euclidean_distance;  
      }
    }
    stringstream output;
    output << "output/d-" << i << ".exr";

    /*imageWriter->SetFileName(output.str().c_str()); 
    imageWriter->SetInputConnection(scale->GetOutputPort()); 
    imageWriter->Write(); */
    imwrite(output.str(), img);
    association << i << " " << output.str() << " " << i << " " << colorFilename.str() << endl;
    groundtruth << i << " 0 0 0 0 1 0 " << i*PI/365 << endl;
  }
  association.close();
  return EXIT_SUCCESS;
}
