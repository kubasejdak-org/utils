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

/// Helper class that provides "InstanceIdType" type alias and "getInstanceId()" method.
/// @tparam T                   Type that should be used as global instance id.
template <typename T>
class Registrable {
public:
    /// Helper type alias for global instance id.
    using InstanceIdType = T;

    /// Constructor.
    /// @param instanceId       Instance id.
    constexpr explicit Registrable(InstanceIdType instanceId)
        : m_cInstanceId(instanceId)
    {}

    /// Copy constructor.
    constexpr Registrable(const Registrable&) = default;

    /// Move constructor.
    constexpr Registrable(Registrable&&) noexcept = default;

    /// Virtual destructor.
    virtual ~Registrable() = default;

    /// Copy assignment operator.
    /// @return Reference to self.
    constexpr Registrable& operator=(const Registrable&) = default;

    /// Move assignment operator.
    /// @return Reference to self.
    constexpr Registrable& operator=(Registrable&&) noexcept = default;

    /// Returns instance id of the current object.
    /// @return Instance id of the current object.
    [[nodiscard]] constexpr InstanceIdType instanceId() const { return m_cInstanceId; }

private:
    const InstanceIdType m_cInstanceId;
};

/// Provides an easy to use way of registering set of global objects of the same type T, accessible from everywhere.
/// @tparam T                   Type of objects, that should be stored within GlobalRegistry.
/// @note In order to create a GlobalRegistry, call GlobalRegistry<T>::init(<objects...>) with the objects that should
///       be stored. Type T must be either either copy-constructible or move-constructible. GlobalRegistry supports
///       move-only types. Ideal usage of the GlobalRegistry<T> is to move (not copy) global objects into registry.
template <typename T>
class GlobalRegistry {
public:
    /// Initializes GlobalRegistry with a given set of T objects.
    /// @tparam Ts              Types, that should be stored within GlobalRegistry.
    /// @param instances        T objects, that should be stored within GlobalRegistry.
    /// @note This method can be called only once for every type T.
    template <typename... Ts>
    static void init(Ts&&... instances)
    {
        static_assert(sizeof...(Ts) > 0);
        static_assert(std::conjunction_v<std::is_constructible<T, Ts>...>);
        assert(m_instances.empty());

        (m_instances.try_emplace(instances.instanceId(), std::make_shared<T>(std::forward<Ts>(instances))), ...);
        assert(sizeof...(Ts) == m_instances.size());
    }

    /// Returns std::shared_ptr with instance of the T type, that is identified with the given id.
    /// @param idx               Index (id) of the instance, that should be returned.
    /// @return std::shared_ptr with instance of the T type, that is identified with the given id.
    static std::shared_ptr<T> get(typename T::InstanceIdType idx)
    {
        if (m_instances.count(idx) == 0)
            return nullptr;

        return m_instances[idx];
    }

    /// Returns size of the GlobalRegistry.
    /// @return Size of the GlobalRegistry.
    static std::size_t size() { return m_instances.size(); }

    /// Clears global registry.
    /// @note After call to this function GlobalRegistry can be initialized once again.
    static void clear() { m_instances.clear(); }

private:
    /// Default constructor.
    GlobalRegistry() = default;

private:
    static inline std::map<typename T::InstanceIdType, std::shared_ptr<T>> m_instances;
};

} // namespace utils
