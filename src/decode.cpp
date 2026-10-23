module;

#include <cassert>

module json;

import std;

namespace json
{
	std::optional<ValueType> match_prefix(std::string_view str) noexcept
	{
		if (str.empty()) return std::nullopt;

		const auto front = str.front();

		if (front == '"') return String;
		if (front == 'n') return Null;
		if (front == 't' || front == 'f') return Bool;
		if (front == '[') return Array;
		if (front == '{') return Object;
		if (std::isdigit(front) != 0 || front == '-') return Number;

		return std::nullopt;
	}

	std::expected<std::pair<std::string_view, double>, InternalError> match_number(
		std::string_view str
	) noexcept
	{
		double value;
		const auto [ptr, err] =
			std::from_chars(std::to_address(str.begin()), std::to_address(str.end()), value);

		if (err != std::errc()) return InternalError{.type = ErrorType::InvalidNumber, .iter = str.begin()};

		const auto offset = std::distance(&str.front(), ptr);
		return std::pair(str.substr(offset), value);
	}

	std::expected<std::pair<std::string_view, std::string>, InternalError> match_string(
		std::string_view str
	) noexcept
	{
		if (str.front() != '"') return InternalError{.type = ErrorType::UnknownChar, .iter = str.begin()};

		std::string result;

		auto cursor = std::next(str.begin());
		while (cursor != str.end())
		{
			// Control character
			if (std::bit_cast<std::uint8_t>(*cursor) < 127 && std::iscntrl(*cursor) != 0)
				return InternalError{.type = ErrorType::InvalidChar, .iter = cursor};

			if (*cursor == '\\')  // Escape
			{
				cursor = std::next(cursor);

				if (cursor == str.end()) break;

				switch (*cursor)
				{
				case '"':
					result.push_back('"');
					break;
				case '/':
					result.push_back('/');
					break;
				case '\\':
					result.push_back('\\');
					break;
				case 'n':
					result.push_back('\n');
					break;
				case 'r':
					result.push_back('\r');
					break;
				case 'b':
					result.push_back('\b');
					break;
				case 'f':
					result.push_back('\f');
					break;
				case 't':
					result.push_back('\t');
					break;
				case 'u':
				{
					cursor = std::next(cursor);
					if (std::distance(cursor, str.end()) < 4)
						return InternalError{.type = ErrorType::UnknownStringEscape, .iter = cursor};

					std::uint16_t code_point;
					const auto [ptr, ec] = std::from_chars(
						std::to_address(cursor),
						std::to_address(std::next(cursor, 4)),
						code_point,
						16
					);
					if (ec != std::errc() || ptr != std::to_address(std::next(cursor, 4)))
						return InternalError{.type = ErrorType::UnknownStringEscape, .iter = cursor};

					if (code_point < 0x80)
					{
						result.push_back(std::bit_cast<char>(static_cast<std::uint8_t>(code_point)));
					}
					else if (code_point < 0x800)
					{
						const auto byte0 =
							std::byte(0b11000000) | std::byte(code_point >> 6u) & std::byte(0b11111);

						const auto byte1 =
							std::byte(0b10000000) | std::byte(code_point) & std::byte(0b111111);

						result.push_back(std::bit_cast<char>(byte0));
						result.push_back(std::bit_cast<char>(byte1));
					}
					else if (code_point < 0xd800 || code_point > 0xdfff)
					{
						const auto byte0 =
							std::byte(0b11100000) | std::byte(code_point >> 12u) & std::byte(0b1111);
						const auto byte1 =
							std::byte(0b10000000) | std::byte(code_point >> 6u) & std::byte(0b111111);
						const auto byte2 =
							std::byte(0b10000000) | std::byte(code_point) & std::byte(0b111111);

						result.push_back(std::bit_cast<char>(byte0));
						result.push_back(std::bit_cast<char>(byte1));
						result.push_back(std::bit_cast<char>(byte2));
					}
					else if (code_point < 0xdc00)  // High surrogate
					{
						const std::uint16_t code_point_high = code_point - static_cast<std::uint16_t>(0xd800);
						cursor = std::next(cursor, 4);

						if (std::distance(cursor, str.end()) < 6)
							return InternalError{.type = ErrorType::UnboundedString, .iter = cursor};
						if (*cursor != '\\' || *std::next(cursor) != 'u')
							return InternalError{.type = ErrorType::UnknownStringEscape, .iter = cursor};
						cursor = std::next(cursor, 2);

						std::uint16_t code_point_low;
						const auto [ptr, ec] = std::from_chars(
							std::to_address(cursor),
							std::to_address(std::next(cursor, 4)),
							code_point_low,
							16
						);
						if (ec != std::errc() || ptr != std::to_address(std::next(cursor, 4)))
							return InternalError{.type = ErrorType::UnknownStringEscape, .iter = cursor};
						code_point_low = code_point_low - 0xdc00;

						const std::uint32_t code_point =
							(static_cast<std::uint32_t>(code_point_high << 10u) | code_point_low) + 0x10000;

						const auto byte0 =
							std::byte(0b11110000) | std::byte(code_point >> 18u) & std::byte(0b111);
						const auto byte1 =
							std::byte(0b10000000) | std::byte(code_point >> 12u) & std::byte(0b111111);
						const auto byte2 =
							std::byte(0b10000000) | std::byte(code_point >> 6u) & std::byte(0b111111);
						const auto byte3 =
							std::byte(0b10000000) | std::byte(code_point) & std::byte(0b111111);

						result.push_back(std::bit_cast<char>(byte0));
						result.push_back(std::bit_cast<char>(byte1));
						result.push_back(std::bit_cast<char>(byte2));
						result.push_back(std::bit_cast<char>(byte3));
					}

					cursor = std::next(cursor, 3);

					break;
				}
				default:
					return InternalError{.type = ErrorType::UnknownStringEscape, .iter = cursor};
				}
			}
			else if (*cursor == '"')
			{
				return std::pair(std::string_view(std::next(cursor), str.end()), std::move(result));
			}
			else  // Normal
			{
				result.push_back(*cursor);
			}

			cursor = std::next(cursor);
		}

		return InternalError{.type = ErrorType::UnboundedString, .iter = cursor};
	}

