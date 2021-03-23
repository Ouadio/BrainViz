#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>

#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkImageData.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>

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
      slicer->SetSliceNumber(z);
   }

   vtkSliderCallback() : slicer(0) {}
   vtkImageSliceMapper *slicer;
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

   // Extracting Image Data
   vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
   imageData = readerDCM->GetOutput();

   int volumeExtents[6];
   imageData->GetExtent(volumeExtents);

   // Slice-Mapping 3D Images to get 2D Mappings
   vtkSmartPointer<vtkImageSliceMapper> imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();

   imageSliceMapper->SetSliceNumber((volumeExtents[5] - volumeExtents[4]) / 3);
   imageSliceMapper->SetInputData(imageData);
   imageSliceMapper->Update();

   // Image Slicer Actor
   vtkSmartPointer<vtkImageSlice> imageSlicer = vtkSmartPointer<vtkImageSlice>::New();
   imageSlicer->SetMapper(imageSliceMapper);

   // Extents of the selected slicing direction (Min & Max) - along z so far -
   int minSlice = volumeExtents[4];
   int maxSlice = volumeExtents[5];

   // Setup renderers
   vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

   renderer->AddViewProp(imageSlicer); // Actor
   renderer->ResetCamera();

   // Setup render window
   vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
   renderWindow->SetSize(1000, 1000);

   renderWindow->AddRenderer(renderer);

   // Setting Window Image Interactor
   vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
       vtkSmartPointer<vtkRenderWindowInteractor>::New();

   vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();

   renderWindowInteractor->SetInteractorStyle(style);

   renderWindowInteractor->SetRenderWindow(renderWindow);

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

   callback->slicer = imageSliceMapper;

   sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

   // Rendering

   renderWindowInteractor->Initialize();
   renderWindow->Render();
   renderWindowInteractor->Start();

   return EXIT_SUCCESS;
}
