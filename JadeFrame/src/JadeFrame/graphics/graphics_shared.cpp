#include "pch.h"
#include "graphics_shared.h"
namespace JadeFrame {

VertexAttribute::VertexAttribute(const std::string& name, SHADER_TYPE type,  bool normalized)
	: name(name)
	, type(type)
	, size(SHADER_TYPE_get_size(type))
	, offset(0)
	, normalized(normalized) {
}

VertexFormat::VertexFormat(const std::initializer_list<VertexAttribute>& attributes)
	: m_attributes(attributes) {
	this->calculate_offset_and_stride(m_attributes);
}
auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void {
	size_t offset = 0;
	m_stride = 0;
	for (VertexAttribute& attribute : attributes) {
		attribute.offset = offset;
		offset += attribute.size;
		m_stride += attribute.size;
	}
}
}