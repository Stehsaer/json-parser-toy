add_rules("mode.debug", "mode.release")
add_requires("gtest 1.17.0")

set_policy("build.c++.modules", true)
set_languages("c++23")

target("json")
	set_kind("static")

	add_files("json.cppm", "module/**.cppm", {public = true})
	add_files("src/**.cpp")

target("json-test")
	set_kind("binary")

	add_files("test/*.cpp")
	add_packages("gtest")
	add_deps("json")