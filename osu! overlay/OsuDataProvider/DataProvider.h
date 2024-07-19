#pragma once

#include "GameState.h"
#include "easywsclient.hpp"
#include "Helper/EventEmitter.h"
#include "Helper/StringHelper.h"

#include <thread>
#include <mutex>

namespace osu
{
    class DataProvider
    {
    public:
        enum ConnectionEvent
        {
            CONNECTION_SUCCESSFUL,
            CONNECTION_FAILED,
            DISCONNECT_COMPLETED
        };

        DataProvider()
        {
            _stop.store(false);
            _runnerThread = std::thread(&DataProvider::_Runner, this);
        }
        ~DataProvider()
        {
            _stop.store(true);
            if (_runnerThread.joinable())
                _runnerThread.join();
        }

        // If a synchronous handler of this event is set, it MUST NOT call any OsuDataProvider methods, as doing so will result in a deadlock
        std::unique_ptr<AsyncEventSubscription<void, ConnectionEvent>> SubscribeOnConnectionEvent()
        {
            return _connectionEventEmitter->SubscribeAsync();
        }

        bool Connect(std::wstring url)
        {
            std::lock_guard<std::mutex> lock(_mtx);

            if (_connectionState == CONNECTED)
                return false;

            if (!_doConnect)
            {
                _url = url;
                _doConnect = true;
            }
            return true;
        }

        bool Disconnect()
        {
            std::lock_guard<std::mutex> lock(_mtx);

            if (_connectionState == DISCONNECTED)
                return false;

            _doDisconnect = true;
            return true;
        }

        bool Ready()
        {
            std::lock_guard<std::mutex> lock(_mtx);
            return _connectionState == CONNECTED;
        }

        std::optional<GameState> GetGameState()
        {
            std::lock_guard<std::mutex> lock(_mtx);
            return _gameState;
        }

    private:
        void _Runner()
        {
            while (!_stop.load())
            {
                std::unique_lock<std::mutex> lock(_mtx);
                switch (_connectionState)
                {
                case DISCONNECTED:
                {
                    if (_doConnect)
                    {
                        // Connect without hogging the lock
                        lock.unlock();
                        auto result = easywsclient::WebSocket::from_url(wstring_to_string(_url));
                        lock.lock();

                        _client = std::unique_ptr<easywsclient::WebSocket>(result);
                        _doConnect = false;
                        if (_client)
                        {
                            _connectionEventEmitter->InvokeAll(CONNECTION_SUCCESSFUL);
                            _connectionState = CONNECTED;
                        }
                        else
                        {
                            _connectionEventEmitter->InvokeAll(CONNECTION_FAILED);
                        }
                        break;
                    }
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    break;
                }
                case CONNECTED:
                {
                    if (_doDisconnect)
                    {
                        _client->close();
                        lock.unlock();
                        _client->poll();
                        lock.lock();

                        _doDisconnect = false;
                        _connectionEventEmitter->InvokeAll(DISCONNECT_COMPLETED);
                        _connectionState = DISCONNECTED;
                        _client.reset();
                        break;
                    }

                    if (_client->getReadyState() == easywsclient::WebSocket::CLOSED)
                    {
                        _doDisconnect = true;
                        break;
                    }
                    lock.unlock();
                    _client->poll();
                    lock.lock();
                    _client->dispatch([&](const std::string& message) {
                        GameState gameState{};
                        ParseJson(gameState, message);
                        _gameState = gameState;
                    });
                    break;
                }
                }
            }
        }

        enum ConnectionState
        {
            DISCONNECTED,
            CONNECTED
        };

        bool _doConnect = false;
        bool _doDisconnect = false;
        EventEmitter<void, ConnectionEvent> _connectionEventEmitter = EventEmitter<void, ConnectionEvent>(EventEmitterThreadMode::MULTITHREADED);
        ConnectionState _connectionState = ConnectionState::DISCONNECTED;
        std::unique_ptr<easywsclient::WebSocket> _client = nullptr;
        std::wstring _url;
        std::mutex _mtx;

        std::thread _runnerThread;
        std::atomic<bool> _stop{ false };
        std::optional<GameState> _gameState = std::nullopt;
    };
}