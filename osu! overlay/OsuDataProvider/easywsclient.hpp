
// The MIT License (MIT)
// 
// Copyright (c) 2012, 2013 <dhbaird@gmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef EASYWSCLIENT_HPP_20120819_MIOFVASDTNUASZDQPLFD
#define EASYWSCLIENT_HPP_20120819_MIOFVASDTNUASZDQPLFD

// This code comes from:
// https://github.com/dhbaird/easywsclient
//
// To get the latest version:
// wget https://raw.github.com/dhbaird/easywsclient/master/easywsclient.hpp
// wget https://raw.github.com/dhbaird/easywsclient/master/easywsclient.cpp

#include <string>
#include <vector>

namespace easywsclient {

    struct Callback_Imp { virtual void operator()(const std::string& message) = 0; };
    struct BytesCallback_Imp { virtual void operator()(const std::vector<uint8_t>& message) = 0; };

    class WebSocket {
    public:
        typedef WebSocket* pointer;
        typedef enum readyStateValues { CLOSING, CLOSED, CONNECTING, OPEN } readyStateValues;

        // Factories:
        static pointer create_dummy();
        static pointer from_url(const std::string& url, const std::string& origin = std::string());
        static pointer from_url_no_mask(const std::string& url, const std::string& origin = std::string());

        // Interfaces:
        virtual ~WebSocket() { }
        virtual void poll(int timeout = 0) = 0; // timeout in milliseconds
        virtual void send(const std::string& message) = 0;
        virtual void sendBinary(const std::string& message) = 0;
        virtual void sendBinary(const std::vector<uint8_t>& message) = 0;
        virtual void sendPing() = 0;
        virtual void close() = 0;
        virtual readyStateValues getReadyState() const = 0;

        template<class Callable>
        void dispatch(Callable callable)
            // For callbacks that accept a string argument.
        { // N.B. this is compatible with both C++11 lambdas, functors and C function pointers
            struct _Callback : public Callback_Imp {
                Callable& callable;
                _Callback(Callable& callable) : callable(callable) { }
                void operator()(const std::string& message) { callable(message); }
            };
            _Callback callback(callable);
            _dispatch(callback);
        }

        template<class Callable>
        void dispatchBinary(Callable callable)
            // For callbacks that accept a std::vector<uint8_t> argument.
        { // N.B. this is compatible with both C++11 lambdas, functors and C function pointers
            struct _Callback : public BytesCallback_Imp {
                Callable& callable;
                _Callback(Callable& callable) : callable(callable) { }
                void operator()(const std::vector<uint8_t>& message) { callable(message); }
            };
            _Callback callback(callable);
            _dispatchBinary(callback);
        }

    protected:
        virtual void _dispatch(Callback_Imp& callable) = 0;
        virtual void _dispatchBinary(BytesCallback_Imp& callable) = 0;
    };

} // namespace easywsclient

#endif /* EASYWSCLIENT_HPP_20120819_MIOFVASDTNUASZDQPLFD */