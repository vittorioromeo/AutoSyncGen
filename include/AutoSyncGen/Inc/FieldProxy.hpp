#ifndef AUTOSYNCGEN_FIELDPROXY
#define AUTOSYNCGEN_FIELDPROXY

#include "../../AutoSyncGen/Inc/Common.hpp"

#define DEFINE_SIMPLE_SYNCFIELDPROXY_MUTABLE_OPERATION(mOp) \
	template<typename T> \
	inline auto& operator mOp (T&& mX) \
	noexcept(noexcept(std::declval<FieldProxy<TI, TObj>>().get() mOp ssvu::fwd<T>(mX))) \
	{ \
		this->syncObj.template setBitAt<TI>(); \
		this->get() mOp ssvu::fwd<T>(mX); \
		return *this; \
	}

#define ENABLEIF_IS_SYNCFIELDPROXY(mType) \
	ssvu::EnableIf<typename IsFieldProxy<ssvu::RemoveAll<mType>>::Type{}>* = nullptr

#define SIMPLE_SYNCFIELDPROXY_OPERATION_TEMPLATE() \
	template<typename T, typename TP, ENABLEIF_IS_SYNCFIELDPROXY(TP)>

#define SIMPLE_SYNCFIELDPROXY_OPERATION_BODY(mOp) \
	noexcept(noexcept(ssvu::fwd<TP>(mP).get() mOp ssvu::fwd<T>(mX))) \
	{ \
		return ssvu::fwd<TP>(mP).get() mOp ssvu::fwd<T>(mX); \
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
			using Type = typename ssvu::RemoveRef<decltype(syncObj)>::template TypeAt<TI>;

			inline FieldProxy(TObj& mSyncObj) noexcept : syncObj{mSyncObj}
			{

			}

			template<typename T> inline auto& operator=(T&& mX)
				noexcept(std::is_nothrow_assignable<Type, T>())
			{
				auto& field(syncObj.template getFieldAt<TI>());
				field = ssvu::fwd<T>(mX);
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
	};

	template<typename T> struct IsFieldProxy
	{
		using Type = std::false_type;
	};

	template<TypeIdx TI, typename TObj> struct IsFieldProxy<FieldProxy<TI, TObj>>
	{
		using Type = std::true_type;
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

#endif