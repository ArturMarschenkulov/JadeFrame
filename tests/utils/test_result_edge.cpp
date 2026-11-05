#include <gtest/gtest.h>

#include "JadeFrame/utils/result.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using namespace JadeFrame;

namespace {

struct MoveOnlyError {
    explicit MoveOnlyError(int value)
        : value(value) {}

    MoveOnlyError(const MoveOnlyError&) = delete;
    auto operator=(const MoveOnlyError&) -> MoveOnlyError& = delete;
    MoveOnlyError(MoveOnlyError&&) noexcept = default;
    auto operator=(MoveOnlyError&&) noexcept -> MoveOnlyError& = default;

    int value;
};

struct ThrowingCopy {
    static inline bool should_throw = false;

    explicit ThrowingCopy(int value)
        : value(value) {}

    ThrowingCopy(const ThrowingCopy& other)
        : value(other.value) {
        if (should_throw) { throw std::runtime_error("copy failed"); }
    }

    ThrowingCopy(ThrowingCopy&& other) noexcept
        : value(other.value) {}

    auto operator=(const ThrowingCopy&) -> ThrowingCopy& = default;
    auto operator=(ThrowingCopy&&) -> ThrowingCopy& = default;

    int value;
};

} // namespace

TEST(ResultEdge, OkErrFactoriesAndAccessors) {
    Result<int, std::string> ok = Ok(12);
    Result<int, std::string> err = Err(std::string("bad"));

    ASSERT_TRUE(ok.is_ok());
    EXPECT_FALSE(ok.is_err());
    EXPECT_EQ(ok.value(), 12);
    EXPECT_EQ(ok.unwrap(), 12);

    ASSERT_TRUE(err.is_err());
    EXPECT_EQ(err.error(), "bad");
    EXPECT_EQ(err.unwrap_err(), "bad");

    EXPECT_TRUE(ok.contains(12));
    EXPECT_FALSE(ok.contains(13));
    EXPECT_TRUE(err.contains_err(std::string("bad")));
}

TEST(ResultEdge, CopyMoveAndStateTransitions) {
    Result<int, int> source(4);
    Result<int, int> destination(Failure(8));

    destination = source;
    ASSERT_TRUE(destination.is_ok());
    EXPECT_EQ(destination.value(), 4);

    source = Result<int, int>(Failure(9));
    destination = source;
    ASSERT_TRUE(destination.is_err());
    EXPECT_EQ(destination.error(), 9);

    Result<int, int> moved(Failure(10));
    Result<int, int> moved_to(11);
    moved_to = std::move(moved);
    ASSERT_TRUE(moved_to.is_err());
    EXPECT_EQ(moved_to.error(), 10);
    EXPECT_TRUE(moved.is_err());
}

TEST(ResultEdge, MoveOnlyValuesAndErrors) {
    Result<std::unique_ptr<int>, MoveOnlyError> value(std::make_unique<int>(6));
    auto                                        mapped =
        std::move(value).map([](std::unique_ptr<int> pointer) { return *pointer + 1; });
    ASSERT_TRUE(mapped.is_ok());
    EXPECT_EQ(mapped.value(), 7);

    Result<int, MoveOnlyError> error(Failure(MoveOnlyError(13)));
    auto                       mapped_error =
        std::move(error).map_err([](MoveOnlyError failure) { return failure.value + 1; });
    ASSERT_TRUE(mapped_error.is_err());
    EXPECT_EQ(mapped_error.error(), 14);
}

TEST(ResultEdge, MapAndChainingPropagateTheCorrectVariant) {
    Result<int, std::string> ok(3);
    Result<int, std::string> err(Failure(std::string("no")));

    EXPECT_EQ(ok.map([](int value) { return value * 2; }).value(), 6);
    EXPECT_EQ(err.map([](int value) { return value * 2; }).error(), "no");

    EXPECT_EQ(
        err.map_err([](const std::string& text) { return text.size(); }).error(), 2U
    );

    auto chained = ok.and_then([](int value) -> Result<std::string, std::string> {
        return std::to_string(value);
    });
    EXPECT_EQ(chained.value(), "3");

    auto short_circuited = err.and_then([](int) -> Result<std::string, std::string> {
        return std::string("unexpected");
    });
    EXPECT_EQ(short_circuited.error(), "no");

    EXPECT_EQ(ok.map_or(0, [](int value) { return value + 3; }), 6);
    EXPECT_EQ(err.map_or(9, [](int value) { return value + 3; }), 9);
    EXPECT_EQ(
        err.map_or_else([] { return 10; }, [](int value) { return value + 3; }), 10
    );
}

