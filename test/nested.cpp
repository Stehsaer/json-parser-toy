#include <gtest/gtest.h>

import json;

TEST(Nested, Arrays)  // NOLINT
{
	{
		const auto result = json::decode("[[]]");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& outer = result->get<json::Array>();
		ASSERT_EQ(outer.size(), 1);
		ASSERT_EQ(outer[0].type(), json::Array);
		EXPECT_EQ(outer[0].get<json::Array>().size(), 0);
	}

	{
		const auto result = json::decode("[[], []]");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& outer = result->get<json::Array>();
		ASSERT_EQ(outer.size(), 2);
		for (const auto& inner : outer)
		{
			ASSERT_EQ(inner.type(), json::Array);
			EXPECT_EQ(inner.get<json::Array>().size(), 0);
		}
	}

	{
		const auto result = json::decode("[[1.0, 2.0], [3.0, 4.0], []]");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& outer = result->get<json::Array>();
		ASSERT_EQ(outer.size(), 3);
		ASSERT_EQ(outer[0].type(), json::Array);
		ASSERT_EQ(outer[1].type(), json::Array);
		ASSERT_EQ(outer[2].type(), json::Array);

		const auto& first = outer[0].get<json::Array>();
		const auto& second = outer[1].get<json::Array>();
		const auto& third = outer[2].get<json::Array>();

		ASSERT_EQ(first.size(), 2);
		ASSERT_EQ(second.size(), 2);
		EXPECT_EQ(third.size(), 0);

		EXPECT_FLOAT_EQ(first[0].get<json::Number>(), 1.0);
		EXPECT_FLOAT_EQ(first[1].get<json::Number>(), 2.0);
		EXPECT_FLOAT_EQ(second[0].get<json::Number>(), 3.0);
		EXPECT_FLOAT_EQ(second[1].get<json::Number>(), 4.0);
	}

	{
		const auto result = json::decode("[1.0, [2.0, [3.0, [4.0]]]]");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& l0 = result->get<json::Array>();
		ASSERT_EQ(l0.size(), 2);
		ASSERT_EQ(l0[0].type(), json::Number);
		EXPECT_FLOAT_EQ(l0[0].get<json::Number>(), 1.0);

		const auto& l1 = l0[1].get<json::Array>();
		ASSERT_EQ(l1.size(), 2);
		EXPECT_FLOAT_EQ(l1[0].get<json::Number>(), 2.0);

		const auto& l2 = l1[1].get<json::Array>();
		ASSERT_EQ(l2.size(), 2);
		EXPECT_FLOAT_EQ(l2[0].get<json::Number>(), 3.0);

		const auto& l3 = l2[1].get<json::Array>();
		ASSERT_EQ(l3.size(), 1);
		EXPECT_FLOAT_EQ(l3[0].get<json::Number>(), 4.0);
	}

	{
		const auto result = json::decode(R"([null, true, false, "text", 1.5, [], {}])");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& arr = result->get<json::Array>();
		ASSERT_EQ(arr.size(), 7);
		EXPECT_EQ(arr[0].type(), json::Null);
		EXPECT_EQ(arr[1].type(), json::Bool);
		EXPECT_EQ(arr[2].type(), json::Bool);
		EXPECT_EQ(arr[3].type(), json::String);
		EXPECT_EQ(arr[4].type(), json::Number);
		EXPECT_EQ(arr[5].type(), json::Array);
		EXPECT_EQ(arr[6].type(), json::Object);

		EXPECT_EQ(arr[1].get<json::Bool>(), true);
		EXPECT_EQ(arr[2].get<json::Bool>(), false);
		EXPECT_EQ(arr[3].get<json::String>(), "text");
		EXPECT_FLOAT_EQ(arr[4].get<json::Number>(), 1.5);
		EXPECT_EQ(arr[5].get<json::Array>().size(), 0);
		EXPECT_EQ(arr[6].get<json::Object>().size(), 0);
	}

	{
		// Whitespace (including newlines and tabs) around nested values.
		const auto result = json::decode(R"([
	[1, 2],
	[3, 4]
])");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& outer = result->get<json::Array>();
		ASSERT_EQ(outer.size(), 2);
		ASSERT_EQ(outer[0].type(), json::Array);
		ASSERT_EQ(outer[1].type(), json::Array);

		const auto& first = outer[0].get<json::Array>();
		const auto& second = outer[1].get<json::Array>();
		ASSERT_EQ(first.size(), 2);
		ASSERT_EQ(second.size(), 2);
		EXPECT_FLOAT_EQ(first[0].get<json::Number>(), 1.0);
		EXPECT_FLOAT_EQ(first[1].get<json::Number>(), 2.0);
		EXPECT_FLOAT_EQ(second[0].get<json::Number>(), 3.0);
		EXPECT_FLOAT_EQ(second[1].get<json::Number>(), 4.0);
	}
}