	std::expected<std::string_view, InternalError> match_null(std::string_view str) noexcept
	{
		if (!str.starts_with("null"))
			return InternalError{.type = ErrorType::UnknownChar, .iter = str.begin()};

		return str.substr(4);
	}

	std::expected<std::pair<std::string_view, bool>, InternalError> match_bool(std::string_view str) noexcept
	{
		if (str.starts_with("true")) return std::pair{str.substr(4), true};
		if (str.starts_with("false")) return std::pair{str.substr(5), false};

		return InternalError{.type = ErrorType::UnknownChar, .iter = str.begin()};
	}

	static std::string_view skip_spaces(std::string_view str) noexcept
	{
		const auto next = std::ranges::find_if_not(str, [](char c) {
			return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\b';
		});
		return {next, str.end()};
	}

	namespace
	{
		struct ArrayStackEntry
		{
			std::vector<Value> item;
			bool pending = false;
		};

		struct ObjectStackEntry
		{
			std::map<std::string, Value> item;
			std::optional<std::string> pending;
		};

		struct ParseStackEntry
		{};
	}

	std::expected<Value, Error> decode(std::string_view str) noexcept
	{
		auto target = skip_spaces(str);
		if (target.empty()) return Error{.type = ErrorType::UnboundedString, .offset = 0};

		const auto target_offset = [&target, str] -> std::size_t {
			return std::distance(str.begin(), target.begin());
		};

		std::optional<Value> result;
		std::stack<std::variant<ArrayStackEntry, ObjectStackEntry, ParseStackEntry>> stack;
		stack.emplace(ParseStackEntry());

		const auto set_return_value = [&stack, &result](Value value) {
			assert(!stack.empty());
			stack.pop();

			if (stack.empty())
			{
				result = std::move(value);
				return;
			}
			auto& top = stack.top();

			if (std::holds_alternative<ArrayStackEntry>(top))
			{
				auto& entry = std::get<ArrayStackEntry>(top);
				entry.item.emplace_back(std::move(value));
			}
			else if (std::holds_alternative<ObjectStackEntry>(top))
			{
				auto& entry = std::get<ObjectStackEntry>(top);
				entry.item.emplace(entry.pending.value(), std::move(value));
			}
			else
			{
				assert(false && "Invalid stack");
			}
		};

		while (true)
		{
			target = skip_spaces(target);

			// End of parsing
			if (stack.empty())
			{
				if (!target.empty())
					return Error{
						.type = ErrorType::UnboundedString,
						.offset = target_offset(),
					};

				assert(result.has_value());
				return std::move(*result);
			}

			// Nothing left to parse while expecting something to parse
			if (target.empty())
				return Error{
					.type = ErrorType::UnboundedString,
					.offset = target_offset(),
				};

			auto& top = stack.top();
			if (std::holds_alternative<ArrayStackEntry>(top))
			{
				auto& entry = std::get<ArrayStackEntry>(top);

				if (entry.pending)
				{
					entry.pending = false;

					if (target.front() == ',')
					{
						target = target.substr(1);
						continue;
					}

					if (target.front() == ']')
					{
						target = target.substr(1);
						set_return_value(std::move(entry.item));
						continue;
					}

					return Error{
						.type = ErrorType::UnknownChar,
						.offset = target_offset(),
					};
				}
				else
				{
					entry.pending = true;
					stack.emplace(ParseStackEntry());
					continue;
				}
			}
			else if (std::holds_alternative<ObjectStackEntry>(top))
			{
				auto& entry = std::get<ObjectStackEntry>(top);

				if (entry.pending.has_value())
				{
					entry.pending = std::nullopt;

					if (target.front() == ',')
					{
						target = target.substr(1);
						continue;
					}

					if (target.front() == '}')
					{
						target = target.substr(1);
						set_return_value(std::move(entry.item));
						continue;
					}

					return Error{
						.type = ErrorType::UnknownChar,
						.offset = target_offset(),
					};
				}
				else
				{
					// Match key
					auto str_result = match_string(target);
					if (!str_result) return Error::from(str_result.error(), str.begin());
					auto [next_target, key] = std::move(*str_result);

					// Match colon
					target = skip_spaces(next_target);
					if (target.empty())
						return Error{
							.type = ErrorType::UnboundedString,
							.offset = target_offset(),
						};
					if (target.front() != ':')
						return Error{
							.type = ErrorType::UnknownChar,
							.offset = target_offset(),
						};

					// Push parse request
					target = skip_spaces(target.substr(1));
					entry.pending = std::move(key);
					stack.emplace(ParseStackEntry());
					continue;
				}
			}
			else if (std::holds_alternative<ParseStackEntry>(top))
			{
				const auto type_result = match_prefix(target);
				if (!type_result) return Error{.type = ErrorType::UnknownChar, .offset = 0};

				switch (*type_result)
				{
				case String:
				{
					auto str_result = match_string(target);
					if (!str_result) return Error::from(str_result.error(), str.begin());
					auto [next_target, str] = std::move(*str_result);

					set_return_value(str);
					target = next_target;
					break;
				}
				case Number:
				{
					auto number_result = match_number(target);
					if (!number_result) return Error::from(number_result.error(), str.begin());
					const auto [next_target, number] = *number_result;

					set_return_value(number);
					target = next_target;
					break;
				}
				case Null:
				{
					auto null_result = match_null(target);
					if (!null_result) return Error::from(null_result.error(), str.begin());

					set_return_value(nullptr);
					target = *null_result;
					break;
				}
				case Object:
				{
					target = skip_spaces(target.substr(1));
					if (target.empty())
						return Error{.type = ErrorType::UnboundedString, .offset = target_offset()};

					if (target.front() == '}')
					{
						set_return_value(std::map<std::string, Value>());
						target = target.substr(1);
					}
					else
					{
						stack.pop();
						stack.emplace(ObjectStackEntry());
					}
					break;
				}
				case Array:
				{
					target = skip_spaces(target.substr(1));
					if (target.empty())
						return Error{.type = ErrorType::UnboundedString, .offset = target_offset()};

					if (target.front() == ']')
					{
						set_return_value(std::vector<Value>());
						target = target.substr(1);
					}
					else
					{
						stack.pop();
						stack.emplace(ArrayStackEntry());
					}
					break;
				}
				case Bool:
				{
					auto bool_result = match_bool(target);
					if (!bool_result) return Error::from(bool_result.error(), str.begin());
					const auto [next_target, flag] = *bool_result;

					set_return_value(flag);
					target = next_target;
					break;
				}
				default:
					std::unreachable();
				}
			}
			else
			{
				std::unreachable();
			}
		}
	}
}
