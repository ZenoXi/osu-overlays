#pragma once

#pragma comment( lib,"d2d1.lib" )
#include <d2d1_1.h>

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
    ID2D1Bitmap* GetImage(std::string name);

private:
    // Empty vector loads all
    void _InitImages(const std::vector<std::string>& resourceNames);

    ID2D1DeviceContext* _target = nullptr;

    std::string _imageResourceFilePath;
    std::vector<ImageResource> _images;
};