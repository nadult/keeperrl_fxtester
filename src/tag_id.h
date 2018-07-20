#pragma once

#include "base.h"

template <class TTag, class Base = unsigned> class TagId {
  public:
	using Tag = TTag;
	static_assert(std::is_unsigned<Base>::value);
	static_assert(sizeof(Base) <= 4);

	explicit TagId(int idx) : m_idx(idx) { PASSERT(idx >= 0); }
	TagId(int idx, NoAssertsTag) : m_idx(idx) {}

	explicit operator bool() const = delete;
	operator int() const { return m_idx; }
	int index() const { return m_idx; }

	bool operator<(TagId rhs) const { return m_idx < rhs.m_idx; }
	bool operator==(TagId rhs) const { return m_idx == rhs.m_idx; }

  private:
	Base m_idx;
};
