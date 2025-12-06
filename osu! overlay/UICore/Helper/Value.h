#pragma once

#include "EventEmitter.h"

#include <functional>
#include <optional>

extern std::atomic<size_t> valueSetCounter;
extern std::atomic<size_t> valueUpdateCounter;

template<typename T>
class Value;

class ValueComputerBase { public: virtual ~ValueComputerBase() {} };

template<typename _Out, typename... _Inp>
class ValueComputer : public ValueComputerBase
{
public:
    template<typename T>
    struct _Input
    {
        EventSubscription<void, T> subscription;
        T lastValue = {};
    };

    Value<_Out>* outputValue = {};
    std::tuple<_Input<_Inp>...> inputs;
    std::function<_Out(_Inp...)> mapper;

    template<size_t... _Idx>
    void BuildInputs(std::integer_sequence<size_t, _Idx...>, Value<_Inp>&... values);
    void UpdateValue();
    _Out Compute(_Input<_Inp>&... inps);
};

template<typename T>
class Value
{
    T _v{};
    std::optional<EventEmitter<void, T>> _emitter;
    std::function<void(T&, const T&)> _setter;
    std::unique_ptr<ValueComputerBase> _valueComputer;
public:

    Value(std::function<void(T&, const T&)> setter = nullptr)
    {
        _setter = setter;
    }
    Value(const T& v, std::function<void(T&, const T&)> setter = nullptr) : Value(setter)
    {
        _v = v;
    }
    void SetSetter(std::function<void(T&, const T&)> setter)
    {
        _setter = setter;
    }

    Value<T>& operator=(const T& v)
    {
        Set(v);
        return *this;
    }
    Value<T>& operator=(const Value<T>& v)
    {
        Set(v.Get());
        return *this;
    }
    void Set(const T& v)
    {
        //valueSetCounter.fetch_add(1);
        if (!(_v == v))
        {
            //valueUpdateCounter.fetch_add(1);
            if (_setter)
                _setter(_v, v);
            else
                _v = v;
            if (_emitter)
                _emitter.value()->InvokeAll(_v);
        }
    }
    // Convenience method to assign a value using operator= of T
    // This is done by creating a copy of T, assigning the value using operator=, and using the resulting modified copy as the value for Value<T>::operator=
    template<typename U>
    void Assign(const U& other)
    {
        T copy = _v;
        copy = other;
        Set(copy);
    }
    operator const T&() const
    {
        return _v;
    }
    const T& Get() const
    {
        return _v;
    }
    T& Get()
    {
        return _v;
    }
    const T* operator->() const
    {
        return &_v;
    }
    T* operator->()
    {
        return &_v;
    }
    // Emits a value change event
    // Should be used after changing the underlying value using -> of Get() instead of assignment, to notify that the value changed
    void NotifyChange()
    {
        if (_emitter)
            _emitter.value()->InvokeAll(_v);
    }

    bool operator==(const T& other) const
    {
        return _v == other;
    }
    bool operator==(const Value<T>& other) const
    {
        return _v == other._v;
    }
    bool operator!=(const T& other) const
    {
        return !(_v == other);
    }
    bool operator!=(const Value<T>& other) const
    {
        return !(_v == other._v);
    }

    template <typename T>
    struct identity
    {
        typedef T type;
    };

    template<class... _Val>
    void ComputedFrom(typename identity<std::function<T(_Val...)>>::type mapper, Value<_Val>&... vals)
    {
        auto computer = std::make_unique<ValueComputer<T, _Val...>>();
        computer->outputValue = this;
        computer->mapper = mapper;
        computer->BuildInputs(std::make_integer_sequence<size_t, sizeof...(vals)>{}, vals...);
        computer->UpdateValue();
        _valueComputer = std::move(computer);
    }

    void ResetComputer()
    {
        _valueComputer.reset();
    }

    [[nodiscard]] auto Subscribe(const std::function<void(T)>& handler)
    {
        if (!_emitter)
            _emitter = EventEmitter<void, T>();
        return _emitter.value()->Subscribe(handler);
    }
};

template<typename _Out, typename... _Inp> template<size_t... _Idx>
void ValueComputer<_Out, _Inp...>::BuildInputs(std::integer_sequence<size_t, _Idx...>, Value<_Inp>&... values)
{
    //([&]{ BuildInput<_Idx>(values); }(), ...);
    ([&] {
        std::get<_Idx>(inputs) = {};
        std::get<_Idx>(inputs).lastValue = (_Inp)values;
        std::get<_Idx>(inputs).subscription = values.Subscribe([=](_Inp newValue) {
            std::get<_Idx>(inputs).lastValue = newValue;
            UpdateValue();
        });
    }(), ...);
}

template<typename _Out, typename... _Inp>
void ValueComputer<_Out, _Inp...>::UpdateValue()
{
    *outputValue = (_Out)std::apply([=](_Input<_Inp>&... inps) { return Compute(inps...); }, inputs);
}

template<typename _Out, typename... _Inp>
_Out ValueComputer<_Out, _Inp...>::Compute(_Input<_Inp>&... inps)
{
    return mapper(inps.lastValue...);
}