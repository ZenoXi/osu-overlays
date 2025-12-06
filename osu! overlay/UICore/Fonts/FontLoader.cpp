#include "FontLoader.h"

//IDWriteFontCollectionLoader* FontCollectionLoader::_instance( new(std::nothrow) FontCollectionLoader() );

HRESULT STDMETHODCALLTYPE FontCollectionLoader::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontCollectionLoader))
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE FontCollectionLoader::AddRef()
{
    return InterlockedIncrement(&_refCount);
}

ULONG STDMETHODCALLTYPE FontCollectionLoader::Release()
{
    ULONG newCount = InterlockedDecrement(&_refCount);
    if (newCount == 0)
        delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE FontCollectionLoader::CreateEnumeratorFromKey(
    IDWriteFactory* factory,
    void const* collectionKey,
    UINT32 collectionKeySize,
    OUT IDWriteFontFileEnumerator** fontFileEnumerator
)
{
    *fontFileEnumerator = NULL;
    HRESULT hr = S_OK;

    FontFileEnumerator* enumerator = new(std::nothrow) FontFileEnumerator(factory, _fontFilePathsRef);
    if (enumerator == NULL)
        return E_OUTOFMEMORY;

    *fontFileEnumerator = SafeAcquire(enumerator);
    return hr;
}

// ------------------------------ FontFileEnumerator ----------------------------------------------------------

FontFileEnumerator::FontFileEnumerator(IDWriteFactory* factory, const std::vector<std::wstring>* fontFilePathsRef) :
    _refCount(0),
    _factory(SafeAcquire(factory)),
    _currentFile(),
    _nextIndex(0),
    _fontFilePathsRef(fontFilePathsRef)
{}

HRESULT STDMETHODCALLTYPE FontFileEnumerator::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileEnumerator))
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE FontFileEnumerator::AddRef()
{
    return InterlockedIncrement(&_refCount);
}

ULONG STDMETHODCALLTYPE FontFileEnumerator::Release()
{
    ULONG newCount = InterlockedDecrement(&_refCount);
    if (newCount == 0)
        delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE FontFileEnumerator::MoveNext(OUT BOOL* hasCurrentFile)
{
    HRESULT hr = S_OK;

    *hasCurrentFile = FALSE;
    SafeRelease(&_currentFile);

    if (_nextIndex < _fontFilePathsRef->size())
    {
        hr = _factory->CreateFontFileReference(
            (*_fontFilePathsRef)[_nextIndex].c_str(),
            NULL,
            &_currentFile
        );

        if (SUCCEEDED(hr))
        {
            *hasCurrentFile = TRUE;
            _nextIndex++;
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE FontFileEnumerator::GetCurrentFontFile(OUT IDWriteFontFile** fontFile)
{
    *fontFile = SafeAcquire(_currentFile);
    return (fontFile != NULL) ? S_OK : E_FAIL;
}

// ---------------------------------------- FontContext ---------------------------------------------------------

FontContext::FontContext(IDWriteFactory* pFactory)
  : _dwriteFactory(pFactory), 
    _fontCollectionLoader(new FontCollectionLoader(&_fontFilePaths))
{
    HRESULT hr = _dwriteFactory->RegisterFontCollectionLoader(_fontCollectionLoader);
    if (hr != S_OK)
    {
        // TODO: Logging
    }
}

FontContext::~FontContext()
{
    HRESULT hr = _dwriteFactory->UnregisterFontCollectionLoader(_fontCollectionLoader);
    if (hr != S_OK)
    {
        // TODO: Logging
    }
    _fontCollectionLoader->Release();
}

void FontContext::AddFontFiles(const std::vector<std::wstring>& filePaths)
{
    for (auto& path : filePaths)
    {
        if (std::find(_fontFilePaths.begin(), _fontFilePaths.end(), path) != _fontFilePaths.end())
            continue;
        _fontFilePaths.push_back(path);
    }
    SafeRelease(&_currentFontCollection);
}

IDWriteFontCollection* FontContext::GetFontCollection()
{
    if (!_currentFontCollection)
    {
        HRESULT hr = _dwriteFactory->CreateCustomFontCollection(_fontCollectionLoader, nullptr, 0, &_currentFontCollection);
        if (hr != S_OK)
        {
            // TODO: Logging
        }
    }
    return _currentFontCollection;
}