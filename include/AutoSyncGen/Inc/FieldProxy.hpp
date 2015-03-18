#ifndef AUTOSYNCGEN_FIELDPROXY
#define AUTOSYNCGEN_FIELDPROXY

#include "../../AutoSyncGen/Inc/Common.hpp"

#define DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(mOp) \
	template<typename T> \
	inline auto& operator mOp (T&& mX) \
	noexcept(noexcept(std::declval<FieldProxy<TI, TObj>>().get() mOp FWD(mX))) \
	{ \
		this->syncObj.template setBitAt<TI>(); \
		this->get() mOp FWD(mX); \
		return *this; \
	}

#define ENABLEIF_IS_SYNCFIELDPROXY(mType) \
	ssvu::EnableIf<typename IsFieldProxy<ssvu::RmAll<mType>>::Type{}>* = nullptr

#define SIMPLE_SYNCFIELDPROXY_OPERATION_TEMPLATE() \
	template<typename T, typename TP, ENABLEIF_IS_SYNCFIELDPROXY(TP)>

#define SIMPLE_SYNCFIELDPROXY_OPERATION_BODY(mOp) \
	noexcept(noexcept(FWD(mP).get() mOp FWD(mX))) \
	{ \
		return FWD(mP).get() mOp FWD(mX); \
	}

#define DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(mOp) \
	SIMPLE_SYNCFIELDPROXY_OPERATION_TEMPLATE() \
	inline auto operator mOp (TP&& mP, T&& mX) \
	SIMPLE_SYNCFIELDPROXY_OPERATION_BODY(mOp) \
	\
	SIMPLE_SYNCFIELDPROXY_OPERATION_TEMPLATE() \
	inline auto operator mOp (T&& mX, TP&& mP) \
	SIMPLE_SYNCFIELDPROXY_OPERATION_BODY(mOp)

namespace syn
{
	template<TypeIdx TI, typename TObj> class FieldProxy
	{
		private:
			// TODO: instead of storing a pointer/reference, use offsetof and crtp
			// or use inheritance
			TObj& syncObj;

		public:
			using Type = typename ssvu::RmRef<decltype(syncObj)>::template TypeAt<TI>;

			inline FieldProxy(TObj& mSyncObj) noexcept : syncObj{mSyncObj}
			{

			}

			template<typename T> inline auto& operator=(T&& mX)
				noexcept(std::is_nothrow_assignable<Type, T>())
			{
				auto& field(syncObj.template getFieldAt<TI>());
				field = FWD(mX);
				syncObj.template setBitAt<TI>();
				return field;
			}

			DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(+=)
			DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(-=)
			DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(*=)
			DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(/=)
			DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(%=)

			inline auto& get() noexcept { return syncObj.template getFieldAt<TI>(); }
			inline const auto& get() const noexcept { return syncObj.template getFieldAt<TI>(); }

			inline auto& operator->() noexcept { return &get(); }
			inline const auto& operator->() const noexcept { return &get(); }

			inline auto& operator*() noexcept { return get(); }
			inline const auto& operator*() const noexcept { return get(); }
	};

	template<typename T> struct IsFieldProxy
	{
		using Type = ssvu::FalseT;
	};

	template<TypeIdx TI, typename TObj> struct IsFieldProxy<FieldProxy<TI, TObj>>
	{
		using Type = ssvu::TrueT;
	};

	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(+)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(-)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(*)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(/)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(%)

	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(==)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(!=)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(>)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(<)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(>=)
	DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION(<=)
}

#undef DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION
#undef ENABLEIF_IS_SYNCFIELDPROXY
#undef SIMPLE_SYNCFIELDPROXY_OPERATION_TEMPLATE
#undef SIMPLE_SYNCFIELDPROXY_OPERATION_BODY
#undef DEFINE_SIMPLE_SYNCFIELDPROXY_OPERATION

#define SYN_PROXY(mIdx, mName) ProxyAt<mIdx> mName{get<mIdx>()}

#endif
