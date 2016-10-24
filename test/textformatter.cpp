#include "catch.hpp"

#include <textformatter.h>

using namespace newsbeuter;

TEST_CASE("lines marked as `wrappable` are wrapped to fit width",
          "[textformatter]") {
	textformatter fmt;

	fmt.add_lines(
		{
			std::make_pair(wrappable, "this one is going to be wrapped"),
			std::make_pair(softwrappable, "this one is going to be wrapped at the window border"),
			std::make_pair(nonwrappable, "this one is going to be preserved even though it's much longer")
		});

	SECTION("formatting to plain text") {
		const std::string expected =
			"this one \n"
			"is going \n"
			"to be \n"
			"wrapped\n"
			"this one is going to be wrapped at the \n"
			"window border\n"
			"this one is going to be preserved even though it's much longer\n";
		REQUIRE(fmt.format_text_plain(10, 40) == expected);
	}

	SECTION("formatting to list") {
		const std::string expected =
			"{list"
				"{listitem text:\"this one \"}"
				"{listitem text:\"is going \"}"
				"{listitem text:\"to be \"}"
				"{listitem text:\"wrapped\"}"
				"{listitem text:\"this one is going to be wrapped at the \"}"
				"{listitem text:\"window border\"}"
				"{listitem text:\"this one is going to be preserved even though it's much longer\"}"
			"}";
		REQUIRE(fmt.format_text_to_list(nullptr, "", 10, 40) == expected);
	}
}

TEST_CASE("regex manager is used by format_text_to_list if one is passed",
          "[textformatter]") {
	textformatter fmt;

	fmt.add_line(wrappable, "Highlight me please!");

	regexmanager rxmgr;
	// the choice of green text on red background does not reflect my personal
	// taste (or lack thereof) :)
	rxmgr.handle_action("highlight", {"article", "please", "green", "default"});

	const std::string expected =
		"{list"
			"{listitem text:\"Highlight me <0>please</>!\"}"
		"}";

	REQUIRE(fmt.format_text_to_list(&rxmgr, "article", 100) == expected);
}

TEST_CASE("<hr> is rendered properly", "[textformatter]") {
	textformatter fmt;

	fmt.add_line(hr, "");

	SECTION("width = 10") {
		const std::string expected =
			"\n"
			" -------- "
			"\n"
			"\n";
		REQUIRE(fmt.format_text_plain(10) == expected);
	}
}

TEST_CASE("wrappable sequences longer then format width are forced-wrapped",
          "[textformatter]") {
	textformatter fmt;
	fmt.add_line(wrappable, "0123456789101112");
	fmt.add_line(softwrappable, "0123456789101112");
	fmt.add_line(nonwrappable, "0123456789101112");

	const std::string expected =
		"01234\n"
		"56789\n"
		"10111\n"
		"2\n"
		"0123456789\n"
		"101112\n"
		"0123456789101112\n";
	REQUIRE(fmt.format_text_plain(5, 10) == expected);
}

TEST_CASE("when wrapping, spaces at the beginning of lines are dropped",
          "[textformatter]") {
	textformatter fmt;
	fmt.add_line(wrappable, "just a test");

	const std::string expected =
		"just\n"
		"a \n"
		"test\n";
	REQUIRE(fmt.format_text_plain(4) == expected);
}

TEST_CASE("softwrappable lines are wrapped by format_text_to_list if "
          "total_width != 0", "[textformatter]") {
	textformatter fmt;
	fmt.add_line(softwrappable, "just a test");
	const size_t wrap_width = 100;
	regexmanager * rxman = nullptr;
	const std::string location = "";

	SECTION("total_width == 4") {
		const std::string expected =
			"{list"
				"{listitem text:\"just\"}"
				"{listitem text:\"a \"}"
				"{listitem text:\"test\"}"
			"}";
		REQUIRE(fmt.format_text_to_list(rxman, location, wrap_width, 4) == expected);
	}

	SECTION("total_width == 0") {
		const std::string expected =
			"{list"
				"{listitem text:\"just a test\"}"
			"}";
		REQUIRE(fmt.format_text_to_list(rxman, location, wrap_width, 0) == expected);
	}
}
