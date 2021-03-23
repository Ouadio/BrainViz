#pragma once

#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor.h>

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

#include <vtkPolyDataMapper.h>
#include <vtkOutlineFilter.h>

#include <vtkMarchingCubes.h>
#include <vtkStripper.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>

// Helper class for formating slice status message
class SlicingStatusMessage
{
public:
    static std::string Format(int slice,
                              int maxSlice,
                              std::string name)
    {
        std::stringstream tmp;
        tmp << name << " Slice  " << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};

// Helper for Slider's User Method & Event Handling
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