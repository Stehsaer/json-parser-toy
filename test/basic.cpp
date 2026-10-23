#include <gtest/gtest.h>

import json;

TEST(Basic, LiteralValue)  // NOLINT
{
	{
		const auto result = json::decode("1.0");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Number);
		EXPECT_FLOAT_EQ(result->get<json::Number>(), 1.0);
	}

	{
		const auto result = json::decode("null");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Null);
	}

	{
		const auto result = json::decode("true");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Bool);
		EXPECT_EQ(result->get<json::Bool>(), true);
	}

	{
		const auto result = json::decode("false");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Bool);
		EXPECT_EQ(result->get<json::Bool>(), false);
	}

	{
		const auto result = json::decode("\"false\"");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::String);
		EXPECT_EQ(result->get<json::String>(), "false");
	}
}

TEST(Basic, TreeValue)  // NOLINT
{
	{
		const auto result = json::decode("[]");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& arr = result->get<json::Array>();
		EXPECT_EQ(arr.size(), 0);
	}

	{
		const auto result = json::decode("[1.0, 2.0, 3.0]");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& arr = result->get<json::Array>();
		ASSERT_EQ(arr.size(), 3);

		ASSERT_EQ(arr[0].type(), json::Number);
		ASSERT_EQ(arr[1].type(), json::Number);
		ASSERT_EQ(arr[2].type(), json::Number);

		EXPECT_FLOAT_EQ(arr[0].get<json::Number>(), 1.0);
		EXPECT_FLOAT_EQ(arr[1].get<json::Number>(), 2.0);
		EXPECT_FLOAT_EQ(arr[2].get<json::Number>(), 3.0);
	}

	{
		const auto result = json::decode("{}");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& arr = result->get<json::Object>();
		EXPECT_EQ(arr.size(), 0);
	}

	{
		const auto result = json::decode(R"({"a": 1.0, "b": "hello", "c": null})");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& obj = result->get<json::Object>();
		EXPECT_EQ(obj.size(), 3);

		ASSERT_TRUE(obj.contains("a"));
		ASSERT_TRUE(obj.contains("b"));
		ASSERT_TRUE(obj.contains("c"));

		ASSERT_EQ(obj.at("a").type(), json::Number);
		ASSERT_EQ(obj.at("b").type(), json::String);
		ASSERT_EQ(obj.at("c").type(), json::Null);

		EXPECT_FLOAT_EQ(obj.at("a").get<json::Number>(), 1.0);
		EXPECT_EQ(obj.at("b").get<json::String>(), "hello");
	}
}
