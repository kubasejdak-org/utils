/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2020, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <cassert>
#include <map>
#include <memory>
#include <type_traits>

namespace utils {

template <typename T>
class Registrable {
public:
    using InstanceIdType = T;

    constexpr explicit Registrable(InstanceIdType instanceId)
        : m_cInstanceId(instanceId)
    {}
    constexpr Registrable(const Registrable&) = default;
    constexpr Registrable(Registrable&&) noexcept = default;
    virtual ~Registrable() = default;
    constexpr Registrable& operator=(const Registrable&) = default;
    constexpr Registrable& operator=(Registrable&&) noexcept = default;

    [[nodiscard]] constexpr InstanceIdType instanceId() const { return m_cInstanceId; }

private:
    const InstanceIdType m_cInstanceId;
};

template <typename T>
class GlobalRegistry {
public:
    template <typename... Ts>
    static void init(Ts&&... instances)
    {
        static_assert(sizeof...(Ts) > 0);
        static_assert(std::conjunction_v<std::is_constructible<T, Ts>...>);
        assert(m_instances.empty());

        (m_instances.try_emplace(instances.instanceId(), std::make_shared<T>(std::forward<Ts>(instances))), ...);
        assert(sizeof...(Ts) == m_instances.size());
    }

    static std::shared_ptr<T> get(typename T::InstanceIdType idx) { return m_instances[idx]; }
    static std::size_t size() { return m_instances.size(); }
    static void clear() { m_instances.clear(); }

private:
    GlobalRegistry() = default;

private:
    static inline std::map<typename T::InstanceIdType, std::shared_ptr<T>> m_instances;
};

} // namespace utils
