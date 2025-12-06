#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <optional>

namespace streams
{
    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamPart
    {
    public:
        // The boolean parameter indicates whether the value can modified
        virtual std::optional<std::pair<_CurT*, bool>> EvaluateItem() { return std::nullopt; }
    };

    template<class _CurT, class _It>
    class StreamSource : public StreamPart<_CurT>
    {
        _It _curIt;
        _It _endIt;

    public:
        StreamSource(_It curIt, _It endIt)
            : _curIt(curIt), _endIt(endIt)
        {}

        std::optional<std::pair<_CurT*, bool>> EvaluateItem()
        {
            if (_curIt != _endIt)
            {
                _CurT* item = const_cast<_CurT*>(&(*_curIt));
                _curIt++;
                return std::pair<_CurT*, bool>(item, false);
            }
            else
            {
                return std::nullopt;
            }
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamMapper : public StreamPart<_CurT, _PrevT, _OtherT...>
    {
        std::function<_CurT(const _PrevT&)> _mapper;
        std::shared_ptr<StreamPart<_PrevT, _OtherT...>> _previousStream;
        std::optional<_CurT> _lastMappedValue;

    public:
        StreamMapper(std::function<_CurT(const _PrevT&)> mapper, std::shared_ptr<StreamPart<_PrevT, _OtherT...>> previousStream)
            : _mapper(mapper), _previousStream(previousStream)
        {}

        std::optional<std::pair<_CurT*, bool>> EvaluateItem()
        {
            std::optional<std::pair<_PrevT*, bool>> previousItem = _previousStream->EvaluateItem();
            if (!previousItem)
                return std::nullopt;

            _lastMappedValue = _mapper(*(previousItem.value().first));
            return std::pair<_CurT*, bool>(&(_lastMappedValue.value()), true);
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamFilter : public StreamPart<_CurT, _PrevT, _OtherT...>
    {
        std::function<bool(const _PrevT&)> _filter;
        std::shared_ptr<StreamPart<_PrevT, _OtherT...>> _previousStream;

    public:
        StreamFilter(std::function<bool(const _PrevT&)> filter, std::shared_ptr<StreamPart<_PrevT, _OtherT...>> previousStream)
            : _filter(filter), _previousStream(previousStream)
        {}

        std::optional<std::pair<_CurT*, bool>> EvaluateItem()
        {
            std::optional<std::pair<_PrevT*, bool>> previousItem;
            while (previousItem = _previousStream->EvaluateItem())
            {
                if (_filter(*(previousItem.value().first)))
                    return previousItem;
            }
            return std::nullopt;
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamSort : public StreamPart<_CurT, _PrevT, _OtherT...>
    {
        std::shared_ptr<StreamPart<_PrevT, _OtherT...>> _previousStream;
        std::function<bool(const _CurT&, const _CurT&)> _comparator;

        std::vector<_CurT> _outputVector;
        bool _sorted = false;
        typename std::vector<_CurT>::iterator _curIt;

    public:
        StreamSort(std::function<bool(const _CurT&, const _CurT&)> comparator, std::shared_ptr<StreamPart<_PrevT, _OtherT...>> previousStream)
            : _comparator(comparator), _previousStream(previousStream)
        {}

        std::optional<std::pair<_CurT&, bool>> EvaluateItem()
        {
            if (!_sorted)
            {
                // Collect and sort all items from previous steps
                std::optional<std::pair<_PrevT*, bool>> previousItem;
                while (previousItem = _previousStream->EvaluateItem())
                {
                    if (previousItem.value().second)
                        _outputVector.push_back(std::move(*(previousItem.value().first)));
                    else
                        _outputVector.push_back(*(previousItem.value().first));
                }
                std::sort(_outputVector.begin(), _outputVector.end(), _comparator);
                _curIt = _outputVector.begin();
                _sorted = true;
            }

            if (_curIt != _outputVector.end())
            {
                _CurT* item = &(*_curIt);
                _curIt++;
                return std::pair<_CurT*, bool>(item, true);
            }
            else
            {
                return std::nullopt;
            }
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamSkip : public StreamPart<_CurT, _PrevT, _OtherT...>
    {
        int _start;
        int _count;
        int _currentItem;
        std::shared_ptr<StreamPart<_PrevT, _OtherT...>> _previousStream;

    public:
        StreamSkip(int start, int count, std::shared_ptr<StreamPart<_PrevT, _OtherT...>> previousStream)
            : _start(start), _count(count), _currentItem(0), _previousStream(previousStream)
        {}

        std::optional<std::pair<_CurT*, bool>> EvaluateItem()
        {
            while (_currentItem >= _start && _currentItem < _start + _count)
            {
                _previousStream->EvaluateItem();
                _currentItem++;
            }
            _currentItem++;
            return _previousStream->EvaluateItem();
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamLimit : public StreamPart<_CurT, _PrevT, _OtherT...>
    {
        int _count;
        int _taken;
        std::shared_ptr<StreamPart<_PrevT, _OtherT...>> _previousStream;

    public:
        StreamLimit(int count, std::shared_ptr<StreamPart<_PrevT, _OtherT...>> previousStream)
            : _count(count), _taken(0), _previousStream(previousStream)
        {}

        std::optional<std::pair<_CurT*, bool>> EvaluateItem()
        {
            if (_taken >= _count)
                return std::nullopt;

            _taken++;
            return _previousStream->EvaluateItem();
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class StreamCollector : public StreamPart<_CurT, _PrevT, _OtherT...>
    {
        std::shared_ptr<StreamPart<_PrevT, _OtherT...>> _previousStream;

    public:
        StreamCollector(std::shared_ptr<StreamPart<_PrevT, _OtherT...>> previousStream)
            : _previousStream(previousStream)
        {}

        std::vector<_CurT> Evaluate()
        {
            std::vector<_CurT> result;
            std::optional<std::pair<_PrevT*, bool>> previousItem;
            while (previousItem = _previousStream->EvaluateItem())
            {
                if (previousItem.value().second)
                    result.push_back(std::move(*(previousItem.value().first)));
                else
                    result.push_back(*(previousItem.value().first));
            }
            return result;
        }
    };

    template<class _CurT, class _PrevT = int, class... _OtherT>
    class Stream
    {
        template<class _Container>
        friend Stream<typename _Container::value_type> From(const _Container&);
        friend class Stream<_PrevT, _OtherT>; // Give access to previous stream

        std::shared_ptr<StreamPart<_CurT, _PrevT, _OtherT...>> _currentStream;

        Stream(std::shared_ptr<StreamPart<_CurT, _PrevT, _OtherT...>> stream)
            : _currentStream(stream) {}

    public:
        template<class _NewT>
        Stream<_NewT, _CurT, _PrevT, _OtherT...> Map(std::function<_NewT(const _CurT&)> mapper)
        {
            auto nextStream = std::make_shared<StreamMapper<_NewT, _CurT, _PrevT, _OtherT...>>(mapper, _currentStream);
            return Stream<_NewT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> Filter(std::function<bool(const _CurT&)> filter)
        {
            auto nextStream = std::make_shared<StreamFilter<_CurT, _CurT, _PrevT, _OtherT...>>(filter, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> SortAsc()
        {
            auto nextStream = std::make_shared<StreamSort<_CurT, _CurT, _PrevT, _OtherT...>>([](const _CurT& left, const _CurT& right) { return left < right; }, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> SortDesc()
        {
            auto nextStream = std::make_shared<StreamSort<_CurT, _CurT, _PrevT, _OtherT...>>([](const _CurT& left, const _CurT& right) { return left > right; }, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> Sort(std::function<bool(const _CurT&, const _CurT&)> comparator)
        {
            auto nextStream = std::make_shared<StreamSort<_CurT, _CurT, _PrevT, _OtherT...>>(comparator, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> Skip(int count)
        {
            auto nextStream = std::make_shared<StreamSkip<_CurT, _CurT, _PrevT, _OtherT...>>(0, count, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> SkipRange(int offset, int count)
        {
            auto nextStream = std::make_shared<StreamSkip<_CurT, _CurT, _PrevT, _OtherT...>>(offset, count, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        Stream<_CurT, _CurT, _PrevT, _OtherT...> Limit(int count)
        {
            auto nextStream = std::make_shared<StreamLimit<_CurT, _CurT, _PrevT, _OtherT...>>(count, _currentStream);
            return Stream<_CurT, _CurT, _PrevT, _OtherT...>(nextStream);
        }

        std::vector<_CurT> ToVector()
        {
            std::vector<_CurT> result;
            std::optional<std::pair<_CurT*, bool>> item;
            while (item = _currentStream->EvaluateItem())
            {
                if (item.value().second)
                    result.push_back(std::move(*(item.value().first)));
                else
                    result.push_back(*(item.value().first));
            }
            return result;
        }

        std::optional<_CurT> FindFirst()
        {
            std::optional<std::pair<_CurT*, bool>> item = _currentStream->EvaluateItem();
            if (item)
            {
                if (item.value().second)
                    return std::move(*(item.value().first));
                else
                    return *(item.value().first);
            }
            else
            {
                return std::nullopt;
            }
        }

        bool AllMatch(std::function<bool(const _CurT&)> predicate)
        {
            std::optional<std::pair<_CurT*, bool>> item;
            while (item = _currentStream->EvaluateItem())
            {
                if (!predicate(*(item.value().first)))
                    return false;
            }
            return true;
        }

        bool AnyMatch(std::function<bool(const _CurT&)> predicate)
        {
            std::optional<std::pair<_CurT*, bool>> item;
            while (item = _currentStream->EvaluateItem())
            {
                if (predicate(*(item.value().first)))
                    return true;
            }
            return false;
        }

        bool NoneMatch(std::function<bool(const _CurT&)> predicate)
        {
            std::optional<std::pair<_CurT*, bool>> item;
            while (item = _currentStream->EvaluateItem())
            {
                if (predicate(*(item.value().first)))
                    return false;
            }
            return true;
        }

        size_t Count()
        {
            size_t count = 0;
            while (_currentStream->EvaluateItem())
                count++;
            return count;
        }

        _CurT Sum()
        {
            _CurT identity = 0;
            std::optional<std::pair<_CurT*, bool>> item;
            while (item = _currentStream->EvaluateItem())
                identity = identity + *(item.value().first);
            return identity;
        }

        _CurT Sum(_CurT identity)
        {
            std::optional<std::pair<_CurT&, bool>> item;
            while (item = _currentStream->EvaluateItem())
                identity = identity + *(item.value().first);
            return identity;
        }
    };

    template<class _Container>
    Stream<typename _Container::value_type> From(const _Container& source)
    {
        using ElemType = typename _Container::value_type;
        using IteratorType = typename _Container::const_iterator;
        return Stream<ElemType>(std::make_shared<StreamSource<ElemType, IteratorType>>(source.cbegin(), source.cend()));
    }
}