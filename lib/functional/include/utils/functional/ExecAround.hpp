/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2021 Kuba Sejdak (kuba.sejdak@gmail.com)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "utils/types/traits.hpp"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace utils::functional {

/// Helper class allowing to wrap UnderlyingType in a way, that every call to UnderlyingType's method is surrounded by
/// calls to preAction callback and postAction callback. This way, we can specify what should happen before and after
/// executing method of some class without changing code of that class. At the same time this behavior is transparent
/// to the client's code.
/// @tparam UnderlyingType              Type to be wrapped with preAction call and postAction call.
template <typename UnderlyingType>
class ExecAround {
    template <typename>
    struct ActionExecutor;

public:
    /// Alias for function that implements preAction and postAction callbacks.
    using Action = std::function<void()>;

    UnderlyingType underlyingObject;
    Action preAction;
    Action postAction;

    /// Default constructor.
    ExecAround() = default;

    /// Constructor.
    /// @tparam T                       Type to be wrapped with preAction call and postAction call.
    /// @param underlyingObject         Object to be wrapped with ExecAround mechanism.
    /// @param preAction                Callback to be called before executing UnderlyingType's method.
    /// @param postAction               Callback to be called after executing UnderlyingType's method.
    template <typename T>
    requires std::same_as<std::decay_t<T>, UnderlyingType>
    explicit ExecAround(T&& underlyingObject, Action preAction, Action postAction)
        : underlyingObject(std::forward<T>(underlyingObject))
        , preAction(std::move(preAction))
        , postAction(std::move(postAction))
    {}

    /// Copy constructor.
    /// @param other                    ExecAround instance to be copied from.
    ExecAround(const ExecAround<UnderlyingType>& other) noexcept
        : underlyingObject(other.underlyingObject)
        , preAction(other.preAction)
        , postAction(other.postAction)
    {}

    /// Move constructor.
    /// @param other                    ExecAround instance to be moved from.
    ExecAround(ExecAround<UnderlyingType>&& other) noexcept
        : underlyingObject(std::move(other.underlyingObject))
        , preAction(std::move(other.preAction))
        , postAction(std::move(other.postAction))
    {}

    /// Default destructor.
    ~ExecAround() = default;

    /// Copy assignment operator.
    /// @param other                    ExecAround instance to be copied from.
    ExecAround& operator=(const ExecAround<UnderlyingType>& other) noexcept
    {
        if (&other != this) {
            underlyingObject = other.underlyingObject;
            preAction = other.preAction;
            postAction = other.postAction;
        }

        return *this;
    }

    /// Move assignment operator.
    /// @param other                    ExecAround instance to be moved from.
    ExecAround& operator=(ExecAround<UnderlyingType>&& other)
    {
        if (&other != this) {
            underlyingObject = std::move(other.underlyingObject);
            preAction = std::move(other.preAction);
            postAction = std::move(other.postAction);
        }

        return *this;
    }

    /// Dereference operator.
    /// @return Object that will provide access to the underlying object through dereference operator and will call
    ///         preAction and postAction callback from ExecAround object.
    auto operator->() { return ActionExecutor<ExecAround>(this); }

    /// Dereference operator.
    /// @return Object that will provide access to the underlying object through dereference operator and will call
    ///         preAction and postAction callback from ExecAround object.
    auto operator->() const { return ActionExecutor<const ExecAround>(this); }

private:
    /// Helper type for performing transparent execution of preAction and postAction methods.
    /// @tparam Wrapper                 Type on which preAction and postAction should be called.
    template <typename Wrapper>
    struct ActionExecutor {
        Wrapper* wrapper;

        /// Constructor. Calls preAction callback on wrapped object.
        /// @param wrapper              ExecAround object on which preAction and postAction methods will be called.
        explicit ActionExecutor(Wrapper* wrapper) noexcept
            : wrapper(wrapper)
        {
            if (wrapper->preAction)
                wrapper->preAction();
        }

        /// Copy constructor.
        /// @note This constructor is deleted, because ActionExecutor is not meant to be copy-constructed.
        ActionExecutor(const ActionExecutor&) = delete;

        /// Move constructor.
        /// @note This constructor is deleted, because ActionExecutor is not meant to be move-constructed.
        ActionExecutor(ActionExecutor&&) noexcept = default;

        /// Destructor. Calls postAction callback on wrapped object.
        ~ActionExecutor()
        {
            if ((wrapper != nullptr) && wrapper->postAction)
                wrapper->postAction();
        }

        /// Copy assignment operator.
        /// @return Reference to self.
        /// @note This operator is deleted, because ActionExecutor is not meant to be copy-assigned.
        ActionExecutor& operator=(const ActionExecutor&) = delete;

        /// Move assignment operator.
        /// @return Reference to self.
        /// @note This operator is deleted, because ActionExecutor is not meant to be move-assigned.
        ActionExecutor& operator=(ActionExecutor&&) = delete;

        /// Dereference operator.
        /// @return Underlying user-defined object.
        auto operator->() const noexcept
        {
            if constexpr (types::IsReferenceWrapper<UnderlyingType>::value) { // NOLINT
                using T = typename UnderlyingType::type;
                if constexpr (std::is_pointer_v<T> || types::IsSharedPointer<T>::value)
                    return wrapper->underlyingObject.get();
                else
                    return &wrapper->underlyingObject.get();
            }
            else if constexpr (types::IsSharedPointer<UnderlyingType>::value) {
                return wrapper->underlyingObject.get();
            }
            else if constexpr (std::is_pointer_v<UnderlyingType>) {
                return wrapper->underlyingObject;
            }
            else
                return &wrapper->underlyingObject;
        }
    };
};

} // namespace utils::functional
