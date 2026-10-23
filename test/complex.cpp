// Tests adapted from JSONTestSuite, https://github.com/nst/JSONTestSuite
// (commit 1ef36fa01286573e846ac449e8683f8833c5b26a).
//
// Only the y_* (must accept) and n_* (must reject) cases from test_parsing are
// included here. The inputs and the expected values are hardcoded into this
// file; the tests do not read any files at runtime. Valid UTF-8 characters use
// C++ \u and \U escapes; octal escapes are only used for raw bytes that are
// not valid UTF-8, and for NUL and other control bytes.
//
// Expected values for the y_* cases were derived with Python's json module.
// Duplicate object member names are the one exception: RFC 8259 leaves which
// value wins unspecified, so that case accepts either value.

#include <gtest/gtest.h>

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import json;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
	[[nodiscard]]
	json::Value jnum(double value)
	{
		return value;
	}

	[[nodiscard]]
	json::Value jstr(std::string value)
	{
		return std::move(value);
	}

	[[nodiscard]]
	json::Value jarr(std::vector<json::Value> values)
	{
		return std::move(values);
	}

	[[nodiscard]]
	json::Value jobj(std::map<std::string, json::Value> values)
	{
		return std::move(values);
	}

	[[nodiscard]]
	json::Value jnull()
	{
		return nullptr;
	}

	[[nodiscard]]
	json::Value jbool(bool value)
	{
		return value;
	}

	[[nodiscard]]
	const char* type_name(json::ValueType type) noexcept
	{
		switch (type)
		{
		case json::String:
			return "String";
		case json::Number:
			return "Number";
		case json::Null:
			return "Null";
		case json::Object:
			return "Object";
		case json::Array:
			return "Array";
		case json::Bool:
			return "Bool";
		}

		return "Unknown";
	}

	void expect_values_equal(
		const json::Value& actual,
		const json::Value& expected,
		const std::string& path = "$"
	)
	{
		ASSERT_EQ(actual.type(), expected.type())
			<< "expected "
			<< type_name(expected.type())
			<< ", got "
			<< type_name(actual.type())
			<< " at "
			<< path;

		switch (expected.type())
		{
		case json::Null:
			return;
		case json::Bool:
			EXPECT_EQ(actual.get<json::Bool>(), expected.get<json::Bool>()) << "at " << path;
			return;
		case json::Number:
			EXPECT_DOUBLE_EQ(actual.get<json::Number>(), expected.get<json::Number>()) << "at " << path;
			return;
		case json::String:
			EXPECT_EQ(actual.get<json::String>(), expected.get<json::String>()) << "at " << path;
			return;
		case json::Array:
		{
			const auto& actual_array = actual.get<json::Array>();
			const auto& expected_array = expected.get<json::Array>();
			ASSERT_EQ(actual_array.size(), expected_array.size()) << "at " << path;
			for (std::size_t i = 0; i < expected_array.size(); ++i)
			{
				expect_values_equal(actual_array[i], expected_array[i], path + "[" + std::to_string(i) + "]");
			}
			return;
		}
		case json::Object:
		{
			const auto& actual_object = actual.get<json::Object>();
			const auto& expected_object = expected.get<json::Object>();
			ASSERT_EQ(actual_object.size(), expected_object.size()) << "at " << path;
			for (const auto& [key, value] : expected_object)
			{
				ASSERT_TRUE(actual_object.contains(key)) << "missing key at " << path << "." << key;
				expect_values_equal(actual_object.at(key), value, path + "." + key);
			}
			return;
		}
		}
	}
}

// ===== JSONTestSuite must-accept cases (y_*.json) =====

