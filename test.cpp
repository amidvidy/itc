#include <memory>
#include <utility>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "itc.hpp"

TEST_CASE("Normalizing an itc::id", "[itc::id]") {
    itc::id id;
    REQUIRE(id.is_leaf());
    REQUIRE(id.leaf() == itc::detail::zero_or_one::zero);

    auto child1 = std::make_unique<itc::id>();
    auto child2 = std::make_unique<itc::id>();

    id.set_left(std::move(child1));
    id.set_right(std::move(child2));

    REQUIRE(!id.is_leaf());
    REQUIRE(id.internal().has_left());
    REQUIRE(id.internal().get_left()->is_leaf());
    REQUIRE(id.internal().get_left()->leaf() == itc::detail::zero_or_one::zero);
    REQUIRE(id.internal().has_right());
    REQUIRE(id.internal().get_right()->is_leaf());
    REQUIRE(id.internal().get_right()->leaf() == itc::detail::zero_or_one::zero);

    id.normalize();
    REQUIRE(id.is_leaf());
    REQUIRE(id.leaf() == itc::detail::zero_or_one::zero);
}
