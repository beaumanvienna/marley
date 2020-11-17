#pragma once

template <typename T, typename U>
class SCREEN_InitConstMap
{
private:
    std::map<T, U> m_map;
public:
    SCREEN_InitConstMap(const T& key, const U& val)
    {
        m_map[key] = val;
    }

    SCREEN_InitConstMap<T, U>& operator()(const T& key, const U& val)
    {
        m_map[key] = val;
        return *this;
    }

    operator std::map<T, U>()
    {
        return m_map;
    }
};
