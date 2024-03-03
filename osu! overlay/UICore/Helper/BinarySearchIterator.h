#pragma once

//class BinarySearchIterator
//{
//    int _valueCount;
//    int _currentValue;
//    int _lowerBound;
//    int _upperBound;
//
//public:
//    BinarySearchIterator(int valueCount)
//    {
//        _valueCount = valueCount;
//        _currentValue = valueCount / 2;
//        _lowerBound = 0;
//        _upperBound = valueCount;
//    }
//
//    int IterateLower()
//    {
//        if (!Completed())
//        {
//            _upperBound = _currentValue;
//            _currentValue = (_currentValue + _lowerBound) / 2;
//        }
//        return _currentValue;
//    }
//
//    int IterateHigher()
//    {
//        if (!Completed())
//        {
//            _lowerBound = _currentValue + 1;
//            _currentValue = (_currentValue + _upperBound) / 2;
//        }
//        return _currentValue;
//    }
//
//    bool Completed() const
//    {
//        return _upperBound <= _lowerBound + 1;
//    }
//
//    int CurrentValue() const
//    {
//        return _currentValue;
//    }
//};
//
//template<typename T>
//class FloatBinarySearchIterator
//{
//    T _currentValue;
//    T _lowerBound;
//    T _upperBound;
//    int _depth;
//
//public:
//    FloatBinarySearchIterator(T lowerBound, T upperBound)
//    {
//        _lowerBound = lowerBound;
//        _upperBound = upperBound;
//        _currentValue = (lowerBound + upperBound) / 2;
//        _depth = 1;
//    }
//
//    T IterateLower()
//    {
//        _depth++;
//        _upperBound = _currentValue;
//        _currentValue = (_currentValue + _lowerBound) / 2;
//        return _currentValue;
//    }
//
//    T IterateHigher()
//    {
//        _depth++;
//        _lowerBound = _currentValue;
//        _currentValue = (_currentValue + _upperBound) / 2;
//        return _currentValue;
//    }
//
//    int CurrentDepth() const
//    {
//        return _depth;
//    }
//
//    T CurrentValue() const
//    {
//        return _currentValue;
//    }
//};