// y_array_arraysWithSpaces.json
TEST(Complex, y_array_arraysWithSpaces)  // NOLINT
{
	const auto result = json::decode("[[]   ]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jarr({})}));
}
// y_array_empty-string.json
TEST(Complex, y_array_emptyminusstring)  // NOLINT
{
	const auto result = json::decode("[\"\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr(""s)}));
}
// y_array_empty.json
TEST(Complex, y_array_empty)  // NOLINT
{
	const auto result = json::decode("[]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({}));
}
// y_array_ending_with_newline.json
TEST(Complex, y_array_ending_with_newline)  // NOLINT
{
	const auto result = json::decode("[\"a\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("a"s)}));
}
// y_array_false.json
TEST(Complex, y_array_false)  // NOLINT
{
	const auto result = json::decode("[false]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jbool(false)}));
}
// y_array_heterogeneous.json
TEST(Complex, y_array_heterogeneous)  // NOLINT
{
	const auto result = json::decode("[null, 1, \"1\", {}]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnull(), jnum(1.0), jstr("1"s), jobj({})}));
}
// y_array_null.json
TEST(Complex, y_array_null)  // NOLINT
{
	const auto result = json::decode("[null]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnull()}));
}
// y_array_with_1_and_newline.json
TEST(Complex, y_array_with_1_and_newline)  // NOLINT
{
	const auto result = json::decode("[1\n]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1.0)}));
}
// y_array_with_leading_space.json
TEST(Complex, y_array_with_leading_space)  // NOLINT
{
	const auto result = json::decode(" [1]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1.0)}));
}
// y_array_with_several_null.json
TEST(Complex, y_array_with_several_null)  // NOLINT
{
	const auto result = json::decode("[1,null,null,null,2]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1.0), jnull(), jnull(), jnull(), jnum(2.0)}));
}
// y_array_with_trailing_space.json
TEST(Complex, y_array_with_trailing_space)  // NOLINT
{
	const auto result = json::decode("[2] "sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(2.0)}));
}
// y_number.json
TEST(Complex, y_number)  // NOLINT
{
	const auto result = json::decode("[123e65]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1.23e+67)}));
}
// y_number_0e+1.json
TEST(Complex, y_number_0eplus1)  // NOLINT
{
	const auto result = json::decode("[0e+1]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(0.0)}));
}
// y_number_0e1.json
TEST(Complex, y_number_0e1)  // NOLINT
{
	const auto result = json::decode("[0e1]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(0.0)}));
}
// y_number_after_space.json
TEST(Complex, y_number_after_space)  // NOLINT
{
	const auto result = json::decode("[ 4]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(4.0)}));
}
// y_number_double_close_to_zero.json
TEST(Complex, y_number_double_close_to_zero)  // NOLINT
{
	const auto result = json::decode(
		"[-0.000000000000000000000000000000000000000000000000000000000000000000000000000001]\n"sv
	);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(-1e-78)}));
}
// y_number_int_with_exp.json
TEST(Complex, y_number_int_with_exp)  // NOLINT
{
	const auto result = json::decode("[20e1]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(200.0)}));
}
// y_number_minus_zero.json
TEST(Complex, y_number_minus_zero)  // NOLINT
{
	const auto result = json::decode("[-0]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(0.0)}));
}
// y_number_negative_int.json
TEST(Complex, y_number_negative_int)  // NOLINT
{
	const auto result = json::decode("[-123]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(-123.0)}));
}
// y_number_negative_one.json
TEST(Complex, y_number_negative_one)  // NOLINT
{
	const auto result = json::decode("[-1]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(-1.0)}));
}
// y_number_negative_zero.json
TEST(Complex, y_number_negative_zero)  // NOLINT
{
	const auto result = json::decode("[-0]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(0.0)}));
}
// y_number_real_capital_e.json
TEST(Complex, y_number_real_capital_e)  // NOLINT
{
	const auto result = json::decode("[1E22]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1e+22)}));
}
// y_number_real_capital_e_neg_exp.json
TEST(Complex, y_number_real_capital_e_neg_exp)  // NOLINT
{
	const auto result = json::decode("[1E-2]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(0.01)}));
}
// y_number_real_capital_e_pos_exp.json
TEST(Complex, y_number_real_capital_e_pos_exp)  // NOLINT
{
	const auto result = json::decode("[1E+2]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(100.0)}));
}
// y_number_real_exponent.json
TEST(Complex, y_number_real_exponent)  // NOLINT
{
	const auto result = json::decode("[123e45]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1.23e+47)}));
}
// y_number_real_fraction_exponent.json
TEST(Complex, y_number_real_fraction_exponent)  // NOLINT
{
	const auto result = json::decode("[123.456e78]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(1.23456e+80)}));
}
// y_number_real_neg_exp.json
TEST(Complex, y_number_real_neg_exp)  // NOLINT
{
	const auto result = json::decode("[1e-2]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(0.01)}));
}
// y_number_real_pos_exponent.json
TEST(Complex, y_number_real_pos_exponent)  // NOLINT
{
	const auto result = json::decode("[1e+2]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(100.0)}));
}
// y_number_simple_int.json
TEST(Complex, y_number_simple_int)  // NOLINT
{
	const auto result = json::decode("[123]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(123.0)}));
}
// y_number_simple_real.json
TEST(Complex, y_number_simple_real)  // NOLINT
{
	const auto result = json::decode("[123.456789]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jnum(123.456789)}));
}
// y_object.json
TEST(Complex, y_object)  // NOLINT
{
	const auto result = json::decode("{\"asd\":\"sdf\", \"dfg\":\"fgh\"}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"asd"s, jstr("sdf"s)},
			{"dfg"s, jstr("fgh"s)}
	})
	);
}
// y_object_basic.json
TEST(Complex, y_object_basic)  // NOLINT
{
	const auto result = json::decode("{\"asd\":\"sdf\"}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"asd"s, jstr("sdf"s)}
	})
	);
}
// y_object_duplicated_key.json
TEST(Complex, y_object_duplicated_key)  // NOLINT
{
	const auto result = json::decode("{\"a\":\"b\",\"a\":\"c\"}"sv);
	ASSERT_TRUE(result);
	ASSERT_EQ(result->type(), json::Object);

	const auto& object = result->get<json::Object>();
	ASSERT_EQ(object.size(), 1);
	ASSERT_TRUE(object.contains("a"));
	ASSERT_EQ(object.at("a").type(), json::String);

	const auto& value = object.at("a").get<json::String>();
	EXPECT_TRUE(value == "b" || value == "c");
}
// y_object_duplicated_key_and_value.json
TEST(Complex, y_object_duplicated_key_and_value)  // NOLINT
{
	const auto result = json::decode("{\"a\":\"b\",\"a\":\"b\"}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"a"s, jstr("b"s)}
	})
	);
}
// y_object_empty.json
TEST(Complex, y_object_empty)  // NOLINT
{
	const auto result = json::decode("{}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jobj({}));
}
// y_object_empty_key.json
TEST(Complex, y_object_empty_key)  // NOLINT
{
	const auto result = json::decode("{\"\":0}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{""s, jnum(0.0)}
	})
	);
}
// y_object_escaped_null_in_key.json
TEST(Complex, y_object_escaped_null_in_key)  // NOLINT
{
	const auto result = json::decode(R"({"foo\u0000bar": 42})"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"foo\000bar"s, jnum(42.0)}
	})
	);
}
// y_object_extreme_numbers.json
TEST(Complex, y_object_extreme_numbers)  // NOLINT
{
	const auto result = json::decode("{ \"min\": -1.0e+28, \"max\": 1.0e+28 }"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"min"s, jnum(-1e+28)},
			{"max"s, jnum(1e+28) }
	})
	);
}
// y_object_long_strings.json
TEST(Complex, y_object_long_strings)  // NOLINT
{
	const auto result = json::decode(
		"{\"x\":[{\"id\": \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"}], \"id\": \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"}"sv
	);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"x"s,  jarr({jobj({{"id"s, jstr("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"s)}})})},
			{"id"s, jstr("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"s)                         }
	})
	);
}
// y_object_simple.json
TEST(Complex, y_object_simple)  // NOLINT
{
	const auto result = json::decode("{\"a\":[]}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"a"s, jarr({})}
	})
	);
}
// y_object_string_unicode.json
TEST(Complex, y_object_string_unicode)  // NOLINT
{
	const auto result = json::decode(
		R"({"title":"\u041f\u043e\u043b\u0442\u043e\u0440\u0430 \u0417\u0435\u043c\u043b\u0435\u043a\u043e\u043f\u0430" })"sv
	);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"title"s,
			 jstr(
				 "\u041f\u043e\u043b\u0442\u043e\u0440\u0430 \u0417\u0435\u043c\u043b\u0435\u043a\u043e\u043f\u0430"s
			 )}
	})
	);
}
// y_object_with_newlines.json
TEST(Complex, y_object_with_newlines)  // NOLINT
{
	const auto result = json::decode("{\n\"a\": \"b\"\n}"sv);
	ASSERT_TRUE(result);
	expect_values_equal(
		*result,
		jobj({
			{"a"s, jstr("b"s)}
	})
	);
}
// y_string_1_2_3_bytes_UTF-8_sequences.json
TEST(Complex, y_string_1_2_3_bytes_UTFminus8_sequences)  // NOLINT
{
	const auto result = json::decode("[\"\\u0060\\u012a\\u12AB\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("`\u012A\u12AB"s)}));
}
// y_string_accepted_surrogate_pair.json
TEST(Complex, y_string_accepted_surrogate_pair)  // NOLINT
{
	const auto result = json::decode("[\"\\uD801\\udc37\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U00010437"s)}));
}
// y_string_accepted_surrogate_pairs.json
TEST(Complex, y_string_accepted_surrogate_pairs)  // NOLINT
{
	const auto result = json::decode("[\"\\ud83d\\ude39\\ud83d\\udc8d\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0001F639\U0001F48D"s)}));
}
// y_string_allowed_escapes.json
TEST(Complex, y_string_allowed_escapes)  // NOLINT
{
	const auto result = json::decode("[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\"\\/\010\014\n\r\t"s)}));
}
// y_string_backslash_and_u_escaped_zero.json
TEST(Complex, y_string_backslash_and_u_escaped_zero)  // NOLINT
{
	const auto result = json::decode("[\"\\\\u0000\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\\u0000"s)}));
}
// y_string_backslash_doublequotes.json
TEST(Complex, y_string_backslash_doublequotes)  // NOLINT
{
	const auto result = json::decode("[\"\\\"\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\""s)}));
}
// y_string_comments.json
TEST(Complex, y_string_comments)  // NOLINT
{
	const auto result = json::decode("[\"a/*b*/c/*d//e\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("a/*b*/c/*d//e"s)}));
}
// y_string_double_escape_a.json
TEST(Complex, y_string_double_escape_a)  // NOLINT
{
	const auto result = json::decode("[\"\\\\a\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\\a"s)}));
}
// y_string_double_escape_n.json
TEST(Complex, y_string_double_escape_n)  // NOLINT
{
	const auto result = json::decode("[\"\\\\n\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\\n"s)}));
}
// y_string_escaped_control_character.json
TEST(Complex, y_string_escaped_control_character)  // NOLINT
{
	const auto result = json::decode("[\"\\u0012\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\022"s)}));
}
// y_string_escaped_noncharacter.json
TEST(Complex, y_string_escaped_noncharacter)  // NOLINT
{
	const auto result = json::decode("[\"\\uFFFF\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\uFFFF"s)}));
}
// y_string_in_array.json
TEST(Complex, y_string_in_array)  // NOLINT
{
	const auto result = json::decode("[\"asd\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("asd"s)}));
}
// y_string_in_array_with_leading_space.json
TEST(Complex, y_string_in_array_with_leading_space)  // NOLINT
{
	const auto result = json::decode("[ \"asd\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("asd"s)}));
}
// y_string_last_surrogates_1_and_2.json
TEST(Complex, y_string_last_surrogates_1_and_2)  // NOLINT
{
	const auto result = json::decode("[\"\\uDBFF\\uDFFF\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0010FFFF"s)}));
}
// y_string_nbsp_uescaped.json
TEST(Complex, y_string_nbsp_uescaped)  // NOLINT
{
	const auto result = json::decode("[\"new\\u00A0line\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("new\u00A0line"s)}));
}
// y_string_nonCharacterInUTF-8_U+10FFFF.json
TEST(Complex, y_string_nonCharacterInUTFminus8_Uplus10FFFF)  // NOLINT
{
	const auto result = json::decode("[\"\U0010FFFF\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0010FFFF"s)}));
}
// y_string_nonCharacterInUTF-8_U+FFFF.json
TEST(Complex, y_string_nonCharacterInUTFminus8_UplusFFFF)  // NOLINT
{
	const auto result = json::decode("[\"\uFFFF\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\uFFFF"s)}));
}
// y_string_null_escape.json
TEST(Complex, y_string_null_escape)  // NOLINT
{
	const auto result = json::decode("[\"\\u0000\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\000"s)}));
}
// y_string_one-byte-utf-8.json
TEST(Complex, y_string_oneminusbyteminusutfminus8)  // NOLINT
{
	const auto result = json::decode("[\"\\u002c\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr(","s)}));
}
// y_string_pi.json
TEST(Complex, y_string_pi)  // NOLINT
{
	const auto result = json::decode("[\"\u03C0\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u03C0"s)}));
}
// y_string_reservedCharacterInUTF-8_U+1BFFF.json
TEST(Complex, y_string_reservedCharacterInUTFminus8_Uplus1BFFF)  // NOLINT
{
	const auto result = json::decode("[\"\U0001BFFF\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0001BFFF"s)}));
}
// y_string_simple_ascii.json
TEST(Complex, y_string_simple_ascii)  // NOLINT
{
	const auto result = json::decode("[\"asd \"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("asd "s)}));
}
// y_string_space.json
TEST(Complex, y_string_space)  // NOLINT
{
	const auto result = json::decode("\" \""sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jstr(" "s));
}
// y_string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF.json
TEST(Complex, y_string_surrogates_Uplus1D11E_MUSICAL_SYMBOL_G_CLEF)  // NOLINT
{
	const auto result = json::decode("[\"\\uD834\\uDd1e\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0001D11E"s)}));
}
// y_string_three-byte-utf-8.json
TEST(Complex, y_string_threeminusbyteminusutfminus8)  // NOLINT
{
	const auto result = json::decode("[\"\\u0821\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u0821"s)}));
}
// y_string_two-byte-utf-8.json
TEST(Complex, y_string_twominusbyteminusutfminus8)  // NOLINT
{
	const auto result = json::decode("[\"\\u0123\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u0123"s)}));
}
// y_string_u+2028_line_sep.json
TEST(Complex, y_string_uplus2028_line_sep)  // NOLINT
{
	const auto result = json::decode("[\"\u2028\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u2028"s)}));
}
// y_string_u+2029_par_sep.json
TEST(Complex, y_string_uplus2029_par_sep)  // NOLINT
{
	const auto result = json::decode("[\"\u2029\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u2029"s)}));
}
// y_string_uEscape.json
TEST(Complex, y_string_uEscape)  // NOLINT
{
	const auto result = json::decode("[\"\\u0061\\u30af\\u30EA\\u30b9\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("a\u30AF\u30EA\u30B9"s)}));
}
// y_string_uescaped_newline.json
TEST(Complex, y_string_uescaped_newline)  // NOLINT
{
	const auto result = json::decode("[\"new\\u000Aline\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("new\nline"s)}));
}
// y_string_unescaped_char_delete.json
TEST(Complex, y_string_unescaped_char_delete)  // NOLINT
{
	const auto result = json::decode("[\"\177\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\177"s)}));
}
// y_string_unicode.json
TEST(Complex, y_string_unicode)  // NOLINT
{
	const auto result = json::decode("[\"\\uA66D\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\uA66D"s)}));
}
// y_string_unicodeEscapedBackslash.json
TEST(Complex, y_string_unicodeEscapedBackslash)  // NOLINT
{
	const auto result = json::decode("[\"\\u005C\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\\"s)}));
}
// y_string_unicode_2.json
TEST(Complex, y_string_unicode_2)  // NOLINT
{
	const auto result = json::decode("[\"\u2342\u3234\u2342\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u2342\u3234\u2342"s)}));
}
// y_string_unicode_U+10FFFE_nonchar.json
TEST(Complex, y_string_unicode_Uplus10FFFE_nonchar)  // NOLINT
{
	const auto result = json::decode("[\"\\uDBFF\\uDFFE\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0010FFFE"s)}));
}
// y_string_unicode_U+1FFFE_nonchar.json
TEST(Complex, y_string_unicode_Uplus1FFFE_nonchar)  // NOLINT
{
	const auto result = json::decode("[\"\\uD83F\\uDFFE\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\U0001FFFE"s)}));
}
// y_string_unicode_U+200B_ZERO_WIDTH_SPACE.json
TEST(Complex, y_string_unicode_Uplus200B_ZERO_WIDTH_SPACE)  // NOLINT
{
	const auto result = json::decode("[\"\\u200B\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u200B"s)}));
}
// y_string_unicode_U+2064_invisible_plus.json
TEST(Complex, y_string_unicode_Uplus2064_invisible_plus)  // NOLINT
{
	const auto result = json::decode("[\"\\u2064\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u2064"s)}));
}
// y_string_unicode_U+FDD0_nonchar.json
TEST(Complex, y_string_unicode_UplusFDD0_nonchar)  // NOLINT
{
	const auto result = json::decode("[\"\\uFDD0\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\uFDD0"s)}));
}
// y_string_unicode_U+FFFE_nonchar.json
TEST(Complex, y_string_unicode_UplusFFFE_nonchar)  // NOLINT
{
	const auto result = json::decode("[\"\\uFFFE\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\uFFFE"s)}));
}
// y_string_unicode_escaped_double_quote.json
TEST(Complex, y_string_unicode_escaped_double_quote)  // NOLINT
{
	const auto result = json::decode("[\"\\u0022\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\""s)}));
}
// y_string_utf8.json
TEST(Complex, y_string_utf8)  // NOLINT
{
	const auto result = json::decode("[\"\u20AC\U0001D11E\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("\u20AC\U0001D11E"s)}));
}
// y_string_with_del_character.json
TEST(Complex, y_string_with_del_character)  // NOLINT
{
	const auto result = json::decode("[\"a\177a\"]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("a\177a"s)}));
}
// y_structure_lonely_false.json
TEST(Complex, y_structure_lonely_false)  // NOLINT
{
	const auto result = json::decode("false"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jbool(false));
}
// y_structure_lonely_int.json
TEST(Complex, y_structure_lonely_int)  // NOLINT
{
	const auto result = json::decode("42"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jnum(42.0));
}
// y_structure_lonely_negative_real.json
TEST(Complex, y_structure_lonely_negative_real)  // NOLINT
{
	const auto result = json::decode("-0.1"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jnum(-0.1));
}
// y_structure_lonely_null.json
TEST(Complex, y_structure_lonely_null)  // NOLINT
{
	const auto result = json::decode("null"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jnull());
}
// y_structure_lonely_string.json
TEST(Complex, y_structure_lonely_string)  // NOLINT
{
	const auto result = json::decode("\"asd\""sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jstr("asd"s));
}
// y_structure_lonely_true.json
TEST(Complex, y_structure_lonely_true)  // NOLINT
{
	const auto result = json::decode("true"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jbool(true));
}
// y_structure_string_empty.json
TEST(Complex, y_structure_string_empty)  // NOLINT
{
	const auto result = json::decode("\"\""sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jstr(""s));
}
// y_structure_trailing_newline.json
TEST(Complex, y_structure_trailing_newline)  // NOLINT
{
	const auto result = json::decode("[\"a\"]\n"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jstr("a"s)}));
}
// y_structure_true_in_array.json
TEST(Complex, y_structure_true_in_array)  // NOLINT
{
	const auto result = json::decode("[true]"sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({jbool(true)}));
}
// y_structure_whitespace_array.json
TEST(Complex, y_structure_whitespace_array)  // NOLINT
{
	const auto result = json::decode(" [] "sv);
	ASSERT_TRUE(result);
	expect_values_equal(*result, jarr({}));
}

// ===== JSONTestSuite must-reject cases (n_*.json) =====

// n_array_1_true_without_comma.json
TEST(Complex, n_array_1_true_without_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1 true]"sv));
}
// n_array_a_invalid_utf8.json
TEST(Complex, n_array_a_invalid_utf8)  // NOLINT
{
	EXPECT_FALSE(json::decode("[a\345]"sv));
}
// n_array_colon_instead_of_comma.json
TEST(Complex, n_array_colon_instead_of_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\": 1]"sv));
}
// n_array_comma_after_close.json
TEST(Complex, n_array_comma_after_close)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\"],"sv));
}
// n_array_comma_and_number.json
TEST(Complex, n_array_comma_and_number)  // NOLINT
{
	EXPECT_FALSE(json::decode("[,1]"sv));
}
// n_array_double_comma.json
TEST(Complex, n_array_double_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1,,2]"sv));
}
// n_array_double_extra_comma.json
TEST(Complex, n_array_double_extra_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"x\",,]"sv));
}
// n_array_extra_close.json
TEST(Complex, n_array_extra_close)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"x\"]]"sv));
}
// n_array_extra_comma.json
TEST(Complex, n_array_extra_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\",]"sv));
}
// n_array_incomplete.json
TEST(Complex, n_array_incomplete)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"x\""sv));
}
// n_array_incomplete_invalid_value.json
TEST(Complex, n_array_incomplete_invalid_value)  // NOLINT
{
	EXPECT_FALSE(json::decode("[x"sv));
}
// n_array_inner_array_no_comma.json
TEST(Complex, n_array_inner_array_no_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[3[4]]"sv));
}
// n_array_invalid_utf8.json
TEST(Complex, n_array_invalid_utf8)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\377]"sv));
}
// n_array_items_separated_by_semicolon.json
TEST(Complex, n_array_items_separated_by_semicolon)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1:2]"sv));
}
// n_array_just_comma.json
TEST(Complex, n_array_just_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[,]"sv));
}
// n_array_just_minus.json
TEST(Complex, n_array_just_minus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-]"sv));
}
// n_array_missing_value.json
TEST(Complex, n_array_missing_value)  // NOLINT
{
	EXPECT_FALSE(json::decode("[   , \"\"]"sv));
}
// n_array_newlines_unclosed.json
TEST(Complex, n_array_newlines_unclosed)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"a\",\n4\n,1,"sv));
}
// n_array_number_and_comma.json
TEST(Complex, n_array_number_and_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1,]"sv));
}
// n_array_number_and_several_commas.json
TEST(Complex, n_array_number_and_several_commas)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1,,]"sv));
}
// n_array_spaces_vertical_tab_formfeed.json
TEST(Complex, n_array_spaces_vertical_tab_formfeed)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\013a\"\\f]"sv));
}
// n_array_star_inside.json
TEST(Complex, n_array_star_inside)  // NOLINT
{
	EXPECT_FALSE(json::decode("[*]"sv));
}
// n_array_unclosed.json
TEST(Complex, n_array_unclosed)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\""sv));
}
// n_array_unclosed_trailing_comma.json
TEST(Complex, n_array_unclosed_trailing_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1,"sv));
}
// n_array_unclosed_with_new_lines.json
TEST(Complex, n_array_unclosed_with_new_lines)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1,\n1\n,1"sv));
}
// n_array_unclosed_with_object_inside.json
TEST(Complex, n_array_unclosed_with_object_inside)  // NOLINT
{
	EXPECT_FALSE(json::decode("[{}"sv));
}
// n_incomplete_false.json
TEST(Complex, n_incomplete_false)  // NOLINT
{
	EXPECT_FALSE(json::decode("[fals]"sv));
}
// n_incomplete_null.json
TEST(Complex, n_incomplete_null)  // NOLINT
{
	EXPECT_FALSE(json::decode("[nul]"sv));
}
// n_incomplete_true.json
TEST(Complex, n_incomplete_true)  // NOLINT
{
	EXPECT_FALSE(json::decode("[tru]"sv));
}
// n_multidigit_number_then_00.json
TEST(Complex, n_multidigit_number_then_00)  // NOLINT
{
	EXPECT_FALSE(json::decode("123\000"sv));
}
// n_number_++.json
TEST(Complex, n_number_plusplus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[++1234]"sv));
}
// n_number_+1.json
TEST(Complex, n_number_plus1)  // NOLINT
{
	EXPECT_FALSE(json::decode("[+1]"sv));
}
// n_number_+Inf.json
TEST(Complex, n_number_plusInf)  // NOLINT
{
	EXPECT_FALSE(json::decode("[+Inf]"sv));
}
// n_number_-01.json
TEST(Complex, n_number_minus01)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-01]"sv));
}
// n_number_-1.0..json
TEST(Complex, n_number_minus1_0_)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-1.0.]"sv));
}
// n_number_-2..json
TEST(Complex, n_number_minus2_)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-2.]"sv));
}
// n_number_-NaN.json
TEST(Complex, n_number_minusNaN)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-NaN]"sv));
}
// n_number_.-1.json
TEST(Complex, n_number__minus1)  // NOLINT
{
	EXPECT_FALSE(json::decode("[.-1]"sv));
}
// n_number_.2e-3.json
TEST(Complex, n_number__2eminus3)  // NOLINT
{
	EXPECT_FALSE(json::decode("[.2e-3]"sv));
}
// n_number_0.1.2.json
TEST(Complex, n_number_0_1_2)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0.1.2]"sv));
}
// n_number_0.3e+.json
TEST(Complex, n_number_0_3eplus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0.3e+]"sv));
}
// n_number_0.3e.json
TEST(Complex, n_number_0_3e)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0.3e]"sv));
}
// n_number_0.e1.json
TEST(Complex, n_number_0_e1)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0.e1]"sv));
}
// n_number_0_capital_E+.json
TEST(Complex, n_number_0_capital_Eplus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0E+]"sv));
}
// n_number_0_capital_E.json
TEST(Complex, n_number_0_capital_E)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0E]"sv));
}
// n_number_0e+.json
TEST(Complex, n_number_0eplus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0e+]"sv));
}
// n_number_0e.json
TEST(Complex, n_number_0e)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0e]"sv));
}
// n_number_1.0e+.json
TEST(Complex, n_number_1_0eplus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1.0e+]"sv));
}
// n_number_1.0e-.json
TEST(Complex, n_number_1_0eminus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1.0e-]"sv));
}
// n_number_1.0e.json
TEST(Complex, n_number_1_0e)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1.0e]"sv));
}
// n_number_1_000.json
TEST(Complex, n_number_1_000)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1 000.0]"sv));
}
// n_number_1eE2.json
TEST(Complex, n_number_1eE2)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1eE2]"sv));
}
// n_number_2.e+3.json
TEST(Complex, n_number_2_eplus3)  // NOLINT
{
	EXPECT_FALSE(json::decode("[2.e+3]"sv));
}
// n_number_2.e-3.json
TEST(Complex, n_number_2_eminus3)  // NOLINT
{
	EXPECT_FALSE(json::decode("[2.e-3]"sv));
}
// n_number_2.e3.json
TEST(Complex, n_number_2_e3)  // NOLINT
{
	EXPECT_FALSE(json::decode("[2.e3]"sv));
}
// n_number_9.e+.json
TEST(Complex, n_number_9_eplus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[9.e+]"sv));
}
// n_number_Inf.json
TEST(Complex, n_number_Inf)  // NOLINT
{
	EXPECT_FALSE(json::decode("[Inf]"sv));
}
// n_number_NaN.json
TEST(Complex, n_number_NaN)  // NOLINT
{
	EXPECT_FALSE(json::decode("[NaN]"sv));
}
// n_number_U+FF11_fullwidth_digit_one.json
TEST(Complex, n_number_UplusFF11_fullwidth_digit_one)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\uFF11]"sv));
}
// n_number_expression.json
TEST(Complex, n_number_expression)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1+2]"sv));
}
// n_number_hex_1_digit.json
TEST(Complex, n_number_hex_1_digit)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0x1]"sv));
}
// n_number_hex_2_digits.json
TEST(Complex, n_number_hex_2_digits)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0x42]"sv));
}
// n_number_infinity.json
TEST(Complex, n_number_infinity)  // NOLINT
{
	EXPECT_FALSE(json::decode("[Infinity]"sv));
}
// n_number_invalid+-.json
TEST(Complex, n_number_invalidplusminus)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0e+-1]"sv));
}
// n_number_invalid-negative-real.json
TEST(Complex, n_number_invalidminusnegativeminusreal)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-123.123foo]"sv));
}
// n_number_invalid-utf-8-in-bigger-int.json
TEST(Complex, n_number_invalidminusutfminus8minusinminusbiggerminusint)  // NOLINT
{
	EXPECT_FALSE(json::decode("[123\345]"sv));
}
// n_number_invalid-utf-8-in-exponent.json
TEST(Complex, n_number_invalidminusutfminus8minusinminusexponent)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1e1\345]"sv));
}
// n_number_invalid-utf-8-in-int.json
TEST(Complex, n_number_invalidminusutfminus8minusinminusint)  // NOLINT
{
	EXPECT_FALSE(json::decode("[0\345]\n"sv));
}
// n_number_minus_infinity.json
TEST(Complex, n_number_minus_infinity)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-Infinity]"sv));
}
// n_number_minus_sign_with_trailing_garbage.json
TEST(Complex, n_number_minus_sign_with_trailing_garbage)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-foo]"sv));
}
// n_number_minus_space_1.json
TEST(Complex, n_number_minus_space_1)  // NOLINT
{
	EXPECT_FALSE(json::decode("[- 1]"sv));
}
// n_number_neg_int_starting_with_zero.json
TEST(Complex, n_number_neg_int_starting_with_zero)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-012]"sv));
}
// n_number_neg_real_without_int_part.json
TEST(Complex, n_number_neg_real_without_int_part)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-.123]"sv));
}
// n_number_neg_with_garbage_at_end.json
TEST(Complex, n_number_neg_with_garbage_at_end)  // NOLINT
{
	EXPECT_FALSE(json::decode("[-1x]"sv));
}
// n_number_real_garbage_after_e.json
TEST(Complex, n_number_real_garbage_after_e)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1ea]"sv));
}
// n_number_real_with_invalid_utf8_after_e.json
TEST(Complex, n_number_real_with_invalid_utf8_after_e)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1e\345]"sv));
}
// n_number_real_without_fractional_part.json
TEST(Complex, n_number_real_without_fractional_part)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1.]"sv));
}
// n_number_starting_with_dot.json
TEST(Complex, n_number_starting_with_dot)  // NOLINT
{
	EXPECT_FALSE(json::decode("[.123]"sv));
}
// n_number_with_alpha.json
TEST(Complex, n_number_with_alpha)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1.2a-3]"sv));
}
// n_number_with_alpha_char.json
TEST(Complex, n_number_with_alpha_char)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1.8011670033376514H-308]"sv));
}
// n_number_with_leading_zero.json
TEST(Complex, n_number_with_leading_zero)  // NOLINT
{
	EXPECT_FALSE(json::decode("[012]"sv));
}
// n_object_bad_value.json
TEST(Complex, n_object_bad_value)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"x\", truth]"sv));
}
// n_object_bracket_key.json
TEST(Complex, n_object_bracket_key)  // NOLINT
{
	EXPECT_FALSE(json::decode("{[: \"x\"}\n"sv));
}
// n_object_comma_instead_of_colon.json
TEST(Complex, n_object_comma_instead_of_colon)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"x\", null}"sv));
}
// n_object_double_colon.json
TEST(Complex, n_object_double_colon)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"x\"::\"b\"}"sv));
}
// n_object_emoji.json
TEST(Complex, n_object_emoji)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\U0001F1E8\U0001F1ED}"sv));
}
// n_object_garbage_at_end.json
TEST(Complex, n_object_garbage_at_end)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"a\" 123}"sv));
}
// n_object_key_with_single_quotes.json
TEST(Complex, n_object_key_with_single_quotes)  // NOLINT
{
	EXPECT_FALSE(json::decode("{key: 'value'}"sv));
}
// n_object_lone_continuation_byte_in_key_and_trailing_comma.json
TEST(Complex, n_object_lone_continuation_byte_in_key_and_trailing_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"\271\":\"0\",}"sv));
}
// n_object_missing_colon.json
TEST(Complex, n_object_missing_colon)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\" b}"sv));
}
// n_object_missing_key.json
TEST(Complex, n_object_missing_key)  // NOLINT
{
	EXPECT_FALSE(json::decode("{:\"b\"}"sv));
}
// n_object_missing_semicolon.json
TEST(Complex, n_object_missing_semicolon)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\" \"b\"}"sv));
}
// n_object_missing_value.json
TEST(Complex, n_object_missing_value)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":"sv));
}
// n_object_no-colon.json
TEST(Complex, n_object_nominuscolon)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\""sv));
}
// n_object_non_string_key.json
TEST(Complex, n_object_non_string_key)  // NOLINT
{
	EXPECT_FALSE(json::decode("{1:1}"sv));
}
// n_object_non_string_key_but_huge_number_instead.json
TEST(Complex, n_object_non_string_key_but_huge_number_instead)  // NOLINT
{
	EXPECT_FALSE(json::decode("{9999E9999:1}"sv));
}
// n_object_repeated_null_null.json
TEST(Complex, n_object_repeated_null_null)  // NOLINT
{
	EXPECT_FALSE(json::decode("{null:null,null:null}"sv));
}
// n_object_several_trailing_commas.json
TEST(Complex, n_object_several_trailing_commas)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"id\":0,,,,,}"sv));
}
// n_object_single_quote.json
TEST(Complex, n_object_single_quote)  // NOLINT
{
	EXPECT_FALSE(json::decode("{'a':0}"sv));
}
// n_object_trailing_comma.json
TEST(Complex, n_object_trailing_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"id\":0,}"sv));
}
// n_object_trailing_comment.json
TEST(Complex, n_object_trailing_comment)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\"}/**/"sv));
}
// n_object_trailing_comment_open.json
TEST(Complex, n_object_trailing_comment_open)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\"}/**//"sv));
}
// n_object_trailing_comment_slash_open.json
TEST(Complex, n_object_trailing_comment_slash_open)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\"}//"sv));
}
// n_object_trailing_comment_slash_open_incomplete.json
TEST(Complex, n_object_trailing_comment_slash_open_incomplete)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\"}/"sv));
}
// n_object_two_commas_in_a_row.json
TEST(Complex, n_object_two_commas_in_a_row)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\",,\"c\":\"d\"}"sv));
}
// n_object_unquoted_key.json
TEST(Complex, n_object_unquoted_key)  // NOLINT
{
	EXPECT_FALSE(json::decode("{a: \"b\"}"sv));
}
// n_object_unterminated-value.json
TEST(Complex, n_object_unterminatedminusvalue)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"a"sv));
}
// n_object_with_single_string.json
TEST(Complex, n_object_with_single_string)  // NOLINT
{
	EXPECT_FALSE(json::decode("{ \"foo\" : \"bar\", \"a\" }"sv));
}
// n_object_with_trailing_garbage.json
TEST(Complex, n_object_with_trailing_garbage)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\"}#"sv));
}
// n_single_space.json
TEST(Complex, n_single_space)  // NOLINT
{
	EXPECT_FALSE(json::decode(" "sv));
}
// n_string_1_surrogate_then_escape.json
TEST(Complex, n_string_1_surrogate_then_escape)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uD800\\\"]"sv));
}
// n_string_1_surrogate_then_escape_u.json
TEST(Complex, n_string_1_surrogate_then_escape_u)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uD800\\u\"]"sv));
}
// n_string_1_surrogate_then_escape_u1.json
TEST(Complex, n_string_1_surrogate_then_escape_u1)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uD800\\u1\"]"sv));
}
// n_string_1_surrogate_then_escape_u1x.json
TEST(Complex, n_string_1_surrogate_then_escape_u1x)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uD800\\u1x\"]"sv));
}
// n_string_accentuated_char_no_quotes.json
TEST(Complex, n_string_accentuated_char_no_quotes)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\u00E9]"sv));
}
// n_string_backslash_00.json
TEST(Complex, n_string_backslash_00)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\\000\"]"sv));
}
// n_string_escape_x.json
TEST(Complex, n_string_escape_x)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\x00\"]"sv));
}
// n_string_escaped_backslash_bad.json
TEST(Complex, n_string_escaped_backslash_bad)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\\\\\\"]"sv));
}
// n_string_escaped_ctrl_char_tab.json
TEST(Complex, n_string_escaped_ctrl_char_tab)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\\t\"]"sv));
}
// n_string_escaped_emoji.json
TEST(Complex, n_string_escaped_emoji)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\\U0001F300\"]"sv));
}
// n_string_incomplete_escape.json
TEST(Complex, n_string_incomplete_escape)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\\"]"sv));
}
// n_string_incomplete_escaped_character.json
TEST(Complex, n_string_incomplete_escaped_character)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\u00A\"]"sv));
}
// n_string_incomplete_surrogate.json
TEST(Complex, n_string_incomplete_surrogate)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uD834\\uDd\"]"sv));
}
// n_string_incomplete_surrogate_escape_invalid.json
TEST(Complex, n_string_incomplete_surrogate_escape_invalid)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uD800\\uD800\\x\"]"sv));
}
// n_string_invalid-utf-8-in-escape.json
TEST(Complex, n_string_invalidminusutfminus8minusinminusescape)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\u\345\"]"sv));
}
// n_string_invalid_backslash_esc.json
TEST(Complex, n_string_invalid_backslash_esc)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\a\"]"sv));
}
// n_string_invalid_unicode_escape.json
TEST(Complex, n_string_invalid_unicode_escape)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\uqqqq\"]"sv));
}
// n_string_invalid_utf8_after_escape.json
TEST(Complex, n_string_invalid_utf8_after_escape)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\\345\"]"sv));
}
// n_string_leading_uescaped_thinspace.json
TEST(Complex, n_string_leading_uescaped_thinspace)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\\u0020\"asd\"]"sv));
}
// n_string_no_quotes_with_bad_escape.json
TEST(Complex, n_string_no_quotes_with_bad_escape)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\\n]"sv));
}
// n_string_single_doublequote.json
TEST(Complex, n_string_single_doublequote)  // NOLINT
{
	EXPECT_FALSE(json::decode("\""sv));
}
// n_string_single_quote.json
TEST(Complex, n_string_single_quote)  // NOLINT
{
	EXPECT_FALSE(json::decode("['single quote']"sv));
}
// n_string_single_string_no_double_quotes.json
TEST(Complex, n_string_single_string_no_double_quotes)  // NOLINT
{
	EXPECT_FALSE(json::decode("abc"sv));
}
// n_string_start_escape_unclosed.json
TEST(Complex, n_string_start_escape_unclosed)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\"sv));
}
// n_string_unescaped_ctrl_char.json
TEST(Complex, n_string_unescaped_ctrl_char)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"a\000a\"]"sv));
}
// n_string_unescaped_newline.json
TEST(Complex, n_string_unescaped_newline)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"new\nline\"]"sv));
}
// n_string_unescaped_tab.json
TEST(Complex, n_string_unescaped_tab)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\t\"]"sv));
}
// n_string_unicode_CapitalU.json
TEST(Complex, n_string_unicode_CapitalU)  // NOLINT
{
	EXPECT_FALSE(json::decode("\"\\UA66D\""sv));
}
// n_string_with_trailing_garbage.json
TEST(Complex, n_string_with_trailing_garbage)  // NOLINT
{
	EXPECT_FALSE(json::decode("\"\"x"sv));
}
// n_structure_100000_opening_arrays.json
TEST(Complex, n_structure_100000_opening_arrays)  // NOLINT
{
	const std::string input(100000, '[');
	EXPECT_FALSE(json::decode(input));
}
// n_structure_U+2060_word_joined.json
TEST(Complex, n_structure_Uplus2060_word_joined)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\u2060]"sv));
}
// n_structure_UTF8_BOM_no_data.json
TEST(Complex, n_structure_UTF8_BOM_no_data)  // NOLINT
{
	EXPECT_FALSE(json::decode("\uFEFF"sv));
}
// n_structure_angle_bracket_..json
TEST(Complex, n_structure_angle_bracket__)  // NOLINT
{
	EXPECT_FALSE(json::decode("<.>"sv));
}
// n_structure_angle_bracket_null.json
TEST(Complex, n_structure_angle_bracket_null)  // NOLINT
{
	EXPECT_FALSE(json::decode("[<null>]"sv));
}
// n_structure_array_trailing_garbage.json
TEST(Complex, n_structure_array_trailing_garbage)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1]x"sv));
}
// n_structure_array_with_extra_array_close.json
TEST(Complex, n_structure_array_with_extra_array_close)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1]]"sv));
}
// n_structure_array_with_unclosed_string.json
TEST(Complex, n_structure_array_with_unclosed_string)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"asd]"sv));
}
// n_structure_ascii-unicode-identifier.json
TEST(Complex, n_structure_asciiminusunicodeminusidentifier)  // NOLINT
{
	EXPECT_FALSE(json::decode("a\u00E5"sv));
}
// n_structure_capitalized_True.json
TEST(Complex, n_structure_capitalized_True)  // NOLINT
{
	EXPECT_FALSE(json::decode("[True]"sv));
}
// n_structure_close_unopened_array.json
TEST(Complex, n_structure_close_unopened_array)  // NOLINT
{
	EXPECT_FALSE(json::decode("1]"sv));
}
// n_structure_comma_instead_of_closing_brace.json
TEST(Complex, n_structure_comma_instead_of_closing_brace)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"x\": true,"sv));
}
// n_structure_double_array.json
TEST(Complex, n_structure_double_array)  // NOLINT
{
	EXPECT_FALSE(json::decode("[][]"sv));
}
// n_structure_end_array.json
TEST(Complex, n_structure_end_array)  // NOLINT
{
	EXPECT_FALSE(json::decode("]"sv));
}
// n_structure_incomplete_UTF8_BOM.json
TEST(Complex, n_structure_incomplete_UTF8_BOM)  // NOLINT
{
	EXPECT_FALSE(json::decode("\357\273{}"sv));
}
// n_structure_lone-invalid-utf-8.json
TEST(Complex, n_structure_loneminusinvalidminusutfminus8)  // NOLINT
{
	EXPECT_FALSE(json::decode("\345"sv));
}
// n_structure_lone-open-bracket.json
TEST(Complex, n_structure_loneminusopenminusbracket)  // NOLINT
{
	EXPECT_FALSE(json::decode("["sv));
}
// n_structure_no_data.json
TEST(Complex, n_structure_no_data)  // NOLINT
{
	EXPECT_FALSE(json::decode(""sv));
}
// n_structure_null-byte-outside-string.json
TEST(Complex, n_structure_nullminusbyteminusoutsideminusstring)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\000]"sv));
}
// n_structure_number_with_trailing_garbage.json
TEST(Complex, n_structure_number_with_trailing_garbage)  // NOLINT
{
	EXPECT_FALSE(json::decode("2@"sv));
}
// n_structure_object_followed_by_closing_object.json
TEST(Complex, n_structure_object_followed_by_closing_object)  // NOLINT
{
	EXPECT_FALSE(json::decode("{}}"sv));
}
// n_structure_object_unclosed_no_value.json
TEST(Complex, n_structure_object_unclosed_no_value)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"\":"sv));
}
// n_structure_object_with_comment.json
TEST(Complex, n_structure_object_with_comment)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":/*comment*/\"b\"}"sv));
}
// n_structure_object_with_trailing_garbage.json
TEST(Complex, n_structure_object_with_trailing_garbage)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\": true} \"x\""sv));
}
// n_structure_open_array_apostrophe.json
TEST(Complex, n_structure_open_array_apostrophe)  // NOLINT
{
	EXPECT_FALSE(json::decode("['"sv));
}
// n_structure_open_array_comma.json
TEST(Complex, n_structure_open_array_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("[,"sv));
}
// n_structure_open_array_object.json
TEST(Complex, n_structure_open_array_object)  // NOLINT
{
	std::string input;
	input.reserve(250001);
	for (std::size_t i = 0; i < 50000; ++i) input += "[{\"\":";
	input += '\n';
	EXPECT_FALSE(json::decode(input));
}
// n_structure_open_array_open_object.json
TEST(Complex, n_structure_open_array_open_object)  // NOLINT
{
	EXPECT_FALSE(json::decode("[{"sv));
}
// n_structure_open_array_open_string.json
TEST(Complex, n_structure_open_array_open_string)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"a"sv));
}
// n_structure_open_array_string.json
TEST(Complex, n_structure_open_array_string)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"a\""sv));
}
// n_structure_open_object.json
TEST(Complex, n_structure_open_object)  // NOLINT
{
	EXPECT_FALSE(json::decode("{"sv));
}
// n_structure_open_object_close_array.json
TEST(Complex, n_structure_open_object_close_array)  // NOLINT
{
	EXPECT_FALSE(json::decode("{]"sv));
}
// n_structure_open_object_comma.json
TEST(Complex, n_structure_open_object_comma)  // NOLINT
{
	EXPECT_FALSE(json::decode("{,"sv));
}
// n_structure_open_object_open_array.json
TEST(Complex, n_structure_open_object_open_array)  // NOLINT
{
	EXPECT_FALSE(json::decode("{["sv));
}
// n_structure_open_object_open_string.json
TEST(Complex, n_structure_open_object_open_string)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a"sv));
}
// n_structure_open_object_string_with_apostrophes.json
TEST(Complex, n_structure_open_object_string_with_apostrophes)  // NOLINT
{
	EXPECT_FALSE(json::decode("{'a'"sv));
}
// n_structure_open_open.json
TEST(Complex, n_structure_open_open)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\"\\{[\"\\{[\"\\{[\"\\{"sv));
}
// n_structure_single_eacute.json
TEST(Complex, n_structure_single_eacute)  // NOLINT
{
	EXPECT_FALSE(json::decode("\351"sv));
}
// n_structure_single_star.json
TEST(Complex, n_structure_single_star)  // NOLINT
{
	EXPECT_FALSE(json::decode("*"sv));
}
// n_structure_trailing_#.json
TEST(Complex, n_structure_trailing__)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"a\":\"b\"}#{}"sv));
}
// n_structure_uescaped_LF_before_string.json
TEST(Complex, n_structure_uescaped_LF_before_string)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\\u000A\"\"]"sv));
}
// n_structure_unclosed_array.json
TEST(Complex, n_structure_unclosed_array)  // NOLINT
{
	EXPECT_FALSE(json::decode("[1"sv));
}
// n_structure_unclosed_array_partial_null.json
TEST(Complex, n_structure_unclosed_array_partial_null)  // NOLINT
{
	EXPECT_FALSE(json::decode("[ false, nul"sv));
}
// n_structure_unclosed_array_unfinished_false.json
TEST(Complex, n_structure_unclosed_array_unfinished_false)  // NOLINT
{
	EXPECT_FALSE(json::decode("[ true, fals"sv));
}
// n_structure_unclosed_array_unfinished_true.json
TEST(Complex, n_structure_unclosed_array_unfinished_true)  // NOLINT
{
	EXPECT_FALSE(json::decode("[ false, tru"sv));
}
// n_structure_unclosed_object.json
TEST(Complex, n_structure_unclosed_object)  // NOLINT
{
	EXPECT_FALSE(json::decode("{\"asd\":\"asd\""sv));
}
// n_structure_unicode-identifier.json
TEST(Complex, n_structure_unicodeminusidentifier)  // NOLINT
{
	EXPECT_FALSE(json::decode("\u00E5"sv));
}
// n_structure_whitespace_U+2060_word_joiner.json
TEST(Complex, n_structure_whitespace_Uplus2060_word_joiner)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\u2060]"sv));
}
// n_structure_whitespace_formfeed.json
TEST(Complex, n_structure_whitespace_formfeed)  // NOLINT
{
	EXPECT_FALSE(json::decode("[\014]"sv));
}
