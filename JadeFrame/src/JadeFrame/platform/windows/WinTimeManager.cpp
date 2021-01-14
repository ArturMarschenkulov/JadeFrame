#include "WinTimeManager.h"
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib")
WinTimeManager::WinTimeManager() {
	timeBeginPeriod(1);
	uint64_t frequency;
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
		m_has_performance_counter = true;
		m_frequency = frequency;
	} else {
		m_has_performance_counter = true;
		m_frequency = 1000;
	}

	m_offset = get_timer_value();
}

auto WinTimeManager::get_time() {
	return static_cast<double>(get_timer_value() - m_offset) / get_timer_frequency();
}

auto WinTimeManager::get_timer_value() -> uint64_t {
	uint64_t value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return value;
}

auto WinTimeManager::get_timer_frequency() -> uint64_t {
	return m_frequency;
}

#include <string>
#include "../../math/Vec2.h"
#include "../../graphics/Mesh.h"

namespace EMouseCursor {
enum Type {
	/** Causes no mouse cursor to be visible */
	None,

	/** Default cursor (arrow) */
	Default,

	/** Text edit beam */
	TextEditBeam,

	/** Resize horizontal */
	ResizeLeftRight,

	/** Resize vertical */
	ResizeUpDown,

	/** Resize diagonal */
	ResizeSouthEast,

	/** Resize other diagonal */
	ResizeSouthWest,

	/** MoveItem */
	CardinalCross,

	/** Target Cross */
	Crosshairs,

	/** Hand cursor */
	Hand,

	/** Grab Hand cursor */
	GrabHand,

	/** Grab Hand cursor closed */
	GrabHandClosed,

	/** a circle with a diagonal line through it */
	SlashedCircle,

	/** Eye-dropper cursor for picking colors */
	EyeDropper,

	/** Custom cursor shape for platforms that support setting a native cursor shape. Same as specifying None if not set. */
	Custom,

	/** Number of cursors we support */
	TotalCursorCount
};
}
class FWindowsCursor/* : public ICursor*/ {
public:

	FWindowsCursor();

	virtual ~FWindowsCursor();

	virtual void* create_cursor_from_file(const std::string& InPathToCursorWithoutExtension, Vec2 HotSpot);

	virtual bool is_create_cursor_from_RGBA_buffer_supported() const {
		return true;
	}

	virtual void* create_cursor_from_RGBA_buffer(const Color* Pixels, int32_t Width, int32_t Height, Vec2 InHotSpot);

	virtual Vec2 get_position() const;

	virtual void set_position(const int32_t X, const int32_t Y);

	virtual void set_type(const EMouseCursor::Type InNewCursor);

	virtual EMouseCursor::Type get_type() const {
		return CurrentType;
	}

	virtual void get_size(int32_t& Width, int32_t& Height) const;

	virtual void show(bool bShow);

	virtual void lock(const RECT* const Bounds);

	virtual void set_type_shape(EMouseCursor::Type InCursorType, void* CursorHandle);

private:

	EMouseCursor::Type CurrentType;

	/** Cursors */
	HCURSOR CursorHandles[EMouseCursor::TotalCursorCount];

	/** Override Cursors */
	HCURSOR CursorOverrideHandles[EMouseCursor::TotalCursorCount];
};