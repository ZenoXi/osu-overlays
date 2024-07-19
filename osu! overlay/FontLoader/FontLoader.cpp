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

    if (collectionKeySize % sizeof(UINT) != 0)
        return E_INVALIDARG;

    FontFileEnumerator* enumerator = new(std::nothrow) FontFileEnumerator(factory);
    if (enumerator == NULL)
        return E_OUTOFMEMORY;

    UINT const* key = static_cast<UINT const*>(collectionKey);
    UINT32 const keySize = collectionKeySize;
    hr = enumerator->Initialize(key, keySize);
    if (FAILED(hr))
    {
        delete enumerator;
        return hr;
    }

    *fontFileEnumerator = SafeAcquire(enumerator);
    return hr;
}

// ------------------------------ FontFileEnumerator ----------------------------------------------------------

FontFileEnumerator::FontFileEnumerator(IDWriteFactory* factory) :
    _refCount(0),
    _factory(SafeAcquire(factory)),
    _currentFile(),
    _nextIndex(0)
{}

HRESULT FontFileEnumerator::Initialize(UINT const* collectionKey, UINT32 keySize)
{
    try
    {
        // dereference collectionKey in order to get index of collection in FontGlobals::fontCollections vector
        UINT cPos = *collectionKey;
        for (auto it = FontGlobals::collections()[cPos].begin(); it != FontGlobals::collections()[cPos].end(); ++it)
            _filePaths.push_back(it->c_str());
    }
    catch (...)
    {
        return ExceptionToHResult();
    }
    return S_OK;
}

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

    if (_nextIndex < _filePaths.size())
    {
        hr = _factory->CreateFontFileReference(
            _filePaths[_nextIndex].c_str(),
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
  : _hr(S_FALSE),
    _dwriteFactory(pFactory), 
    _fontCollectionLoader(new FontCollectionLoader())
{}

FontContext::~FontContext()
{
    _dwriteFactory->UnregisterFontCollectionLoader(_fontCollectionLoader);
    _fontCollectionLoader->Release();
}

HRESULT FontContext::Initialize()
{
    if (_hr == S_FALSE)
        _hr = InitializeInternal();
    return _hr;
}

HRESULT FontContext::InitializeInternal()
{
    HRESULT hr = S_OK;

    //if (!FontCollectionLoader::IsLoaderInitialized())
    //    return E_FAIL;

    // Register our custom loader with the factory object.
    hr = _dwriteFactory->RegisterFontCollectionLoader(_fontCollectionLoader);

    return hr;
}

HRESULT FontContext::CreateFontCollection(std::vector<std::wstring>& newCollection, OUT IDWriteFontCollection** result)
{
    *result = NULL;
    HRESULT hr = S_OK;

    // save new collection in FontGlobals::fontCollections vector
    size_t collectionKey = FontGlobals::push(newCollection);
    cKeys.push_back(collectionKey);
    const void* fontCollectionKey = &cKeys.back();
    UINT32 keySize = sizeof(collectionKey);

    hr = Initialize();
    if (FAILED(hr))
        return hr;

    hr = _dwriteFactory->CreateCustomFontCollection(
        _fontCollectionLoader,
        fontCollectionKey,
        keySize,
        result
    );

    return hr;
}

std::vector<size_t> FontContext::cKeys = std::vector<size_t>(0);

// ----------------------------------- FontGlobals ---------------------------------------------------------

std::vector<std::vector<std::wstring>> FontGlobals::fontCollections = std::vector<std::vector<std::wstring>>(0);