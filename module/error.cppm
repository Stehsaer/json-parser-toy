export module json:error;

import std;

namespace json
{
	export enum class ErrorType
	{
		UnknownChar,
		InvalidNumber,
		UnknownStringEscape,
		UnboundedString,
		InvalidType,
		InvalidChar,
	};

	struct InternalError
	{
		ErrorType type;
		std::string_view::const_iterator iter;

		template <typename T>
		constexpr operator std::expected<T, InternalError>() const noexcept
		{
			return std::unexpected(*this);
		}
	};

	export struct Error
	{
		ErrorType type;
		std::size_t offset;

		static Error from(InternalError err, std::string_view::const_iterator begin) noexcept
		{
			return {
				.type = err.type,
				.offset = static_cast<std::size_t>(std::distance(begin, err.iter)),
			};
		}

		template <typename T>
		constexpr operator std::expected<T, Error>() const noexcept
		{
			return std::unexpected(*this);
		}
	};
}
