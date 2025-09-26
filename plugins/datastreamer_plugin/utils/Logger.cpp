#pragma once
/**
 * @file DDSWrapper.hpp
 * @brief Provides a wrapper for DDS functionality in the airbotix project.
 */
#include <string>
#include <ace/OS_NS_sys_time.h>
#include <ace/Log_Msg.h>

// Give colors to the output.
#define RESET "\033[0m"
#define RED "\033[31m"
#define MAGENTA "\033[35m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"


// ---------------------------------------------------------------------------
// Core Logging
// ---------------------------------------------------------------------------
#define DDS_DEBUG(prefix, fmt, ...) \
    ACE_DEBUG((LM_DEBUG, BLUE "%s [%s] [DEBUG] " fmt RESET "\n", \
               airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__))

#define DDS_INFO(prefix, fmt, ...) \
    ACE_DEBUG((LM_INFO, GREEN "%s [%s] [INFO] " fmt RESET "\n", \
               airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__))

#define DDS_WARNING(prefix, fmt, ...) \
    ACE_DEBUG((LM_WARNING, YELLOW "%s [%s] [WARNING] " fmt RESET "\n", \
               airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__))

#define DDS_ERROR(prefix, fmt, ...) \
    ACE_DEBUG((LM_ERROR, RED "%s [%s] [ERROR] " fmt RESET "\n", \
               airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__))

#define DDS_CRITICAL(prefix, fmt, ...) \
    ACE_DEBUG((LM_CRITICAL, MAGENTA "%s [%s] [CRITICAL] " fmt RESET "\n", \
               airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__))

// ---------------------------------------------------------------------------
// Log ONCE (thread-safe using call_once)
// ---------------------------------------------------------------------------
#define DDS_DEBUG_ONCE(prefix, fmt, ...) \
do { \
    static std::once_flag flag; \
    std::call_once(flag, [&]() { \
        ACE_DEBUG((LM_DEBUG, BLUE "%s [%s] [DEBUG] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    }); \
} while(0)

#define DDS_INFO_ONCE(prefix, fmt, ...) \
do { \
    static std::once_flag flag; \
    std::call_once(flag, [&]() { \
        ACE_DEBUG((LM_INFO, GREEN "%s [%s] [INFO] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    }); \
} while(0)

#define DDS_WARNING_ONCE(prefix, fmt, ...) \
do { \
    static std::once_flag flag; \
    std::call_once(flag, [&]() { \
        ACE_DEBUG((LM_WARNING, YELLOW "%s [%s] [WARNING] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    }); \
} while(0)

#define DDS_ERROR_ONCE(prefix, fmt, ...) \
do { \
    static std::once_flag flag; \
    std::call_once(flag, [&]() { \
        ACE_DEBUG((LM_ERROR, RED "%s [%s] [ERROR] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    }); \
} while(0)

#define DDS_CRITICAL_ONCE(prefix, fmt, ...) \
do { \
    static std::once_flag flag; \
    std::call_once(flag, [&]() { \
        ACE_DEBUG((LM_CRITICAL, MAGENTA "%s [%s] [CRITICAL] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    }); \
} while(0)

// ---------------------------------------------------------------------------
// Log EVERY N (thread-safe using atomic counter)
// ---------------------------------------------------------------------------
#define DDS_DEBUG_EVERY_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if ((count.fetch_add(1, std::memory_order_relaxed) % (n)) == 0) { \
        ACE_DEBUG((LM_DEBUG, BLUE "%s [%s] [DEBUG] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_INFO_EVERY_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if ((count.fetch_add(1, std::memory_order_relaxed) % (n)) == 0) { \
        ACE_DEBUG((LM_INFO, GREEN "%s [%s] [INFO] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_WARNING_EVERY_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if ((count.fetch_add(1, std::memory_order_relaxed) % (n)) == 0) { \
        ACE_DEBUG((LM_WARNING, YELLOW "%s [%s] [WARNING] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_ERROR_EVERY_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if ((count.fetch_add(1, std::memory_order_relaxed) % (n)) == 0) { \
        ACE_DEBUG((LM_ERROR, RED "%s [%s] [ERROR] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_CRITICAL_EVERY_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if ((count.fetch_add(1, std::memory_order_relaxed) % (n)) == 0) { \
        ACE_DEBUG((LM_CRITICAL, MAGENTA "%s [%s] [CRITICAL] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

// ---------------------------------------------------------------------------
// Log FIRST N
// ---------------------------------------------------------------------------
#define DDS_DEBUG_FIRST_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if (count.fetch_add(1, std::memory_order_relaxed) < (n)) { \
        ACE_DEBUG((LM_DEBUG, BLUE "%s [%s] [DEBUG] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_INFO_FIRST_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if (count.fetch_add(1, std::memory_order_relaxed) < (n)) { \
        ACE_DEBUG((LM_INFO, GREEN "%s [%s] [INFO] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_WARNING_FIRST_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if (count.fetch_add(1, std::memory_order_relaxed) < (n)) { \
        ACE_DEBUG((LM_WARNING, YELLOW "%s [%s] [WARNING] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_ERROR_FIRST_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if (count.fetch_add(1, std::memory_order_relaxed) < (n)) { \
        ACE_DEBUG((LM_ERROR, RED "%s [%s] [ERROR] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

#define DDS_CRITICAL_FIRST_N(n, prefix, fmt, ...) \
do { \
    static std::atomic<int> count{0}; \
    if (count.fetch_add(1, std::memory_order_relaxed) < (n)) { \
        ACE_DEBUG((LM_CRITICAL, MAGENTA "%s [%s] [CRITICAL] " fmt RESET "\n", \
                   airbotix::dds::current_timestamp().c_str(), prefix __VA_OPT__(,) __VA_ARGS__)); \
    } \
} while(0)

namespace airbotix::dds
{
    /// Map DDS_DEBUG_LEVEL env var to ACE log mask
    inline unsigned int mask_from_env(const char *env_var_name = "DDS_DEBUG_LEVEL") {
        const char *env_val = std::getenv(env_var_name);

        // Always include ERROR + CRITICAL
        unsigned int mask = LM_ERROR | LM_CRITICAL | LM_INFO;

        if (!env_val) {
            return mask; // default if not set
        }

        int level = std::atoi(env_val);
        switch (level) {
            case 1:
                mask |= LM_WARNING;
                break;
            case 2:
                mask |= LM_WARNING | LM_INFO;
                break;
            case 3:
                mask |= LM_WARNING | LM_INFO | LM_DEBUG;
                break;
            default:
                // if invalid, fallback to just error + critical
                break;
        }

        return mask;
    }

    /// Initialize logging from env
    inline void init_logging_from_env(const char *env_var_name = "DDS_DEBUG_LEVEL") {
        unsigned int mask = mask_from_env(env_var_name);
        ACE_Log_Msg::instance()->priority_mask(mask, ACE_Log_Msg::PROCESS);
    }

    inline std::string current_timestamp() // Add inline
    {
        ACE_Time_Value now = ACE_OS::gettimeofday();

        std::string result;
        result.reserve(32); // Increase reserve size

        result += '[';
        result += std::to_string(now.sec());
        result += '.';

        // More concise microsecond formatting
        char usec_str[7];
        snprintf(usec_str, sizeof(usec_str), "%06ld", now.usec());
        result += usec_str;
        result += ']';

        return result;
    }
}