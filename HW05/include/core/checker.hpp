#ifndef CHECKER_HPP
#define CHECKER_HPP
#include <string>
#include <vector>
namespace core
{
std::vector<std::vector<std::string>> group_all(std::string const &path);
std::vector<std::string> group_for(std::string const &path);
} // namespace core

#endif // CHECKER_HPP
