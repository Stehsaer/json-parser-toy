export module json:decode;

import std;
import :data;
import :error;

namespace json
{
	[[nodiscard]]
	std::optional<ValueType> match_prefix(std::string_view str) noexcept;

	[[nodiscard]]
	std::expected<std::pair<std::string_view, double>, InternalError> match_number(
		std::string_view str
	) noexcept;

	[[nodiscard]]
	std::expected<std::pair<std::string_view, std::string>, InternalError> match_string(
		std::string_view str
	) noexcept;

	[[nodiscard]]
	std::expected<std::string_view, InternalError> match_null(std::string_view str) noexcept;

	[[nodiscard]]
	std::expected<std::pair<std::string_view, bool>, InternalError> match_bool(std::string_view str) noexcept;

	export [[nodiscard]]
	std::expected<Value, Error> decode(std::string_view str) noexcept;
}
