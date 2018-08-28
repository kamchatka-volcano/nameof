//  _   _                             __    _____
// | \ | |                           / _|  / ____|_     _
// |  \| | __ _ _ __ ___   ___  ___ | |_  | |   _| |_ _| |_
// | . ` |/ _` | '_ ` _ \ / _ \/ _ \|  _| | |  |_   _|_   _|
// | |\  | (_| | | | | | |  __/ (_) | |   | |____|_|   |_|
// |_| \_|\__,_|_| |_| |_|\___|\___/|_|    \_____|
// https://github.com/Neargye/nameof
// vesion 0.5.0
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// Copyright (c) 2016, 2018 Daniil Goncharov <neargye@gmail.com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstddef>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <string>
#include <ostream>

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304L
#  define NAMEOF_HAS_CONSTEXPR14 1
#  define NAMEOF_CONSTEXPR14 constexpr
#else
#  define NAMEOF_CONSTEXPR14 inline
#endif

#if (defined(__clang__) || defined(_MSC_VER)) || (defined(__GNUC__) && __GNUC__ >= 5)
#  define NAMEOF_TYPE_CONSTEXPR constexpr
#  define NAMEOF_TYPE_HAS_CONSTEXPR 1
#else
#  define NAMEOF_TYPE_CONSTEXPR inline
#endif

namespace nameof {

namespace detail {

namespace nstd {

template <typename T>
struct identity {
  using type = T;
};

} // namespace nstd

constexpr bool StrEquals(const char* lhs, const char* rhs, std::size_t size) {
#if defined(NAMEOF_HAS_CONSTEXPR14)
  for (std::size_t i = 0; i < size; ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }

  return true;
#else
  return (size == 0) ? (lhs[0] == rhs[0]) : (lhs[size - 1] == rhs[size - 1] && StrEquals(lhs, rhs, size - 1));
#endif
}

constexpr std::size_t StrLen(const char* str, std::size_t size = 0) {
#if defined(NAMEOF_HAS_CONSTEXPR14)
  for (; str != nullptr; ++size) {
    if (str[size] == '\0') {
      return size;
    }
  }

  return size;
#else
  return (str[size] == '\0') ? size : StrLen(str, size + 1);
#endif
}

} // namespace detail

class cstring final {
  const char* str_;
  std::size_t size_;

 public:
  constexpr cstring(const char* str, std::size_t size, std::size_t prefix = 0, std::size_t suffix = 0) noexcept
      : str_{str + prefix},
        size_{size - prefix - suffix} {}

  constexpr cstring() noexcept : cstring{nullptr, 0, 0, 0} {}

  constexpr cstring(const char* str) noexcept : cstring{str, detail::StrLen(str), 0, 0} {}

  cstring(const std::string& str) noexcept : cstring{str.data(), str.size(), 0, 0} {}

  cstring(const cstring&) = default;

  cstring(cstring&&) = default;

  cstring& operator=(const cstring&) = default;

  cstring& operator=(cstring&&) = default;

  ~cstring() = default;

  constexpr std::size_t size() const noexcept { return size_; }

  constexpr std::size_t length() const noexcept { return size_; }

  constexpr std::size_t max_size() const noexcept { return (std::numeric_limits<std::size_t>::max)(); }

  constexpr bool empty() const noexcept { return size_ == 0; }

  constexpr const char* begin() const noexcept { return str_; }

  constexpr const char* end() const noexcept { return str_ + size_; }

  constexpr const char* cbegin() const noexcept { return begin(); }

  constexpr const char* cend() const noexcept { return end(); }

  constexpr const char& operator[](std::size_t i) const { return str_[i]; }

  NAMEOF_CONSTEXPR14 const char& at(std::size_t i) const {
    if (i < size_) {
      return str_[i];
    }
    throw std::out_of_range("cstring::at()");
  }

  constexpr const char& front() const { return str_[0]; }

  constexpr const char& back() const { return str_[size_ - 1]; }

  constexpr const char* data() const noexcept { return str_; }

  constexpr cstring remove_prefix(std::size_t n) const { return {str_ + n, size_ - n}; }

  constexpr cstring add_prefix(std::size_t n) const { return {str_ - n, size_ + n}; }

  constexpr cstring remove_suffix(std::size_t n) const { return {str_, size_ - n}; }

  constexpr cstring add_suffix(std::size_t n) const { return {str_, size_ + n}; }

  constexpr cstring substr(std::size_t pos, std::size_t n) const { return {str_ + pos, n}; }

  int compare(cstring other) const {
    if (const auto result = std::char_traits<char>::compare(str_, other.str_, other.size_ < size_ ? other.size_ : size_))
      return result;

    return (size_ == other.size_) ? 0 : ((size_ < other.size_) ? -1 : 1);
  }

