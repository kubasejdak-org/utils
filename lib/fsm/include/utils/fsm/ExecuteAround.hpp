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

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace utils::fsm {

template <typename UnderlyingType>
class ExecAround {
    template <typename>
    struct ActionExecutor;

public:
    using Action = std::function<void()>;

    Action preAction;
    Action postAction;
    UnderlyingType underlyingObject;

    template <typename... Args>
    ExecAround(Action preAction, Action postAction, UnderlyingType underlyingObject)
        : preAction(std::move(preAction))
        , postAction(std::move(postAction))
        , underlyingObject(std::move(underlyingObject))
    {}

    template <typename T>
    explicit ExecAround(ExecAround<T>&& other)
        : preAction(std::move(other.preAction))
        , postAction(std::move(other.postAction))
        , underlyingObject(std::move(other.underlyingObject))
    {}

    template <typename T>
    ExecAround& operator=(const ExecAround<T>& other)
    {
        preAction = other.preAction;
        postAction = other.postAction;
        underlyingObject = other.underlyingObject;
        return *this;
    }

    template <typename T>
    ExecAround& operator=(ExecAround<T>&& other)
    {
        preAction = std::move(other.preAction);
        postAction = std::move(other.postAction);
        underlyingObject = std::move(other.underlyingObject);
        return *this;
    }

    auto operator->() { return ActionExecutor<ExecAround>(this); }
    auto operator->() const { return ActionExecutor<const ExecAround>(this); }

private:
    template <typename T>
    struct IsReferenceWrapper : std::false_type {};

    template <typename T>
    struct IsReferenceWrapper<std::reference_wrapper<T>> : std::true_type {};

    template <typename T>
    struct IsSharedPointer : std::false_type {};

    template <typename T>
    struct IsSharedPointer<std::shared_ptr<T>> : std::true_type {};

    template <typename Wrapper>
    struct ActionExecutor {
        Wrapper* wrapper;

        explicit ActionExecutor(Wrapper* wrapper) noexcept
            : wrapper(wrapper)
        {
            if (wrapper->preAction)
                wrapper->preAction();
        }
        ActionExecutor(const ActionExecutor&) = delete;
        ActionExecutor(ActionExecutor&&) noexcept = default;
        ~ActionExecutor()
        {
            if ((wrapper != nullptr) && wrapper->postAction)
                wrapper->postAction();
        }

        ActionExecutor& operator=(const ActionExecutor&) = delete;
        ActionExecutor& operator=(ActionExecutor&&) = delete;

        auto operator->() const noexcept
        {
            if constexpr (IsReferenceWrapper<UnderlyingType>::value || IsSharedPointer<UnderlyingType>::value) {
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

} // namespace utils::fsm
