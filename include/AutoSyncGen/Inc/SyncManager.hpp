#ifndef AUTOSYNCGEN_SYNCMANAGER
#define AUTOSYNCGEN_SYNCMANAGER

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
    template <template <typename> class TLFManager, typename... TTypes>
    class SyncManager
    {
        friend struct syn::Impl::ManagerHelper;

    public:
        /// @brief Count of the types managed by this `SyncManager`.
        static constexpr SizeT typeCount{sizeof...(TTypes)};

        /// @brief Lifetime manager for the type `T`.
        template <typename T>
        using LFManagerFor = TLFManager<T>;

        /// @brief Handle for the type `T`.
        template <typename T>
        using HandleFor = typename LFManagerFor<T>::Handle;

        /// @brief Handle map for the type `T`.
        template <typename T>
        using HandleMapFor = std::map<ID, HandleFor<T>>;

        /// @brief Type of this manager.
        using ThisType = SyncManager<TLFManager, TTypes...>;

        /// @brief Type of snapshot.
        using SnapshotType = Impl::Snapshot<ThisType>;

        /// @brief Type of diff.
        using DiffType = Impl::Diff<ThisType>;

        /// @brief Type of bitset that keeps track of alive objects.
        using ObjBitset = std::bitset<maxObjs>;

        // TODO: tuple?
        /// @brief Type of bitset storage per type.
        using BitsetStorage = std::array<ObjBitset, typeCount>;

    private:
        /// @brief Type of lifetime managers tuple.
        using TplLFManagers = ssvu::Tpl<LFManagerFor<TTypes>...>;

        /// @brief Type of handle maps tuple.
        using TplHandleMaps = ssvu::Tpl<HandleMapFor<TTypes>...>;

        /// @brief Type of ID per type tuple.
        using TplIDs = ssvu::TplRepeat<ID, typeCount>;

        /// @brief Type of member function that creates objects.
        using MemFnCreate = void (ThisType::*)(ID, const ssvj::Val&);

        /// @brief Type of member function that removes objects.
        using MemFnRemove = void (ThisType::*)(ID);

        /// @brief Type of member function that updates objects.
        using MemFnUpdate = void (ThisType::*)(ID, const ssvj::Val&);

        /// @brief Tuple containing all the lifetime managers.
        TplLFManagers lfManagers;

        /// @brief Tuple containing the handle maps.
        TplHandleMaps handleMaps;

        /// @brief Tuple containing the last ID for every type.
        TplIDs lastIDs;

        /// @brief Array containing the creation functions per type.
        std::array<MemFnCreate, typeCount> funcsCreate;

        /// @brief Array containing the removal functions per type.
        std::array<MemFnRemove, typeCount> funcsRemove;

        /// @brief Array containing the update functions per type.
        std::array<MemFnUpdate, typeCount> funcsUpdate;

        /// @brief Array contaning the alive/dead bitset for per type.
        BitsetStorage bitsetIDs;

        /// @brief Returns the alive/dead bitset for type `T`.
        template <typename T>
        inline auto& getBitsetFor() noexcept
        {
            return bitsetIDs[getTypeID<T>()];
        }

        /// @brief Returns the alive/dead bitset for type `T`. (const version)
        template <typename T>
        inline const auto& getBitsetFor() const noexcept
        {
            return bitsetIDs[getTypeID<T>()];
        }

        /// @brief Returns true if the object `mID` of type `T` is alive.
        template <typename T>
        inline bool isPresent(ID mID) const noexcept
        {
            return getBitsetFor<T>()[mID];
        }

        /// @brief Sets the alive/dead for the object `mID` of type `T`.
        template <typename T>
        inline void setPresent(ID mID, bool mX) noexcept
        {
            getBitsetFor<T>()[mID] = mX;
        }

        /// @brief Creates an object of type `T` with id `mID` from the json
        /// value
        /// `mVal`.
        template <typename T>
        inline void createImpl(ID mID, const ssvj::Val& mVal)
        {
            // Creating an already-existing object should never happen.
            SSVU_ASSERT(!isPresent<T>(mID));

            // Sets the bit of the new object to alive.
            setPresent<T>(mID, true);

            // Creates an empty handle...
            auto& handle(getHandleFor<T>(mID));

            // ...and sets it to a newly created handle from the lifetime
            // manager.
            handle = getLFManagerFor<T>().create();

            // Initializes the contents of the handle from json.
            handle->setFromJson(mVal);


            // getHandleMapFor<T>()[mID] = handle;
        }
        template <typename T>
        inline void removeImpl(ID mID)
        {
            SSVU_ASSERT(isPresent<T>(mID));
            setPresent<T>(mID, false);

            auto& handle(getHandleFor<T>(mID));
            getLFManagerFor<T>().remove(handle);
            handle = getNullHandleFor<T>();

            // TODO: null handle?
            getHandleMapFor<T>().erase(mID);
        }
        template <typename T>
        inline void updateImpl(ID mID, const ssvj::Val& mVal)
        {
            SSVU_ASSERT(isPresent<T>(mID));

            auto& handle(getHandleFor<T>(mID));
            handle->setFromJson(mVal);
        }

    public:
        inline SyncManager()
        {
            Impl::ManagerHelper::initManager<SyncManager<TLFManager, TTypes...>,
                0, TTypes...>(*this);
        }

        template <typename T>
        inline static constexpr ID getTypeID() noexcept
        {
            return ssvu::getTplIdxOf<T, ssvu::Tpl<TTypes...>>();
        }

        // TODO: ID pool, recycle deleted IDs
        template <typename T>
        inline ID getFirstFreeID() noexcept
        {
            return std::get<getTypeID<T>()>(lastIDs)++;
        }

        template <typename T>
        inline auto getNullHandleFor() noexcept
        {
            return getLFManagerFor<T>().getNullHandle();
        }
        template <typename T>
        inline auto& getLFManagerFor() noexcept
        {
            return std::get<LFManagerFor<T>>(lfManagers);
        }
        template <typename T>
        inline auto& getHandleMapFor() noexcept
        {
            return std::get<HandleMapFor<T>>(handleMaps);
        }

        template <typename T>
        inline auto& getHandleFor(ID mID) noexcept
        {
            return getHandleMapFor<T>()[mID];
        }

        // TODO: rename?
        /*template<typename T> inline auto& serverCreate2()
        {
            T data(FWD(mArgs)...);
            return serverCreate<T>(data.toJsonAll());
        }*/
        template <typename T>
        inline auto serverCreate(const ssvj::Val& mVal)
        {
            auto createID(getFirstFreeID<T>());
            (this->*funcsCreate[getTypeID<T>()])(createID, mVal);
            return getHandleMapFor<T>()[createID];
        }

        inline auto getNullBitsetStorage() { return BitsetStorage{}; }

        inline void onReceivedPacketCreate(
            ID mIDType, ID mID, const ssvj::Val& mVal)
        {
            (this->*funcsCreate[mIDType])(mID, mVal);
        }

        inline void onReceivedPacketRemove(ID mIDType, ID mID)
        {
            (this->*funcsRemove[mIDType])(mID);
        }

        inline void onReceivedPacketUpdate(
            ID mIDType, ID mID, const ssvj::Val& mVal)
        {
            (this->*funcsUpdate[mIDType])(mID, mVal);
        }

        inline void applyDiff(const DiffType& mX)
        {
            ssvu::tplForData(
                [this, &mX](auto mD, auto& mTD) mutable
                {
                    // for(auto i(0u); i < maxObjs; ++i)
                    {
                        // TODO: can be compile-time?
                        for(const auto& p : mTD.toCreate)
                            this->onReceivedPacketCreate(
                                mD.getIdx(), p.first, p.second);
                        for(auto x : mTD.toRemove)
                            this->onReceivedPacketRemove(mD.getIdx(), x);
                        for(const auto& p : mTD.toUpdate)
                            this->onReceivedPacketUpdate(
                                mD.getIdx(), p.first, p.second);
                    }
                },
                mX.typeDatas);
        }

        inline auto getSnapshot()
        {
            SnapshotType result;
            result.bitsetIDs = bitsetIDs;

            ssvu::tplFor(
                [this, &result](auto& mHM, auto& mTD) mutable
                {
                    for(const auto& x : mHM)
                    {
                        mTD.items[x.first] = x.second->toJsonAll();
                    }
                },
                handleMaps, result.typeDatas);

            return result;
        }
    };
}

#endif