  friend constexpr bool operator==(cstring lhs, cstring rhs) noexcept {
    return lhs.size_ == rhs.size_ && detail::StrEquals(lhs.str_, rhs.str_, lhs.size_);
  }

  friend constexpr bool operator!=(cstring lhs, cstring rhs) noexcept {
    return !(lhs == rhs);
  }

  std::string append(cstring s) const {
    return std::string(str_, size_).append(s.str_, s.size_);
  }

  friend std::string operator+(cstring lhs, cstring rhs) {
    return std::string(lhs) + std::string(rhs);
  }

  friend std::ostream& operator<<(std::ostream& os, cstring str) {
    os.write(str.str_, str.size_);
    return os;
  }

  operator std::string() const { return std::string(str_, size_); }
};

namespace detail {

constexpr bool IsLexeme(char s) noexcept {
  return !((s >= '0' && s <= '9') || (s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z') || s == '_');
}

constexpr bool IsBracket(char s) noexcept {
  return s == ')' || s == '}' || s == '>' || s == '(' || s == '{' || s == '<';
}

#if defined(NAMEOF_HAS_CONSTEXPR14)
constexpr cstring NameofPretty(cstring name, bool with_suffix) noexcept {
  for (std::size_t i = name.size(), h = 0, s = 0; i > 0; --i) {
    if (h == 0 && IsLexeme(name[i - 1]) && !IsBracket(name[i - 1])) {
      ++s;
      continue;
    }

    if (name[i - 1] == ')' || name[i - 1] == '}') {
      ++h;
      ++s;
      continue;
    } else if (name[i - 1] == '(' || name[i - 1] == '{') {
      --h;
      ++s;
      continue;
    }

    if (h == 0) {
      name = name.remove_suffix(s);
      break;
    } else {
      ++s;
      continue;
    }
  }

  std::size_t s = 0;
  for (std::size_t i = name.size(), h = 0; i > 0; --i) {
    if (name[i - 1] == '>') {
      ++h;
      ++s;
      continue;
    } else if (name[i - 1] == '<') {
      --h;
      ++s;
      continue;
    }

    if (h == 0) {
      break;
    } else {
      ++s;
      continue;
    }
  }

  for (std::size_t i = name.size() - s; i > 0; --i) {
    if (IsLexeme(name[i - 1])) {
      return name.remove_prefix(i).remove_suffix(with_suffix ? 0 : s);
    }
  }

  return name.remove_suffix(with_suffix ? 0 : s);
}
#else
constexpr cstring RemoveSuffix(cstring name, std::size_t h = 0) noexcept {
  return (h == 0 && IsLexeme(name.back()) && !IsBracket(name.back()))
             ? RemoveSuffix(name.remove_suffix(1), h)
             : (name.back() == ')' || name.back() == '}')
                   ? RemoveSuffix(name.remove_suffix(1), h + 1)
                   : (name.back() == '(' || name.back() == '{')
                         ? RemoveSuffix(name.remove_suffix(1), h - 1)
                         : (h == 0) ? name : RemoveSuffix(name.remove_suffix(1), h);
}

constexpr std::size_t FindSuffix(cstring name, std::size_t h = 0, std::size_t s = 0) noexcept {
  return (name[name.size() - 1 - s] == '>')
             ? FindSuffix(name, h + 1, s + 1)
             : (name[name.size() - 1 - s] == '<')
                   ? FindSuffix(name, h - 1, s + 1)
                   : (h == 0) ? s : FindSuffix(name, h, s + 1);
}

constexpr cstring RemovePrefix(cstring name, const std::size_t p = 0) noexcept {
  return p == name.size() ? name : IsLexeme(name[name.size() - 1 - p])
                                       ? name.remove_prefix(name.size() - p)
                                       : RemovePrefix(name, p + 1);
}

constexpr cstring NameofPrettyImpl_(cstring name, std::size_t s, bool with_suffix) noexcept {
  return RemovePrefix(name.remove_suffix(s)).add_suffix(with_suffix ? s : 0);
}

constexpr cstring NameofPrettyImpl(cstring name, bool with_suffix) noexcept {
  return NameofPrettyImpl_(name, FindSuffix(name), with_suffix);
}

constexpr cstring NameofPretty(cstring name, bool with_suffix) noexcept {
  return NameofPrettyImpl(RemoveSuffix(name), with_suffix);
}
#endif

#if defined(_MSC_VER)
constexpr cstring RemoveSpaceSuffix(cstring name) noexcept {
  return (name.back() == ' ') ? RemoveSpaceSuffix(name.remove_suffix(1)) : name;
}

constexpr cstring RemoveClassPrefix(cstring name) noexcept {
  return (name.size() > sizeof("class") && name[0] == 'c' && name[1] == 'l' &&
          name[2] == 'a' && name[3] == 's' && name[4] == 's' && name[5] == ' ')
             ? name.remove_prefix(sizeof("class"))
             : name;
}

constexpr cstring RemoveEnumPrefix(cstring name) noexcept {
  return (name.size() > sizeof("enum") && name[0] == 'e' && name[1] == 'n' &&
          name[2] == 'u' && name[3] == 'm' && name[4] == ' ')
             ? name.remove_prefix(sizeof("enum"))
             : name;
}

constexpr cstring RemoveStructPrefix(cstring name) noexcept {
  return (name.size() > sizeof("struct") && name[0] == 's' && name[1] == 't' &&
          name[2] == 'r' && name[3] == 'u' && name[4] == 'c' && name[5] == 't' && name[6] == ' ')
             ? name.remove_prefix(sizeof("struct"))
             : name;
}

constexpr cstring NameofTypePretty(cstring name) noexcept {
  return RemoveClassPrefix(RemoveStructPrefix(RemoveEnumPrefix(RemoveSpaceSuffix(name))));
}
#elif defined(__clang__) || defined(__GNUC__)
constexpr cstring NameofTypePretty(const char* str, std::size_t size, std::size_t prefix, std::size_t suffix) noexcept {
  return {str, size, prefix, suffix + ((str[size - suffix - 1] == ' ') ? 1 : 0)};
}
#endif

template <typename T>
NAMEOF_TYPE_CONSTEXPR cstring NameofType() noexcept {
#if defined(__clang__)
  return NameofTypePretty(
      __PRETTY_FUNCTION__,
      sizeof(__PRETTY_FUNCTION__) - 1,
      sizeof("nameof::cstring nameof::detail::NameofType() [T = nameof::detail::nstd::identity<") - 1,
      sizeof(">]") - 1);
#elif defined(__GNUC__)
  return NameofTypePretty(
      __PRETTY_FUNCTION__,
      sizeof(__PRETTY_FUNCTION__) - 1,
#  if defined(NAMEOF_TYPE_HAS_CONSTEXPR)
      sizeof("constexpr nameof::cstring nameof::detail::NameofType() [with T = nameof::detail::nstd::identity<") - 1,
#  else
      sizeof("nameof::cstring nameof::detail::NameofType() [with T = nameof::detail::nstd::identity<") - 1,
#  endif
      sizeof(">]") - 1);
#elif defined(_MSC_VER)
  return NameofTypePretty(
      {__FUNCSIG__,
      sizeof(__FUNCSIG__) - 1,
      sizeof("class nameof::cstring __cdecl nameof::detail::NameofType<struct nameof::detail::nstd::identity<") - 1,
      sizeof(">>(void) noexcept") - 1});
#else
  return {};
#endif
}

} // namespace detail

template <typename T,
          typename = typename std::enable_if<!std::is_reference<T>::value>::type>
constexpr cstring Nameof(const char* name, std::size_t size, bool with_suffix) noexcept {
  return detail::NameofPretty({name, size}, with_suffix);
}

template <typename T>
constexpr cstring NameofRaw(const char* name, std::size_t size) noexcept {
  return {name, size};
}

template <typename T, typename H = detail::nstd::identity<T>>
NAMEOF_TYPE_CONSTEXPR cstring NameofType() noexcept {
  return true ? detail::NameofType<H>() : detail::NameofType<H>();
}

} // namespace nameof

// Used to obtain the simple (unqualified) string name of a variable, member, function, macros.
#define NAMEOF(...) ::nameof::Nameof<decltype(__VA_ARGS__)>(#__VA_ARGS__, (sizeof(#__VA_ARGS__) / sizeof(char)) - 1, false)

// Used to obtain the full string name of a variable, member, function, macros.
#define NAMEOF_FULL(...) ::nameof::Nameof<decltype(__VA_ARGS__)>(#__VA_ARGS__, (sizeof(#__VA_ARGS__) / sizeof(char)) - 1, true)

// Used to obtain the raw string name of a variable, member, function, macros.
#define NAMEOF_RAW(...) ::nameof::NameofRaw<decltype(__VA_ARGS__)>(#__VA_ARGS__, (sizeof(#__VA_ARGS__) / sizeof(char)) - 1)

// Used to obtain the string name of a type.
#define NAMEOF_TYPE(...) ::nameof::NameofType<decltype(__VA_ARGS__)>()
#define NAMEOF_TYPE_T(...) ::nameof::NameofType<__VA_ARGS__>()
