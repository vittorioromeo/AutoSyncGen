#ifndef AUTOSYNCGEN_FIELDPROXY
#define AUTOSYNCGEN_FIELDPROXY

#include "../../AutoSyncGen/Inc/Common.hpp"

namespace syn
{
    template <TypeIdx TI, typename TObj>
    class FieldProxy
    {
    private:
        TObj& syncObj;

        /// @brief Set the "dirty bit" associated to this field to true.
        inline void setDirtyBit() noexcept { syncObj.template setBitAt<TI>(); }

    public:
        /// @brief Type of the underlying field.
        using Type =
            typename ssvu::RmRef<decltype(syncObj)>::template TypeAt<TI>;

        inline FieldProxy(TObj& mSyncObj) noexcept : syncObj{mSyncObj} {}

        /// @brief Returns a non-const reference to the underlying field, and
        /// sets
        /// the dirty bit.
        inline auto& edit() noexcept
        {
            setDirtyBit();
            return syncObj.template getFieldAt<TI>();
        }

        /// @brief Returns a const reference to the underlying field.
        inline const auto& view() const noexcept
        {
            return syncObj.template getFieldAt<TI>();
        }
    };

    /// @brief `type_traits`-like class that checks whether or not a class is a
    /// `FieldProxy`.
    template <typename>
    struct IsFieldProxy
    {
        using Type = ssvu::FalseT;
    };

    template <TypeIdx TI, typename TObj>
    struct IsFieldProxy<FieldProxy<TI, TObj>>
    {
        using Type = ssvu::TrueT;
    };
}

#define SYN_PROXY(mIdx, mName) \
    ProxyAt<mIdx> mName { get<mIdx>() }

#endif

// TODO: instead of storing a pointer/reference to the syncObj, use offsetof and
// crtp or use inheritance