TEST(Nested, Objects)  // NOLINT
{
	{
		const auto result = json::decode(R"({"a": {}})");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& obj = result->get<json::Object>();
		ASSERT_EQ(obj.size(), 1);
		ASSERT_TRUE(obj.contains("a"));
		ASSERT_EQ(obj.at("a").type(), json::Object);
		EXPECT_EQ(obj.at("a").get<json::Object>().size(), 0);
	}

	{
		const auto result = json::decode(R"({"a": {"b": {}}})");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& outer = result->get<json::Object>();
		ASSERT_EQ(outer.size(), 1);
		ASSERT_TRUE(outer.contains("a"));
		ASSERT_EQ(outer.at("a").type(), json::Object);

		const auto& inner = outer.at("a").get<json::Object>();
		ASSERT_EQ(inner.size(), 1);
		ASSERT_TRUE(inner.contains("b"));
		ASSERT_EQ(inner.at("b").type(), json::Object);
		EXPECT_EQ(inner.at("b").get<json::Object>().size(), 0);
	}

	{
		const auto result = json::decode(R"({"a": {"b": {"c": 1.5}}})");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& a = result->get<json::Object>();
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.at("a").type(), json::Object);

		const auto& b = a.at("a").get<json::Object>();
		ASSERT_EQ(b.size(), 1);
		ASSERT_EQ(b.at("b").type(), json::Object);

		const auto& c = b.at("b").get<json::Object>();
		ASSERT_EQ(c.size(), 1);
		ASSERT_TRUE(c.contains("c"));
		ASSERT_EQ(c.at("c").type(), json::Number);
		EXPECT_FLOAT_EQ(c.at("c").get<json::Number>(), 1.5);
	}

	{
		const auto result = json::decode(R"({"a": {"b": 1.0}, "c": {"d": 2.0}, "e": {}})");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& obj = result->get<json::Object>();
		ASSERT_EQ(obj.size(), 3);
		ASSERT_TRUE(obj.contains("a"));
		ASSERT_TRUE(obj.contains("c"));
		ASSERT_TRUE(obj.contains("e"));

		const auto& a = obj.at("a").get<json::Object>();
		const auto& c = obj.at("c").get<json::Object>();
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(c.size(), 1);
		ASSERT_EQ(a.at("b").type(), json::Number);
		ASSERT_EQ(c.at("d").type(), json::Number);
		EXPECT_FLOAT_EQ(a.at("b").get<json::Number>(), 1.0);
		EXPECT_FLOAT_EQ(c.at("d").get<json::Number>(), 2.0);
		EXPECT_EQ(obj.at("e").get<json::Object>().size(), 0);
	}

	{
		const auto result = json::decode(
			R"({"array": [1.0, 2.0], "object": {"x": true, "y": null}, "number": 3.5, "string": "text"})"
		);
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& obj = result->get<json::Object>();
		ASSERT_EQ(obj.size(), 4);

		const auto& array = obj.at("array").get<json::Array>();
		ASSERT_EQ(array.size(), 2);
		EXPECT_FLOAT_EQ(array[0].get<json::Number>(), 1.0);
		EXPECT_FLOAT_EQ(array[1].get<json::Number>(), 2.0);

		const auto& object = obj.at("object").get<json::Object>();
		ASSERT_EQ(object.size(), 2);
		ASSERT_EQ(object.at("x").type(), json::Bool);
		ASSERT_EQ(object.at("y").type(), json::Null);
		EXPECT_EQ(object.at("x").get<json::Bool>(), true);

		EXPECT_EQ(obj.at("number").type(), json::Number);
		EXPECT_FLOAT_EQ(obj.at("number").get<json::Number>(), 3.5);
		EXPECT_EQ(obj.at("string").get<json::String>(), "text");
	}
}

