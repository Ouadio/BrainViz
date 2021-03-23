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
#include <vtkCamera.h>
#include <vtkImageProperty.h>
#include <vtkImageStack.h>

#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkTextActor.h>

// Helper class for formating slice status message
class SlicingStatusMessage
{
public:
    static std::string Format(int slice, int maxSlice, std::string name)
    {
        std::stringstream tmp;
        tmp << name << " Slice  " << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};

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
        cout << "[" << name << "] -> Slice : " << z << std::endl;
        slicer->SetSliceNumber(z);

        std::string msg = SlicingStatusMessage::Format(z, sliderWidget->GetSliderRepresentation()->GetMaximumValue(), name);

        statMapper->SetInput(msg.c_str());
    }

    vtkSliderCallback() : slicer(0), statMapper(0) {}
    vtkImageSliceMapper *slicer;
    std::string name;
    vtkTextMapper *statMapper;
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

    // AXIAL
    // Slice-Mapping 3D Images to get 2D Mappings
    vtkSmartPointer<vtkImageSliceMapper> imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();

    imageSliceMapper->SetOrientationToZ();
    imageSliceMapper->SetInputData(imageData);
    imageSliceMapper->Update();

    // Extents of the selected slicing direction (Min & Max) - along z so far -
    int minSlice = imageSliceMapper->GetSliceNumberMinValue();
    int maxSlice = imageSliceMapper->GetSliceNumberMaxValue();

    std::cout << "[AXIAL BOUNDARIES] : " << minSlice << " ---- " << maxSlice << std::endl;

    imageSliceMapper->SetSliceNumber((maxSlice - minSlice) / 2);
    imageSliceMapper->Update();

    //SAGITAL
    // Slice-Mapping 3D Images to get 2D Mappings
    vtkSmartPointer<vtkImageSliceMapper> imageSliceMapperSagital = vtkSmartPointer<vtkImageSliceMapper>::New();

    imageSliceMapperSagital->SetOrientationToX();
    imageSliceMapperSagital->SetInputData(imageData);
    imageSliceMapperSagital->Update();

    // Extents of the selected slicing direction (Min & Max) - along z so far -
    int minSliceSagital = imageSliceMapperSagital->GetSliceNumberMinValue();
    int maxSliceSagital = imageSliceMapperSagital->GetSliceNumberMaxValue();

    std::cout << "[SAGITAL BOUNDARIES] : " << minSliceSagital << " ---- " << maxSliceSagital << std::endl;

    imageSliceMapperSagital->SetSliceNumber((maxSliceSagital - minSliceSagital) / 2);
    imageSliceMapperSagital->Update();

    // CORONAL
    // Slice-Mapping 3D Images to get 2D Mappings
    vtkSmartPointer<vtkImageSliceMapper> imageSliceMapperCoronal = vtkSmartPointer<vtkImageSliceMapper>::New();

    imageSliceMapperCoronal->SetOrientationToY();
    imageSliceMapperCoronal->SetInputData(imageData);
    imageSliceMapperCoronal->Update();

    // Extents of the selected slicing direction (Min & Max) - along z so far -
    int minSliceCoronal = imageSliceMapperCoronal->GetSliceNumberMinValue();
    int maxSliceCoronal = imageSliceMapperCoronal->GetSliceNumberMaxValue();

    std::cout << "[CORONAL BOUNDARIES] : " << minSliceCoronal << " ---- " << maxSliceCoronal << std::endl;

    imageSliceMapperCoronal->SetSliceNumber((maxSliceCoronal - minSliceCoronal) / 2);
    imageSliceMapperCoronal->Update();

    // Image Slicer Actor
    // AXIAL
    vtkSmartPointer<vtkImageSlice> imageSlicer = vtkSmartPointer<vtkImageSlice>::New();
    imageSlicer->SetMapper(imageSliceMapper);
    imageSlicer->Update();

    // SAGITAL
    vtkSmartPointer<vtkImageSlice> imageSlicerSagital = vtkSmartPointer<vtkImageSlice>::New();
    imageSlicerSagital->SetMapper(imageSliceMapperSagital);

    imageSlicerSagital->RotateWXYZ(-90, 0, 1, 0);
    imageSlicerSagital->RotateWXYZ(90, 0, 0, 1);

    imageSlicerSagital->Update();

    // CORONAL
    vtkSmartPointer<vtkImageSlice> imageSlicerCoronal = vtkSmartPointer<vtkImageSlice>::New();
    imageSlicerCoronal->SetMapper(imageSliceMapperCoronal);

    imageSlicerCoronal->RotateWXYZ(90, 1, 0, 0);

    imageSlicerCoronal->Update();

    // AXIAL BASE
    vtkSmartPointer<vtkImageSlice> imageSlicerAxial3D = vtkSmartPointer<vtkImageSlice>::New();
    imageSlicerAxial3D->SetMapper(imageSliceMapper);
    imageSlicerAxial3D->GetProperty()->SetOpacity(0.5);

    imageSlicerAxial3D->RotateWXYZ(45, 1, 1, -1);

    imageSlicerAxial3D->Update();

    // CORONAL BASE
    vtkSmartPointer<vtkImageSlice> imageSlicerCoronal3D = vtkSmartPointer<vtkImageSlice>::New();
    imageSlicerCoronal3D->SetMapper(imageSliceMapperCoronal);
    imageSlicerCoronal3D->GetProperty()->SetOpacity(0.5);

    imageSlicerCoronal3D->RotateWXYZ(45, 1, 1, -1);

    imageSlicerCoronal3D->Update();

    // SAGITAL BASE
    vtkSmartPointer<vtkImageSlice> imageSlicerSagital3D = vtkSmartPointer<vtkImageSlice>::New();
    imageSlicerSagital3D->SetMapper(imageSliceMapperSagital);
    imageSlicerSagital3D->GetProperty()->SetOpacity(0.5);

    imageSlicerSagital3D->RotateWXYZ(45, 1, 1, -1);

    imageSlicerSagital3D->Update();

    // Setup renderers

    // AXIAL
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddViewProp(imageSlicer); // Actor
    renderer->SetViewport(0, 0.5, 0.5, 1);

    // SAGITAL
    vtkSmartPointer<vtkRenderer> rendererSagital = vtkSmartPointer<vtkRenderer>::New();
    rendererSagital->AddViewProp(imageSlicerSagital); // Actor
    rendererSagital->SetViewport(0.5, 0.5, 1, 1);

    // CORONAL
    vtkSmartPointer<vtkRenderer> rendererCoronal = vtkSmartPointer<vtkRenderer>::New();
    rendererCoronal->AddViewProp(imageSlicerCoronal); // Actor
    rendererCoronal->SetViewport(0.5, 0, 1, 0.5);

    // Stack 3D
    vtkSmartPointer<vtkImageStack> imageStack = vtkSmartPointer<vtkImageStack>::New();
    imageStack->AddImage(imageSlicerAxial3D);
    imageStack->AddImage(imageSlicerSagital3D);
    imageStack->AddImage(imageSlicerCoronal3D);

    imageStack->SetActiveLayer(1);

    // Setup renderers
    vtkSmartPointer<vtkRenderer> rendererStack = vtkSmartPointer<vtkRenderer>::New();
    rendererStack->AddViewProp(imageStack);
    rendererStack->SetViewport(0, 0, 0.5, 0.5);

    // Setup render window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetSize(1200, 1200);

    renderWindow->AddRenderer(renderer);
    renderWindow->AddRenderer(rendererSagital);
    renderWindow->AddRenderer(rendererCoronal);
    renderWindow->AddRenderer(rendererStack);

    // Setting Window Image Interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();

    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    renderWindowInteractor->SetInteractorStyle(style);

    renderWindowInteractor->SetRenderWindow(renderWindow);

    // Setting Slider Representation (AXIAL)
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
    sliderRep->SetSliderLength(0.013);
    sliderRep->SetSliderWidth(0.013);
    sliderRep->SetEndCapLength(0.0005);
    sliderRep->SetEndCapWidth(0.01);

    sliderRep->SetTitleText("Slice Control");
    sliderRep->GetTitleProperty()->SetFontFamilyToCourier();
    sliderRep->GetTitleProperty()->SetOrientation(90);
    sliderRep->GetTitleProperty()->SetLineOffset(30);
    sliderRep->GetTitleProperty()->SetLineSpacing(30);

    // Setting Slider Representation (SAGITAL)
    vtkSmartPointer<vtkSliderRepresentation2D> sliderRepS =
        vtkSmartPointer<vtkSliderRepresentation2D>::New();
    sliderRepS->SetMinimumValue(minSliceSagital);
    sliderRepS->SetMaximumValue(maxSliceSagital);
    sliderRepS->SetValue((maxSliceSagital - minSliceSagital) / 2);
    sliderRepS->SetTubeWidth(0.001);
    sliderRepS->UseBoundsOn();
    sliderRepS->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    sliderRepS->GetPoint1Coordinate()->SetValue(85, 100);
    sliderRepS->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    sliderRepS->GetPoint2Coordinate()->SetValue(85, 400);
    sliderRepS->SetSliderLength(0.013);
    sliderRepS->SetSliderWidth(0.013);
    sliderRepS->SetEndCapLength(0.0005);
    sliderRepS->SetEndCapWidth(0.01);

    // Setting Slider Representation (CORONAL)
    vtkSmartPointer<vtkSliderRepresentation2D> sliderRepC =
        vtkSmartPointer<vtkSliderRepresentation2D>::New();
    sliderRepC->SetMinimumValue(minSliceCoronal);
    sliderRepC->SetMaximumValue(maxSliceCoronal);
    sliderRepC->SetValue((maxSliceCoronal - minSliceCoronal) / 2);
    sliderRepC->SetTubeWidth(0.001);
    sliderRepC->UseBoundsOn();
    sliderRepC->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    sliderRepC->GetPoint1Coordinate()->SetValue(105, 100);
    sliderRepC->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    sliderRepC->GetPoint2Coordinate()->SetValue(105, 400);
    sliderRepC->SetSliderLength(0.013);
    sliderRepC->SetSliderWidth(0.013);
    sliderRepC->SetEndCapLength(0.0005);
    sliderRepC->SetEndCapWidth(0.01);

    // slice status message
    // AXIAL
    vtkSmartPointer<vtkTextProperty> sliceTextProp = vtkSmartPointer<vtkTextProperty>::New();
    sliceTextProp->SetFontFamilyToCourier();
    sliceTextProp->SetFontSize(20);
    sliceTextProp->SetVerticalJustificationToBottom();
    sliceTextProp->SetJustificationToLeft();
    vtkSmartPointer<vtkTextMapper> sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    std::string msg = SlicingStatusMessage::Format((maxSlice + minSlice) / 2, maxSlice, "AXIAL");
    sliceTextMapper->SetInput(msg.c_str());
    sliceTextMapper->SetTextProperty(sliceTextProp);
    vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
    sliceTextActor->SetMapper(sliceTextMapper);
    sliceTextActor->SetPosition(10, 10);

    // slice status message
    // SAGITAL

    vtkSmartPointer<vtkTextMapper> sliceTextMapperSagital = vtkSmartPointer<vtkTextMapper>::New();
    std::string msgSagital = SlicingStatusMessage::Format((maxSliceSagital + minSliceSagital) / 2, maxSliceSagital, "SAGITAL");
    sliceTextMapperSagital->SetInput(msgSagital.c_str());
    sliceTextMapperSagital->SetTextProperty(sliceTextProp);
    vtkSmartPointer<vtkActor2D> sliceTextActorSagital = vtkSmartPointer<vtkActor2D>::New();
    sliceTextActorSagital->SetMapper(sliceTextMapperSagital);
    sliceTextActorSagital->SetPosition(10, 10);

    // slice status message
    // CORONAL
    vtkSmartPointer<vtkTextMapper> sliceTextMapperCoronal = vtkSmartPointer<vtkTextMapper>::New();
    std::string msgCoronal = SlicingStatusMessage::Format((maxSliceCoronal + minSliceCoronal) / 2, maxSliceCoronal, "CORONAL");
    sliceTextMapperCoronal->SetInput(msgCoronal.c_str());
    sliceTextMapperCoronal->SetTextProperty(sliceTextProp);
    vtkSmartPointer<vtkActor2D> sliceTextActorCoronal = vtkSmartPointer<vtkActor2D>::New();
    sliceTextActorCoronal->SetMapper(sliceTextMapperCoronal);
    sliceTextActorCoronal->SetPosition(10, 10);

    // Adding text status actors to respective renderers
    renderer->AddActor2D(sliceTextActor);
    rendererSagital->AddActor2D(sliceTextActorSagital);
    rendererCoronal->AddActor2D(sliceTextActorCoronal);

    // Setting Slider Widget
    //AXIAL
    vtkSmartPointer<vtkSliderWidget> sliderWidget =
        vtkSmartPointer<vtkSliderWidget>::New();

    sliderWidget->SetInteractor(renderWindowInteractor);
    sliderWidget->SetRepresentation(sliderRep);
    sliderWidget->SetAnimationModeToAnimate();
    sliderWidget->EnabledOn();

    // User Method & Event Handling for the Slider
    // AXIAL
    vtkSmartPointer<vtkSliderCallback> callback =
        vtkSmartPointer<vtkSliderCallback>::New();

    callback->slicer = imageSliceMapper;
    callback->name.assign("AXIAL");
    callback->statMapper = sliceTextMapper;

    sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

    // Setting Slider Widget
    // SAGITAL

    vtkSmartPointer<vtkSliderWidget> sliderWidgetSagital =
        vtkSmartPointer<vtkSliderWidget>::New();

    sliderWidgetSagital->SetInteractor(renderWindowInteractor);
    sliderWidgetSagital->SetRepresentation(sliderRepS);
    sliderWidgetSagital->SetAnimationModeToAnimate();
    sliderWidgetSagital->EnabledOn();

    // User Method & Event Handling for the Slider
    // SAGITAL

    vtkSmartPointer<vtkSliderCallback> callbackSagital =
        vtkSmartPointer<vtkSliderCallback>::New();

    callbackSagital->slicer = imageSliceMapperSagital;
    callbackSagital->name.assign("SAGITAL");
    callbackSagital->statMapper = sliceTextMapperSagital;

    sliderWidgetSagital->AddObserver(vtkCommand::InteractionEvent, callbackSagital);

    // Setting Slider Widget
    // CORONAL

    vtkSmartPointer<vtkSliderWidget> sliderWidgetCoronal =
        vtkSmartPointer<vtkSliderWidget>::New();

    sliderWidgetCoronal->SetInteractor(renderWindowInteractor);
    sliderWidgetCoronal->SetRepresentation(sliderRepC);
    sliderWidgetCoronal->SetAnimationModeToAnimate();
    sliderWidgetCoronal->EnabledOn();

    // User Method & Event Handling for the Slider
    // CORONAL

    vtkSmartPointer<vtkSliderCallback> callbackCoronal =
        vtkSmartPointer<vtkSliderCallback>::New();

    callbackCoronal->slicer = imageSliceMapperCoronal;
    callbackCoronal->name.assign("CORONAL");
    callbackCoronal->statMapper = sliceTextMapperCoronal;

    sliderWidgetCoronal->AddObserver(vtkCommand::InteractionEvent, callbackCoronal);

    // Rendering

    renderWindowInteractor->Initialize();
    renderWindow->Render();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
