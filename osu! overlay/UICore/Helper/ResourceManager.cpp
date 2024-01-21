#include "ResourceManager.h"

#include "StringHelper.h"

#include <wincodec.h>
#include <fstream>

std::vector<ImageResource> ResourceManagerOld::_images;

void ResourceManagerOld::Init(std::string resourceFilePath, ID2D1DeviceContext* target)
{
    // Create 
    IWICImagingFactory* WICFactory;
    CoCreateInstance
    (
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&WICFactory)
    );

    std::ifstream in(resourceFilePath);
    while (true)
    {
        std::string resourceName;
        std::string resourcePath;
        in >> resourceName;
        in.get();
        bool eof = !std::getline(in, resourcePath);

        std::wstring resourcePathW = string_to_wstring(resourcePath);

        if (!eof)
        {
            HRESULT hr;

            IWICBitmapDecoder* decoder;
            hr = WICFactory->CreateDecoderFromFilename
            (
                resourcePathW.c_str(),
                NULL,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &decoder
            );

            IWICBitmapFrameDecode* source;
            hr = decoder->GetFrame(0, &source);

            IWICFormatConverter* converter;
            hr = WICFactory->CreateFormatConverter(&converter);
            hr = converter->Initialize
            (
                source,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeMedianCut
            );

            ID2D1Bitmap* bitmap;
            hr = target->CreateBitmapFromWicBitmap
            (
                converter,
                NULL,
                &bitmap
            );

            ResourceManagerOld::_images.push_back({ resourceName, bitmap });

            decoder->Release();
            source->Release();
            converter->Release();
        }
        else
        {
            break;
        }
    }

    WICFactory->Release();
}

ID2D1Bitmap* ResourceManagerOld::GetImage(std::string name)
{
    for (auto& resource : ResourceManagerOld::_images)
    {
        if (resource.name == name)
        {
            return resource.bitmap;
        }
    }
    return nullptr;
}

ResourceManager::~ResourceManager()
{
    ReleaseResources();
}

void ResourceManager::ReleaseResources()
{
    for (auto& image : _images)
        image.bitmap->Release();
    _images.clear();
}

void ResourceManager::CoInit()
{
    CoInitialize(NULL);
}

void ResourceManager::CoUninit()
{
    CoUninitialize();
}

void ResourceManager::SetImageResourceFilePath(std::string imageResourceFilePath)
{
    _imageResourceFilePath = imageResourceFilePath;
}

void ResourceManager::SetDeviceContext(ID2D1DeviceContext* target)
{
    _target = target;
}

void ResourceManager::InitAllImages()
{
    _InitImages({});
}

void ResourceManager::InitImages(const std::vector<std::string>& resourceNames)
{
    _InitImages(resourceNames);
}

void ResourceManager::InitImage(std::string resourceName)
{
    _InitImages({ resourceName });
}

ID2D1Bitmap* ResourceManager::GetImage(std::string name)
{
    for (auto& image : _images)
    {
        if (image.name == name)
        {
            return image.bitmap;
        }
    }
    return nullptr;
}

void ResourceManager::_InitImages(const std::vector<std::string>& resourceNames)
{
    if (!_target)
        return;

    HRESULT hr;

    IWICImagingFactory* WICFactory;
    hr = CoCreateInstance
    (
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&WICFactory)
    );

    bool coinit = false;
    if (hr == CO_E_NOTINITIALIZED)
    {
        coinit = true;
        CoInitialize(NULL);
        hr = CoCreateInstance
        (
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&WICFactory)
        );
    }

    std::ifstream in(_imageResourceFilePath);
    while (true)
    {
        std::string resourceName;
        std::string resourcePath;
        in >> resourceName;
        in.get();
        bool eof = !std::getline(in, resourcePath);

        std::wstring resourcePathW = string_to_wstring(resourcePath);

        if (!eof)
        {
            // Only load specified resources
            if (!resourceNames.empty() && std::find(resourceNames.begin(), resourceNames.end(), resourceName) == resourceNames.end())
                continue;

            IWICBitmapDecoder* decoder;
            hr = WICFactory->CreateDecoderFromFilename
            (
                resourcePathW.c_str(),
                NULL,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &decoder
            );

            IWICBitmapFrameDecode* source;
            hr = decoder->GetFrame(0, &source);

            IWICFormatConverter* converter;
            hr = WICFactory->CreateFormatConverter(&converter);
            hr = converter->Initialize
            (
                source,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeMedianCut
            );

            ID2D1Bitmap* bitmap;
            hr = _target->CreateBitmapFromWicBitmap
            (
                converter,
                NULL,
                &bitmap
            );

            _images.push_back({ resourceName, bitmap });

            decoder->Release();
            source->Release();
            converter->Release();
        }
        else
        {
            break;
        }
    }

    WICFactory->Release();

    if (coinit)
    {
        CoUninitialize();
    }
}