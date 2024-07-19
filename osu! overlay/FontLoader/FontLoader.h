#pragma once
#include <string>
#include "Common.h"

class FontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
    FontCollectionLoader() : _refCount(1)
    {
    }

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontCollectionLoader methods
    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
        IDWriteFactory* factory,
        void const* collectionKey,
        UINT32 collectionKeySize,
        OUT IDWriteFontFileEnumerator** fontFileEnumerator
    );

    //// Gets the singleton loader instance.
    //static IDWriteFontCollectionLoader* GetLoader()
    //{
    //    return _instance;
    //}

    //static bool IsLoaderInitialized()
    //{
    //    return _instance != NULL;
    //}

private:
    ULONG _refCount;
    //static IDWriteFontCollectionLoader* _instance;
};

class FontFileEnumerator : public IDWriteFontFileEnumerator
{
public:
    FontFileEnumerator(IDWriteFactory* factory);
    ~FontFileEnumerator()
    {
        SafeRelease(&_currentFile);
        SafeRelease(&_factory);
    }
    HRESULT Initialize(UINT const* collectionKey, UINT32 keySize);

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileEnumerator methods
    virtual HRESULT STDMETHODCALLTYPE MoveNext(OUT BOOL* hasCurrentFile);
    virtual HRESULT STDMETHODCALLTYPE GetCurrentFontFile(OUT IDWriteFontFile** fontFile);

private:
    ULONG _refCount;

    IDWriteFactory* _factory;
    IDWriteFontFile* _currentFile;
    std::vector<std::wstring> _filePaths;
    size_t _nextIndex;
};

class FontContext
{
public:
    FontContext(IDWriteFactory* pFactory);
    ~FontContext();
    HRESULT Initialize();
    HRESULT CreateFontCollection(std::vector<std::wstring>& newCollection, OUT IDWriteFontCollection** result);

private:
    FontContext(FontContext const&) = delete;
    void operator=(FontContext const&) = delete;

    HRESULT InitializeInternal();
    IDWriteFactory* _dwriteFactory;
    static std::vector<size_t> cKeys;
    FontCollectionLoader* _fontCollectionLoader = nullptr;

    HRESULT _hr;
};

class FontGlobals
{
public:
    FontGlobals() {}
    static size_t push(std::vector<std::wstring>& addCollection)
    {
        size_t ret = fontCollections.size();
        fontCollections.push_back(addCollection);
        return ret;
    }
    static std::vector<std::vector<std::wstring>>& collections()
    {
        return fontCollections;
    }
private:
    static std::vector<std::vector<std::wstring>> fontCollections;
};