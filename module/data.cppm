export module json:data;

import std;

namespace json
{
	export enum ValueType
	{
		String,
		Number,
		Null,
		Object,
		Array,
		Bool,
	};

	export struct Value;

	namespace detail
	{
		template <ValueType Type>
		struct ValueBaseTypeImpl;

		template <>
		struct ValueBaseTypeImpl<String>
		{
			using Type = std::string;
		};

		template <>
		struct ValueBaseTypeImpl<Number>
		{
			using Type = double;
		};

		template <>
		struct ValueBaseTypeImpl<Object>
		{
			using Type = std::map<std::string, Value>;
		};

		template <>
		struct ValueBaseTypeImpl<Array>
		{
			using Type = std::vector<Value>;
		};

		template <>
		struct ValueBaseTypeImpl<Bool>
		{
			using Type = bool;
		};
	}

	export {
		template <ValueType Type>
		using ValueBaseType = detail::ValueBaseTypeImpl<Type>::Type;
	}

	using Variant = std::variant<
		std::nullptr_t,
		ValueBaseType<Number>,
		ValueBaseType<String>,
		ValueBaseType<Object>,
		ValueBaseType<Array>,
		ValueBaseType<Bool>
	>;

	export struct Value : Variant
	{
		using Variant::variant;
		using Variant::operator=;

		[[nodiscard]]
		ValueType type() const noexcept
		{
			switch (index())
			{
			case 0:
				return Null;
			case 1:
				return Number;
			case 2:
				return String;
			case 3:
				return Object;
			case 4:
				return Array;
			case 5:
				return Bool;
			default:
				std::unreachable();
			}
		}

		template <ValueType Type>
		[[nodiscard]]
		const ValueBaseType<Type>& get() const noexcept
		{
			return std::get<ValueBaseType<Type>>(*this);
		}
	};
}
