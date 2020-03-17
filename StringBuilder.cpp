//
// Created by Steve on 3/15/2020.
//

#include <string>
#include <deque>
#include <numeric>
#include <iostream>
#include <vector>

#include "noncopyable.h"

template<typename T>
class StringBuilder : public sun::noncopyable {
    typedef std::basic_string<T> string_t;
    typedef std::deque<string_t> container_t;
    typedef typename string_t::size_type size_type;
    container_t data_;
    size_type total_;
public:
    StringBuilder(string_t &&s) {
        if (!s.empty()) {
            data_.push_back(std::forward(s));
        }
        total_ = s.size();
    }

    StringBuilder() {
        total_ = 0;
    }

    StringBuilder &Append(string_t &&s) noexcept {
        total_ += s.size();
        data_.push_back(std::move(s));
        return *this;
    }

    StringBuilder &Append(const string_t &s) {
        total_ += s.size();
        data_.push_back(std::move(s));
        return *this;
    }

    template<typename InputIterator>
    StringBuilder &Add(const InputIterator &start, const InputIterator &end) {
        for (auto iter = start; iter != end; ++iter) {
            Append(*iter);
        }
        return *this;
    }

    StringBuilder &AppendLine() {
        static T newLine[]{10, 0};
        data_.push_back(newLine);
        ++total_;
        return *this;
    }

    StringBuilder &AppendLine(string_t &&s) {
        Append(std::forward<string_t>(s));
        return AppendLine();
    }

    [[nodiscard]] string_t ToString() const {
        string_t res;
        res.reserve(total_ + 1);
        for (auto iter = data_.begin(); iter != data_.end(); ++iter) {
            res += *iter;
        }
        return res;
    }

    [[nodiscard]] string_t Join(const string_t &delimiter) const {
        if (delimiter.empty()) {
            return ToString();
        }
        string_t res;
        if (data_.empty()) {
            return res;
        }
        res.reserve(delimiter.size() * (data_.size() - 1) + total_ + 1);
        struct Joiner {
            const string_t &s_;

            explicit Joiner(const string_t &s) : s_(s) {}

            string_t &operator()(string_t &lhs, const string_t &rhs) {
                lhs += s_;
                lhs += rhs;
                return lhs;
            }
        } joiner(delimiter);
        auto iter = data_.begin();
        res += *iter;
        return std::accumulate(++iter, data_.end(), res, joiner);
    }
};

#ifdef STRING_BUILDER_TEST
int main() {
    // 8-bit characters
    StringBuilder<char> ansi;
    ansi.Append("Hello").Append(" ").AppendLine("World!");
    std::cout << ansi.ToString();

    // Wide characters
    std::vector<std::wstring> cargoCult{
            L"A", L" cargo", L" cult", L" is", L" a",
            L" kind", L" of", L" Melanesian",
            L" millenarian", L" movement",
            // many more lines here...
            L" applied", L" retroactively", L" to", L" movements",
            L" in", L" a", L" much", L" earlier", L" era.\n"
    };
    StringBuilder<wchar_t> wide;
    wide.Add(cargoCult.begin(), cargoCult.end()).AppendLine();
    std::wcout << wide.ToString() << std::endl;
    std::wcout << wide.Join(L" _\n") << std::endl;
    return 0;
}
#endif