TEST(ResultEdge, OrAndLazyErrorRecovery) {
    Result<int, std::string> ok(1);
    Result<int, std::string> first_error(Failure(std::string("first")));
    Result<int, std::string> second_error(Failure(std::string("second")));

    EXPECT_EQ(ok.or_(Result<int, std::string>(2)).value(), 1);
    EXPECT_EQ(first_error.or_(Result<int, std::string>(2)).value(), 2);
    EXPECT_EQ(
        first_error.and_(Result<std::string, std::string>("unused")).error(), "first"
    );

    int  calls = 0;
    auto recover = [&](const std::string& error) -> Result<int, std::string> {
        ++calls;
        return error == "first" ? Result<int, std::string>(5)
                                : Result<int, std::string>(Failure(error));
    };

    EXPECT_EQ(ok.or_else(recover).value(), 1);
    EXPECT_EQ(calls, 0);
    EXPECT_EQ(first_error.or_else(recover).value(), 5);
    EXPECT_EQ(calls, 1);

    EXPECT_EQ(ok.unwrap_or(9), 1);
    EXPECT_EQ(first_error.unwrap_or(9), 9);
    EXPECT_EQ(first_error.unwrap_or_else([] { return 10; }), 10);
}

TEST(ResultEdge, OptionConversionsAndEquality) {
    Result<int, std::string> ok(8);
    Result<int, std::string> err(Failure(std::string("bad")));

    auto ok_option = ok.ok();
    auto empty_ok = err.ok();
    auto empty_err = ok.err();
    auto err_option = err.err();

    EXPECT_EQ(ok_option.unwrap(), 8);
    EXPECT_TRUE(!empty_ok.has_value());
    EXPECT_TRUE(!empty_err.has_value());
    EXPECT_EQ(err_option.unwrap(), "bad");

    EXPECT_TRUE((ok == Result<int, std::string>(8)));
    EXPECT_TRUE((err == Result<int, std::string>(Failure(std::string("bad")))));
    EXPECT_FALSE(ok == err);
}

TEST(ResultEdge, ReferenceAndVoidValues) {
    int                       value = 3;
    Result<int&, std::string> reference(value);
    reference.value() = 7;
    EXPECT_EQ(value, 7);

    Result<void, std::string> ok;
    EXPECT_TRUE(ok.is_ok());
    ok.unwrap();
    auto unit = ok.ok();
    EXPECT_TRUE(unit.has_value());

    Result<void, std::string> err(Failure(std::string("failed")));
    EXPECT_TRUE(err.is_err());
    EXPECT_EQ(err.error(), "failed");
    EXPECT_TRUE(err.map([] { return 4; }).is_err());

    auto as_ref = Result<int, std::string>(8).as_ref();
    ASSERT_TRUE(as_ref.is_ok());
    EXPECT_EQ(as_ref.value(), 8);

    Result<int, std::string> mutable_result(8);
    auto                     as_mut = mutable_result.as_mut();
    as_mut.value() = 12;
    EXPECT_EQ(mutable_result.value(), 12);

    auto mapped_void = Result<int, std::string>(8).map([](int) {});
    EXPECT_TRUE(mapped_void.is_ok());
}

TEST(ResultEdge, FailedCopyAssignmentPreservesDestination) {
    Result<ThrowingCopy, int> destination(ThrowingCopy(1));
    Result<ThrowingCopy, int> source(ThrowingCopy(2));

    ThrowingCopy::should_throw = true;
    EXPECT_THROW(destination = source, std::runtime_error);
    ThrowingCopy::should_throw = false;

    ASSERT_TRUE(destination.is_ok());
    EXPECT_EQ(destination.value().value, 1);
}

TEST(ResultEdge, WrongVariantAccessTerminates) {
    Result<int, std::string> err(Failure(std::string("bad")));
    EXPECT_DEATH({ err.value(); }, "");

    Result<int, std::string> ok(1);
    EXPECT_DEATH({ ok.error(); }, "");
}
