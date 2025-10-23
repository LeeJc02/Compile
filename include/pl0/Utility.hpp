#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace pl0 {

[[nodiscard]] std::string read_file_utf8(const std::filesystem::path& path);

[[nodiscard]] std::vector<std::string_view> split_lines(std::string_view text);

void trim_trailing_cr(std::string& line);

}  // namespace pl0

