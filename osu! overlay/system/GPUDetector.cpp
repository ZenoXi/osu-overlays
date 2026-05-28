#include "GPUDetector.h"

#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")

std::vector<GPUDetector::GPUType> GPUDetector::_detectedGPUs;
std::mutex GPUDetector::_mtx;

void GPUDetector::UpdateGPUInfo()
{
	std::lock_guard<std::mutex> lock(_mtx);

	_detectedGPUs.clear();

    IDXGIFactory* pFactory = nullptr;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    if (FAILED(hr) || !pFactory)
        return;

    UINT i = 0;
    IDXGIAdapter* pAdapter = nullptr;
    while (pFactory->EnumAdapters(i++, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        if (SUCCEEDED(pAdapter->GetDesc(&desc)))
        {
			if (desc.VendorId == 0x10DE)
				_detectedGPUs.push_back(GPUType::NVIDIA);
			else if (desc.VendorId == 0x1002)
				_detectedGPUs.push_back(GPUType::AMD);
			else if (desc.VendorId == 0x8086)
				_detectedGPUs.push_back(GPUType::INTEL);
			else
				_detectedGPUs.push_back(GPUType::UNKNOWN);
        }
        pAdapter->Release();
    }

    pFactory->Release();
}

std::optional<GPUDetector::GPUType> GPUDetector::GetPrimaryGPUType()
{
	std::lock_guard<std::mutex> lock(_mtx);

	if (_detectedGPUs.empty())
		return std::nullopt;
	return _detectedGPUs[0];
}

bool GPUDetector::AtLeastOneOfType(GPUType type)
{
	std::lock_guard<std::mutex> lock(_mtx);

	for (GPUType gpu : _detectedGPUs)
	{
		if (gpu == type)
			return true;
	}
	return false;
}

bool GPUDetector::AllOfType(GPUType type)
{
	std::lock_guard<std::mutex> lock(_mtx);

	for (GPUType gpu : _detectedGPUs)
	{
		if (gpu != type)
			return false;
	}
	return !_detectedGPUs.empty();
}

std::vector<GPUDetector::GPUType> GPUDetector::GetDetectedGPUs()
{
	std::lock_guard<std::mutex> lock(_mtx);
	return _detectedGPUs;
}

std::string GPUTypeToString(GPUDetector::GPUType type)
{
	switch (type)
	{
	case GPUDetector::GPUType::UNKNOWN:
		return "Unknown";
	case GPUDetector::GPUType::NVIDIA:
		return "NVIDIA";
	case GPUDetector::GPUType::AMD:
		return "AMD";
	case GPUDetector::GPUType::INTEL:
		return "Intel";
	default:
		return "Invalid";
	}
}