#include <vtkExtractVOI.h>
#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <sstream>
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkProperty.h>

class vtkSliderCallback : public vtkCommand
{
public:
   static vtkSliderCallback *New()
   {
      return new vtkSliderCallback;
   }

   virtual void Execute(vtkObject *caller, unsigned long, void *)
   {
      vtkSliderWidget *sliderWidget =
          reinterpret_cast<vtkSliderWidget *>(caller);

      this->z = static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
      cout << "[AXIAL] -> Slice : " << z << std::endl;
      _ImageViewer->SetSlice(z);
   }

   vtkSliderCallback() : _ImageViewer(0) {}
   vtkImageViewer2 *_ImageViewer;
   int z;
};

int main(int argc, char *argv[])
{
   // Input arguments
   if (argc != 2)
   {
      std::cout << "Folder Name Required but not provided ! (EXIT)" << std::endl;
      return EXIT_FAILURE;
   }

   std::string folder = argv[1];

   // Reading DICOM files within directory
   vtkSmartPointer<vtkDICOMImageReader> readerDCM =
       vtkSmartPointer<vtkDICOMImageReader>::New();
   readerDCM->SetDirectoryName(folder.c_str());
   readerDCM->Update();

   readerDCM->Print(std::cout);

   // Setting Image Viewer
   vtkSmartPointer<vtkImageViewer2> imageViewer =
       vtkSmartPointer<vtkImageViewer2>::New();

   // Axial slicing (By default)
   imageViewer->SetSliceOrientationToXY();

   imageViewer->SetInputConnection(readerDCM->GetOutputPort());
   readerDCM->Update();

   imageViewer->SetSize(1000, 800);

   // Extents of the selected slicing direction (Min & Max)
   int minSlice = imageViewer->GetSliceMin();
   int maxSlice = imageViewer->GetSliceMax();

   // Setting Window Interactor
   vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
       vtkSmartPointer<vtkRenderWindowInteractor>::New();

   imageViewer->SetupInteractor(renderWindowInteractor);

   // Setting Slider Representation
   vtkSmartPointer<vtkSliderRepresentation2D> sliderRep =
       vtkSmartPointer<vtkSliderRepresentation2D>::New();
   sliderRep->SetMinimumValue(minSlice);
   sliderRep->SetMaximumValue(maxSlice);
   sliderRep->SetValue((maxSlice - minSlice) / 2);
   sliderRep->SetTubeWidth(0.001);
   sliderRep->UseBoundsOn();
   sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
   sliderRep->GetPoint1Coordinate()->SetValue(65, 100);
   sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
   sliderRep->GetPoint2Coordinate()->SetValue(65, 400);
   sliderRep->SetSliderLength(0.0075);
   sliderRep->SetSliderWidth(0.01);
   sliderRep->SetEndCapLength(0.0005);
   sliderRep->SetEndCapWidth(0.01);

   // Setting Slider Widget
   vtkSmartPointer<vtkSliderWidget> sliderWidget =
       vtkSmartPointer<vtkSliderWidget>::New();

   sliderWidget->SetInteractor(renderWindowInteractor);
   sliderWidget->SetRepresentation(sliderRep);
   sliderWidget->SetAnimationModeToAnimate();
   sliderWidget->EnabledOn();

   // User Method & Event Handling for the Slider
   vtkSmartPointer<vtkSliderCallback> callback =
       vtkSmartPointer<vtkSliderCallback>::New();

   callback->_ImageViewer = imageViewer;

   sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

   // Rendering

   renderWindowInteractor->Initialize();
   imageViewer->Render();
   imageViewer->GetRenderer()->ResetCamera();
   imageViewer->Render();
   renderWindowInteractor->Start();

   return EXIT_SUCCESS;
}
