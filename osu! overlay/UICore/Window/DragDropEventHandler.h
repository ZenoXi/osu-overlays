#pragma once

#include <windef.h>

#include <vector>
#include <string>
#include <mutex>
#include <filesystem>
#include <iostream>

enum class DropResponse
{
    DENY,
    ALLOW
};

struct DropResult
{
    int x = 0;
    int y = 0;
    std::vector<std::wstring> paths;
};

class IDragDropEventHandler
{
public:
    virtual RECT GetDropRect() = 0;
    virtual DropResponse OnDragEnter(const std::vector<std::wstring>& paths) = 0;
    virtual void OnDragLeave() = 0;
    virtual void OnDrag(int x, int y) = 0;
    virtual void OnDrop(int x, int y, const std::vector<std::wstring>& paths) = 0;
};

class DragDropHandler : public IDragDropEventHandler
{
public:
    /* INTERFACE START */

    RECT GetDropRect()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _dropRect;
    }

    DropResponse OnDragEnter(const std::vector<std::wstring>& paths)
    {
        namespace fls = std::filesystem;

        std::lock_guard<std::mutex> lock(_mutex);

        if (!_allowMultipleFiles && paths.size() > 1)
            return DropResponse::DENY;
        if (!_allowFolders)
            for (auto& path : paths)
                if (fls::is_directory(fls::path(path)))
                    return DropResponse::DENY;
        if (!_allowedExtensions.empty())
        {
            bool someMatch = false;
            for (auto& path : paths)
            {
                if (fls::is_regular_file(path))
                {
                    fls::path fpath = fls::path(path);
                    std::wstring ext = L"";
                    if (fpath.has_extension())
                        ext = fpath.extension().wstring().substr(1);
                    if (std::find(_allowedExtensions.begin(), _allowedExtensions.end(), ext) == _allowedExtensions.end())
                    {
                        if (_matchAllExtensions)
                            return DropResponse::DENY;
                    }
                    else
                    {
                        if (!_matchAllExtensions)
                        {
                            someMatch = true;
                            break;
                        }
                    }
                }
                else if (fls::is_directory(path))
                {
                    auto iterator = fls::recursive_directory_iterator(path);
                    for (const auto& entry : iterator)
                    {
                        if (!entry.is_regular_file())
                            continue;

                        auto path = entry.path();
                        std::wstring ext = L"";
                        if (path.has_extension())
                            ext = path.extension().wstring().substr(1);
                        if (std::find(_allowedExtensions.begin(), _allowedExtensions.end(), ext) == _allowedExtensions.end())
                        {
                            if (_matchAllExtensions)
                                return DropResponse::DENY;
                        }
                        else
                        {
                            if (!_matchAllExtensions)
                            {
                                someMatch = true;
                                break;
                            }
                        }
                    }
                    if (someMatch)
                        break;
                }
            }
        }

        _dragging = true;
        return DropResponse::ALLOW;
    }

    void OnDragLeave()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _dragging = false;
        _mouseInRect = false;
    }

    void OnDrag(int x, int y)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _curX = x;
        _curY = y;

        if (x >= _dropRect.left &&
            x < _dropRect.right &&
            y >= _dropRect.top &&
            y < _dropRect.bottom)
            _mouseInRect = true;
        else
            _mouseInRect = false;
    }

    void OnDrop(int x, int y, const std::vector<std::wstring>& paths)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_dropResult)
            _dropResult = std::make_unique<DropResult>();

        _dropResult->x = x;
        _dropResult->y = y;
        _dropResult->paths = paths;

        _dragging = false;
        _mouseInRect = false;
    }

    /* INTERFACE END */

    void SetDropRect(RECT rect)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _dropRect = rect;
    }

    void SetAllowedExtensions(std::vector<std::wstring> allowedExtensions)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _allowedExtensions = allowedExtensions;
    }

    void SetAllowFolders(bool allow)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _allowFolders = allow;
    }

    void SetAllowMultipleFiles(bool allow)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _allowMultipleFiles = allow;
    }

    void SetMatchAllExtensions(bool matchAll)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _matchAllExtensions = matchAll;
    }

    std::unique_ptr<DropResult> GetDropResult()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_dropResult)
            return std::move(_dropResult);
        else
            return nullptr;
    }

    bool Dragging()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _dragging;
    }

    bool MouseInRect()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _mouseInRect;
    }

    std::pair<int, int> MousePos()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return { _curX, _curY };
    }

    std::pair<int, int> MouseRectPos()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return { _curX - _dropRect.left, _curY - _dropRect.top };
    }

    DragDropHandler(RECT dropRect)
    {
        _dropRect = dropRect;
    }

private:
    std::mutex _mutex;

    RECT _dropRect;
    bool _dragging = false;
    bool _mouseInRect = false;
    int _curX = 0;
    int _curY = 0;
    std::vector<std::wstring> _allowedExtensions;
    bool _matchAllExtensions = true;
    bool _allowFolders = true;
    bool _allowMultipleFiles = true;

    std::unique_ptr<DropResult> _dropResult = nullptr;
};