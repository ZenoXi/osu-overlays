#pragma once

#include <string>
#include "Common.h"

class FontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
    FontCollectionLoader(const std::vector<std::wstring>* fontFilePathsRef) : _refCount(1), _fontFilePathsRef(fontFilePathsRef) {}

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
        IDWriteFactory* factory,
        void const* collectionKey,
        UINT32 collectionKeySize,
        OUT IDWriteFontFileEnumerator** fontFileEnumerator
    );

private:
    ULONG _refCount;

    const std::vector<std::wstring>* _fontFilePathsRef;
};

class FontFileEnumerator : public IDWriteFontFileEnumerator
{
public:
    FontFileEnumerator(IDWriteFactory* factory, const std::vector<std::wstring>* fontFilePathsRef);
    ~FontFileEnumerator()
    {
        SafeRelease(&_currentFile);
        SafeRelease(&_factory);
    }

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
    size_t _nextIndex;
    const std::vector<std::wstring>* _fontFilePathsRef;
};

class FontContext
{
public:
    FontContext(IDWriteFactory* pFactory);
    ~FontContext();
    void AddFontFiles(const std::vector<std::wstring>& filePaths);
    IDWriteFontCollection* GetFontCollection();

private:
    FontContext(FontContext const&) = delete;
    FontContext(FontContext &&) = delete;
    void operator=(FontContext const&) = delete;
    void operator=(FontContext &&) = delete;

    IDWriteFactory* _dwriteFactory = nullptr;
    FontCollectionLoader* _fontCollectionLoader = nullptr;

    std::vector<std::wstring> _fontFilePaths;
    IDWriteFontCollection* _currentFontCollection = nullptr;
};