TEST(Nested, MixedContainers)  // NOLINT
{
	{
		const auto result = json::decode(R"([{"a": [1.0, 2.0]}, {"b": {"c": [3.0, 4.0]}}])");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& arr = result->get<json::Array>();
		ASSERT_EQ(arr.size(), 2);

		ASSERT_EQ(arr[0].type(), json::Object);
		const auto& first = arr[0].get<json::Object>();
		ASSERT_EQ(first.size(), 1);
		const auto& a = first.at("a").get<json::Array>();
		ASSERT_EQ(a.size(), 2);
		EXPECT_FLOAT_EQ(a[0].get<json::Number>(), 1.0);
		EXPECT_FLOAT_EQ(a[1].get<json::Number>(), 2.0);

		ASSERT_EQ(arr[1].type(), json::Object);
		const auto& second = arr[1].get<json::Object>();
		ASSERT_EQ(second.size(), 1);
		const auto& b = second.at("b").get<json::Object>();
		ASSERT_EQ(b.size(), 1);
		const auto& c = b.at("c").get<json::Array>();
		ASSERT_EQ(c.size(), 2);
		EXPECT_FLOAT_EQ(c[0].get<json::Number>(), 3.0);
		EXPECT_FLOAT_EQ(c[1].get<json::Number>(), 4.0);
	}

	{
		const auto result = json::decode(R"({"a": [{"b": [1.0, {"c": null}]}]})");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Object);

		const auto& outer = result->get<json::Object>();
		ASSERT_EQ(outer.size(), 1);
		const auto& outer_array = outer.at("a").get<json::Array>();
		ASSERT_EQ(outer_array.size(), 1);

		const auto& inner = outer_array[0].get<json::Object>();
		ASSERT_EQ(inner.size(), 1);
		const auto& inner_array = inner.at("b").get<json::Array>();
		ASSERT_EQ(inner_array.size(), 2);
		EXPECT_FLOAT_EQ(inner_array[0].get<json::Number>(), 1.0);

		const auto& leaf = inner_array[1].get<json::Object>();
		ASSERT_EQ(leaf.size(), 1);
		ASSERT_EQ(leaf.at("c").type(), json::Null);
	}

	{
		// Deeply alternating array/object nesting.
		const auto result = json::decode(R"([{"a": [{"b": [{"c": "deep"}]}]}])");
		ASSERT_TRUE(result);
		ASSERT_EQ(result->type(), json::Array);

		const auto& l0 = result->get<json::Array>();
		ASSERT_EQ(l0.size(), 1);
		const auto& o0 = l0[0].get<json::Object>();
		const auto& l1 = o0.at("a").get<json::Array>();
		ASSERT_EQ(l1.size(), 1);
		const auto& o1 = l1[0].get<json::Object>();
		const auto& l2 = o1.at("b").get<json::Array>();
		ASSERT_EQ(l2.size(), 1);
		const auto& o2 = l2[0].get<json::Object>();
		ASSERT_EQ(o2.at("c").type(), json::String);
		EXPECT_EQ(o2.at("c").get<json::String>(), "deep");
	}
}
