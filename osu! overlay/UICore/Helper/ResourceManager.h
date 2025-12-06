#pragma once

#include "UICore/Window/DirectX.h"
#include "UICore/Model/Bitmap.h"

#include <string>
#include <vector>

struct ImageResource
{
    std::string name;
    ID2D1Bitmap* bitmap;
};

class ResourceManagerOld
{
    ResourceManagerOld() {}

    static std::vector<ImageResource> _images;

public:
    static void Init(std::string resourceFilePath, ID2D1DeviceContext* target);
    static ID2D1Bitmap* GetImage(std::string name);
};

namespace zcom
{
    class StandaloneBitmap : public BitmapStorage
    {
        ID2D1Bitmap* _bitmap;

    public:
        StandaloneBitmap(ID2D1Bitmap* bitmap) : _bitmap(bitmap) {}
        ~StandaloneBitmap() {}

        Size GetSize() const override { return { (int)_bitmap->GetSize().width, (int)_bitmap->GetSize().height }; }
        bool CanBeTarget() const override { return false; }
        ID2D1Bitmap* GetSource() const override { return _bitmap; }
        Rect GetSourceRect() const override { return GetSize().ToRect(); }
    };
}

class ResourceManager
{
public:
    ~ResourceManager();
    void ReleaseResources();

    void CoInit();
    void CoUninit();

    // Change the image resource file path
    void SetImageResourceFilePath(std::string resourceFilePath);
    // Set the device context to use for creating bitmaps for images
    void SetDeviceContext(ID2D1DeviceContext* target);

    // Loads all images from the resource file
    void InitAllImages();
    // Loads images with the specified resource names from the resource file
    void InitImages(const std::vector<std::string>& resourceNames);
    // Loads an image with the specified resource name from the resource file
    void InitImage(std::string resourceName);

    // Returns an image with the specified resource name
    std::optional<zcom::Bitmap> GetImage(std::string name);

private:
    // Empty vector loads all
    void _InitImages(const std::vector<std::string>& resourceNames);

    ID2D1DeviceContext* _target = nullptr;

    std::string _imageResourceFilePath;
    std::vector<ImageResource> _images;
};