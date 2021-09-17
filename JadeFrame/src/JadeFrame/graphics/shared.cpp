#include "pch.h"
#include "shared.h"
namespace JadeFrame {

BufferLayout::BufferElement::BufferElement(SHADER_TYPE type, const std::string & name, bool normalized)
	: name(name)
	, type(type)
	, size(SHADER_TYPE_get_size(type))
	, offset(0)
	, normalized(normalized) {
}
BufferLayout::BufferLayout(const std::initializer_list<BufferElement>&elements)
	: m_elements(elements) {
	this->calculate_offset_and_stride(m_elements);
}
auto BufferLayout::calculate_offset_and_stride(std::vector<BufferElement>&elements) -> void {
	size_t offset = 0;
	m_stride = 0;
	for (BufferElement& element : elements) {
		element.offset = offset;
		offset += element.size;
		m_stride += element.size;
	}
}
}