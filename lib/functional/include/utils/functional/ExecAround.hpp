/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2021, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utils/types/traits.hpp>

#include <functional>
#include <memory>
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

    Action preAction;
    Action postAction;
    UnderlyingType underlyingObject;

    /// Constructor.
    /// @param preAction                Callback to be called before executing UnderlyingType's method.
    /// @param postAction               Callback to be called after executing UnderlyingType's method.
    /// @param underlyingObject         Object to be wrapped with ExecAround mechanism.
    ExecAround(Action preAction, Action postAction, UnderlyingType&& underlyingObject)
        : preAction(std::move(preAction))
        , postAction(std::move(postAction))
        , underlyingObject(std::forward<UnderlyingType>(underlyingObject))
    {}

    /// Copy constructor.
    /// @param other                    ExecAround instance to be copied from.
    ExecAround(const ExecAround<UnderlyingType>& other) noexcept
        : preAction(std::move(other.preAction))
        , postAction(std::move(other.postAction))
        , underlyingObject(std::move(other.underlyingObject))
    {}

    /// Move constructor.
    /// @param other                    ExecAround instance to be moved from.
    ExecAround(ExecAround<UnderlyingType>&& other) noexcept
        : preAction(std::move(other.preAction))
        , postAction(std::move(other.postAction))
        , underlyingObject(std::move(other.underlyingObject))
    {}

    /// Default destructor.
    ~ExecAround() = default;

    /// Copy assignment operator.
    /// @param other                    ExecAround instance to be copied from.
    ExecAround& operator=(const ExecAround<UnderlyingType>& other) // NOLINT
    {
        if (other != *this) {
            preAction = other.preAction;
            postAction = other.postAction;
            underlyingObject = other.underlyingObject;
        }

        return *this;
    }

    /// Move assignment operator.
    /// @param other                    ExecAround instance to be moved from.
    ExecAround& operator=(ExecAround<UnderlyingType>&& other) // NOLINT
    {
        if (other != *this) {
            preAction = std::move(other.preAction);
            postAction = std::move(other.postAction);
            underlyingObject = std::move(other.underlyingObject);
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
                return wrapper->underlyingObject.get();
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
