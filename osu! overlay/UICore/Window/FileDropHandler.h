#pragma once

#include "WindowsEx.h"
#include "DragDropEventHandler.h"

#include <wrl.h>
#include <shellapi.h>

class FileDropHandler : public IDropTarget
{
public:
    void AddDragDropEventHandler(IDragDropEventHandler* handler)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _handlers.push_back(handler);

        if (_dragging)
        {
            if (handler->OnDragEnter(_draggedPaths) == DropResponse::ALLOW)
            {
                _availableHandlers.push_back(handler);
            }
        }
    }

    bool RemoveDragDropEventHandler(IDragDropEventHandler* handler)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (int i = 0; i < _handlers.size(); i++)
        {
            if (_handlers[i] == handler)
            {
                if (_dragging)
                {
                    for (int j = 0; j < _availableHandlers.size(); j++)
                    {
                        if (_availableHandlers[j] == _handlers[i])
                        {
                            _availableHandlers[j]->OnDragLeave();
                            _availableHandlers.erase(_availableHandlers.begin() + j);
                            break;
                        }
                    }
                }

                _handlers.erase(_handlers.begin() + i);
                return true;
            }
        }
        return false;
    }

    HRESULT DragEnter(
        IDataObject* pDataObj,
        DWORD grfKeyState,
        POINTL pt,
        DWORD* pdwEffect)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _dragging = true;
        _availableHandlers.clear();

        IEnumFORMATETC* enumfmt;
        if (FAILED(pDataObj->EnumFormatEtc(DATADIR_GET, &enumfmt)))
            return S_OK;

        FORMATETC fmt;
        while (enumfmt->Next(1, &fmt, NULL) == S_OK)
        {
            STGMEDIUM stg;
            if (FAILED(pDataObj->GetData(&fmt, &stg)))
                continue;
            switch (stg.tymed)
            {
            case TYMED_HGLOBAL:
            {
                //HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, 0);
                //stg.hGlobal;
                if (fmt.cfFormat == CF_HDROP && fmt.dwAspect == 1)
                {
                    _draggedPaths.clear();
                    UINT filecount = DragQueryFile((HDROP)stg.hGlobal, 0xFFFFFFFF, NULL, NULL);
                    for (UINT i = 0; i < filecount; i++)
                    {
                        UINT strLen = DragQueryFile((HDROP)stg.hGlobal, i, NULL, NULL);
                        std::wstring path;
                        path.resize((size_t)strLen + 1);
                        if (DragQueryFile((HDROP)stg.hGlobal, i, path.data(), strLen + 1) != 0)
                        {
                            if (path.back() == L'\0')
                                path.erase(path.end() - 1);
                            _draggedPaths.push_back(path);
                        }
                    }

                    // Gather available handlers
                    for (int i = 0; i < _handlers.size(); i++)
                    {
                        if (_handlers[i]->OnDragEnter(_draggedPaths) == DropResponse::ALLOW)
                        {
                            _availableHandlers.push_back(_handlers[i]);
                        }
                    }
                }

                break;
            }
            default:
                break;
            }
            ReleaseStgMedium(&stg);
        }

        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    HRESULT DragOver(
        DWORD grfKeyState,
        POINTL pt,
        DWORD* pdwEffect)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        POINT offset;
        offset.x = 0;
        offset.y = 0;
        ClientToScreen(_hwnd, &offset);
        int clientX = pt.x - offset.x;
        int clientY = pt.y - offset.y;

        bool overHandler = false;
        for (int i = 0; i < _availableHandlers.size(); i++)
        {
            _availableHandlers[i]->OnDrag(clientX, clientY);
            RECT rect = _availableHandlers[i]->GetDropRect();
            if (clientX >= rect.left &&
                clientX < rect.right &&
                clientY >= rect.top &&
                clientY < rect.bottom)
            {
                overHandler = true;
            }
        }

        if (overHandler)
            *pdwEffect = DROPEFFECT_COPY;
        else
            *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    HRESULT DragLeave()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _dragging = false;

        for (int i = 0; i < _availableHandlers.size(); i++)
            _availableHandlers[i]->OnDragLeave();

        return S_OK;
    }

    HRESULT Drop(
        IDataObject* pDataObj,
        DWORD grfKeyState,
        POINTL pt,
        DWORD* pdwEffect)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _dragging = false;

        POINT offset;
        offset.x = 0;
        offset.y = 0;
        ClientToScreen(_hwnd, &offset);
        int clientX = pt.x - offset.x;
        int clientY = pt.y - offset.y;

        for (int i = 0; i < _availableHandlers.size(); i++)
        {
            RECT rect = _availableHandlers[i]->GetDropRect();
            if (clientX >= rect.left &&
                clientX < rect.right &&
                clientY >= rect.top &&
                clientY < rect.bottom)
            {
                _availableHandlers[i]->OnDrop(clientX, clientY, _draggedPaths);
            }
        }

        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) { return S_OK; }
    ULONG STDMETHODCALLTYPE AddRef(void) { return 0; }
    ULONG STDMETHODCALLTYPE Release(void) { return 0; }

    FileDropHandler(HWND hwnd)
    {
        _hwnd = hwnd;
    }

private:
    HWND _hwnd;

    std::mutex _mutex;
    bool _dragging = false;
    std::vector<IDragDropEventHandler*> _availableHandlers;
    std::vector<IDragDropEventHandler*> _handlers;
    std::vector<std::wstring> _draggedPaths;
};