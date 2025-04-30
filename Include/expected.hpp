#pragma once

#include <string>
#include <string_view>

#include <outcome.hpp>

namespace terrain
{
    class error
    {
        static inline std::string kDefaultMsg = "Unknown error";

        std::string mErrorMsg{kDefaultMsg};

    public:
        explicit error(const std::string& msg): mErrorMsg(msg) {}

        const auto& get() {return mErrorMsg; }        
    };

    enum class errc
    {
        GENERIC_ERROR = 0,
    };

    inline std::error_code make_error_code(const error &) { return {static_cast<int32_t>(errc::GENERIC_ERROR), std::generic_category()}; }

    inline void outcome_throw_as_system_error_with_payload(error err)
    {
      outcome_v2::try_throw_std_exception_from_error(std::error_code(0, std::generic_category()));

      throw std::runtime_error(err.get());
    }
  

    template <typename T>
    using expected = outcome_v2::result<T, error>;
}