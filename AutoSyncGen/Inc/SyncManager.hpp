#ifndef AUTOSYNCGEN_SYNCMANAGER
#define AUTOSYNCGEN_SYNCMANAGER

#include "../../AutoSyncGen/Inc/Common.hpp"
#include "../../AutoSyncGen/Inc/SerializationHelper.hpp"
#include "../../AutoSyncGen/Inc/ManagerHelper.hpp"

namespace syn
{
	template<template<typename> class TLFManager, typename... TTypes> class SyncManagerna
	{
		friend struct ManagerHelper;

		public:
			static constexpr ssvu::SizeT typeCount{sizeof...(TTypes)};

			template<typename T> using LFManagerFor = TLFManager<T>;
			template<typename T> using HandleFor = typename LFManagerFor<T>::Handle;
			template<typename T> using HandleMapFor = std::map<ID, HandleFor<T>>;

		private:
			using TplLFManagers = std::tuple<LFManagerFor<TTypes>...>;
			using TplHandleMaps = std::tuple<HandleMapFor<TTypes>...>;
			using TplIDs = ssvu::TplRepeat<ID, typeCount>;

			using ObjBitset = std::bitset<maxObjs>;
			//using TplBitset = ssvu::TplRepeat<ID, typeCount>;

			using MemFnCreate = void(SyncManager<TLFManager, TTypes...>::*)(ID, const ssvj::Val&);
			using MemFnRemove = void(SyncManager<TLFManager, TTypes...>::*)(ID);
			using MemFnUpdate = void(SyncManager<TLFManager, TTypes...>::*)(ID, const ssvj::Val&);

			TplLFManagers lfManagers;
			TplHandleMaps handleMaps;
			TplIDs lastIDs;

			std::array<MemFnCreate, typeCount> funcsCreate;
			std::array<MemFnRemove, typeCount> funcsRemove;
			std::array<MemFnUpdate, typeCount> funcsUpdate;

			std::array<ObjBitset, typeCount> bitsetIDs;

			void testFn(int) { }

			template<typename T> inline auto& getBitsetFor() noexcept { return bitsetIDs[getTypeID<T>()]; }
			template<typename T> inline const auto& getBitsetFor() const noexcept { return bitsetIDs[getTypeID<T>()]; }
			template<typename T> inline bool isPresent(ID mID) const noexcept { return getBitsetFor<T>()[mID]; }
			template<typename T> inline void setPresent(ID mID, bool mX) noexcept { getBitsetFor<T>()[mID] = mX; }

			template<typename T> inline void createImpl(ID mID, const ssvj::Val& mVal)
			{
				SSVU_ASSERT(!isPresent<T>(mID));
				setPresent<T>(mID, true);

				auto& handle(getHandleFor<T>(mID));
				handle = getLFManagerFor<T>().create();

				handle->setFromJson(mVal);

				getHandleMapFor<T>()[mID] = handle;
			}
			template<typename T> inline void removeImpl(ID mID)
			{
				SSVU_ASSERT(isPresent<T>(mID));
				setPresent<T>(mID, false);

				auto& handle(getHandleFor<T>(mID));
				getLFManagerFor<T>().remove(handle);
				handle = getNullHandleFor<T>();

				getHandleMapFor<T>().erase(mID);
			}
			template<typename T> inline void updateImpl(ID mID, const ssvj::Val& mVal)
			{
				SSVU_ASSERT(isPresent<T>(mID));

				auto& handle(getHandleFor<T>(mID));
				getLFManagerFor<T>().update(handle);

				handle->setFromJson(mVal);
			}

		public:
			inline SyncManager()
			{
				ManagerHelper::initManager<SyncManager<TLFManager, TTypes...>, 0, TTypes...>(*this);
			}

			template<typename T> inline static constexpr ID getTypeID() noexcept
			{
				return ssvu::TplIdxOf<T, std::tuple<TTypes...>>::value;
			}

			template<typename T> inline ID getFirstFreeID() noexcept
			{
				constexpr ID typeID{getTypeID<T>()};
				return std::get<typeID>(lastIDs)++;
			}

			template<typename T> inline auto getNullHandleFor() noexcept
			{
				return getLFManagerFor<T>().getNullHandle();
			}

			template<typename T> inline auto& getLFManagerFor() noexcept
			{
				return std::get<LFManagerFor<T>>(lfManagers);
			}
			template<typename T> inline auto& getHandleMapFor() noexcept
			{
				return std::get<HandleMapFor<T>>(handleMaps);
			}
			template<typename T> inline auto& getHandleFor(ID mID) noexcept
			{
				return getHandleMapFor<T>()[mID];
			}
			///template<typename T> inline auto& getLastIDOf() noexcept
			//{
				//return std::get<ssvu::TplIdxOf<T, decltype(handles)>>(lastIDs);
			//}

			template<typename T> inline auto serverCreate(const ssvj::Val& mVal)
			{
				auto createID(getFirstFreeID<T>());
				(this->*funcsCreate[getTypeID<T>()])(createID, mVal);
				return getHandleMapFor<T>()[createID];
			}

			inline auto getChanged(ID mIDType, ID mID)
			{

			}

			inline void onReceivedPacketCreate(ID mIDType, ID mID, const ssvj::Val& mVal)
			{
				(this->*funcsCreate[mIDType])(mID, mVal);
			}

			inline void onReceivedPacketRemove(ID mIDType, ID mID)
			{
				(this->*funcsRemove[mIDType])(mID);
			}

			inline void onReceivedPacketUpdate(ID mIDType, ID mID, const ssvj::Val& mVal)
			{
				(this->*funcsUpdate[mIDType])(mID, mVal);
			}

			struct Diff
			{
				struct DiffTypeData
				{
					std::map<ID, ssvj::Val> toCreate, toUpdate;
					std::vector<ID> toRemove;

					inline auto toJson() const
					{
						using namespace ssvj;

						Val result{Obj{}};

						result["create"] = Obj{};
						result["remove"] = Arr{};
						result["update"] = Obj{};

						auto& jCreate(result["create"]);
						auto& jRemove(result["remove"]);
						auto& jUpdate(result["update"]);

						for(const auto& x : toCreate) jCreate[ssvu::toStr(x.first)] = x.second;
						for(const auto& x : toRemove) jRemove.emplace(x);
						for(const auto& x : toUpdate) jUpdate[ssvu::toStr(x.first)] = x.second;

						return result;
					}
				};

				ssvu::TplRepeat<DiffTypeData, typeCount> diffTypeDatas;

				inline auto toJson()
				{
					using namespace ssvj;

					Val result{Arr{}};
					ssvu::tplFor([this, &result](const auto& mI){ result.emplace(mI.toJson()); }, diffTypeDatas);
					return result;
				}
			};

			inline auto getDiffWith(const SyncManager& mX)
			{
				Diff result;

				ssvu::tplForIdx([this, &result, &mX](auto mIType, auto& mI, auto& mDTD) mutable
				{				
					const auto& myBitset(bitsetIDs[mIType]);
					const auto& otherBitset(mX.bitsetIDs[mIType]);

					auto otherBitsetToCreate((~otherBitset) & myBitset);
					auto otherBitsetToRemove((~myBitset) & otherBitset);
					auto otherBitsetToUpdate(myBitset & otherBitset);

					for(auto i(0u); i < maxObjs; ++i)
					{						
						if(otherBitsetToCreate[i]) mDTD.toCreate[i] = mI[i]->toJsonAll();
						if(otherBitsetToRemove[i]) mDTD.toRemove.emplace_back(i);
						if(otherBitsetToUpdate[i]) mDTD.toUpdate[i] = mI[i]->toJsonChanged();
					}
				}, handleMaps, result.diffTypeDatas);

				return result;
			}

			inline void applyDiff(const Diff& mX)
			{
				ssvu::tplForIdx([this, &mX](auto mIType, auto& mI, auto& mDTD) mutable
				{				
					for(auto i(0u); i < maxObjs; ++i)
					{
						for(const auto& p : mDTD.toCreate) this->onReceivedPacketCreate(mIType, p.first, p.second);
						for(auto i : mDTD.toRemove) this->onReceivedPacketRemove(mIType, i);
						for(const auto& p : mDTD.toUpdate) this->onReceivedPacketUpdate(mIType, p.first, p.second);

						
					}
				}, handleMaps, mX.diffTypeDatas);
			}
	};
}

#endif