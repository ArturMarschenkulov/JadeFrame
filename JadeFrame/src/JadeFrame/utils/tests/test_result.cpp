#include <gtest/gtest.h>
#include "JadeFrame/utils/result.h"
using namespace JadeFrame;

struct Student {
    std::string name;
    int         age;

    [[nodiscard]] auto get_name() const -> Result<const std::string&, std::string> {
        if (age < 18) {
            return Failure(std::string("Too young"));
        } else {
            return Result<const std::string&, std::string>(name);
        }
    }
};

auto pp() -> void {}
enum class Version {
    VER1,
    VER2
};

auto parse_version(const int& version) -> Result<Version, const char*> {
    if (version == 1) {
        return Result<Version, const char*>(Version::VER2);
    } else if (version == 2) {
        return Result<Version, const char*>(Version::VER2);
    } else {
        auto f = Failure("invalid version");
        return f;
    }
}

auto to_double(const char* str) noexcept -> Result<double, std::errc> {
    auto* last_entry = static_cast<char*>(nullptr);

    errno = 0;
    const double result = std::strtod(str, &last_entry);

    if (errno != 0) {
        // Returns an error value
        return Failure(static_cast<std::errc>(errno));
    }
    // Returns a value
    return result;
}

auto general_usage() { auto res = parse_version(1); }

TEST(Result, General) {
    auto res = parse_version(1);

    Result<double, std::errc> possible_double = to_double("123.123");
    // if (possible_double.is_ok()) {
    //     double d = possible_double.unwrap();
    //     EXPECT_EQ(d, 123.123);
    // } else {
    //     std::errc err = possible_double.unwrap_err();
    //     EXPECT_EQ(err, std::errc::invalid_argument);
    // }
}