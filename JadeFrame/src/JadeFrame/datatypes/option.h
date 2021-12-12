#pragma once
#include "JadeFrame/defines.h"

namespace JadeFrame {
template<typename T>
class Option {
public:
	using ValueType = T;

	Option() = default;
	Option(const Option& o)
		: m_has_value(o.m_has_value) {
		if(o.m_has_value) {
			//new(&m_data T(other.value());
		}
	}
	Option(T&& value)
		: m_has_value(true) {
		m_data = value;
	}
	~Option() {
		if(m_has_value) {
			~T();
			m_has_value = false;
		}
	}

private:
	T m_data;
	bool m_has_value = false;

};
}