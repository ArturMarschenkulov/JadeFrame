#include "GLBuffer.h"

#include "Mesh.h"

static auto SHADER_DATA_TYPE_get_size(const SHADER_DATA_TYPE type) -> uint32_t {
	switch (type) {
	case SHADER_DATA_TYPE::FLOAT:	return 4;
	case SHADER_DATA_TYPE::FLOAT_2:	return 4 * 2;
	case SHADER_DATA_TYPE::FLOAT_3:	return 4 * 3;
	case SHADER_DATA_TYPE::FLOAT_4:	return 4 * 4;
	case SHADER_DATA_TYPE::MAT_3:	return 4 * 3 * 3;
	case SHADER_DATA_TYPE::MAT_4:	return 4 * 4 * 4;
	case SHADER_DATA_TYPE::INT:		return 4;
	case SHADER_DATA_TYPE::INT_2:	return 4 * 2;
	case SHADER_DATA_TYPE::INT_3:	return 4 * 3;
	case SHADER_DATA_TYPE::INT_4:	return 4 * 4;
	case SHADER_DATA_TYPE::BOOL:	return 1;
	default: __debugbreak(); return -1;
	}
	return 0;
}
static auto SHADER_DATA_TYPE_to_openGL_type(const SHADER_DATA_TYPE type) -> GLenum {
	switch (type) {
	case SHADER_DATA_TYPE::FLOAT:
	case SHADER_DATA_TYPE::FLOAT_2:
	case SHADER_DATA_TYPE::FLOAT_3:
	case SHADER_DATA_TYPE::FLOAT_4: return GL_FLOAT;
	default: __debugbreak(); return 0;
	}
}

BufferLayout::BufferElement::BufferElement(SHADER_DATA_TYPE type, const std::string& name, bool normalized)
	: name(name), type(type), size(SHADER_DATA_TYPE_get_size(type)), offset(0), normalized(normalized) {
}

auto BufferLayout::BufferElement::get_component_count() -> uint32_t {
	switch (type) {
	case SHADER_DATA_TYPE::FLOAT:	return 1;
	case SHADER_DATA_TYPE::FLOAT_2:	return 2;
	case SHADER_DATA_TYPE::FLOAT_3:	return 3;
	case SHADER_DATA_TYPE::FLOAT_4:	return 4;
	case SHADER_DATA_TYPE::MAT_3:	return 3; // 3* float3
	case SHADER_DATA_TYPE::MAT_4:	return 4; // 4* float4
	case SHADER_DATA_TYPE::INT:		return 1;
	case SHADER_DATA_TYPE::INT_2:	return 2;
	case SHADER_DATA_TYPE::INT_3:	return 3;
	case SHADER_DATA_TYPE::INT_4:	return 4;
	case SHADER_DATA_TYPE::BOOL:	return 1;
	default: __debugbreak(); return -1;
	}
}

BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
	: m_elements(elements) {
	calculate_offset_and_stride();
}

auto BufferLayout::calculate_offset_and_stride() -> void {
	size_t offset = 0;
	m_stride = 0;
	for (BufferElement& element : m_elements) {
		element.offset = offset;
		offset += element.size;
		m_stride += element.size;
	}
}

GLVertexBuffer::GLVertexBuffer() {
	glCreateBuffers(1, &m_ID);
}

GLVertexBuffer::~GLVertexBuffer() {
	if (is_invalid == false) {
		glDeleteBuffers(1, &m_ID);
	}
}

GLVertexBuffer::GLVertexBuffer(GLVertexBuffer&& other) : m_ID(std::move(other.m_ID)) {
	other.is_invalid = true;
}

auto GLVertexBuffer::bind() const -> void {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

auto GLVertexBuffer::unbind() const -> void {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto GLVertexBuffer::reserve_in_GPU(GLuint size_in_bytes) const -> void {
	// if NULL is passed in as data, it only reserves size_in_bytes bytes.
	glBufferData(GL_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, NULL, buffer_usage);
}

auto GLVertexBuffer::send_to_GPU(GLuint size_in_bytes, Vertex* data) const -> void {
	glBufferData(GL_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, data, buffer_usage);
}

auto GLVertexBuffer::send_to_GPU(GLuint size_in_bytes, void* data) const -> void {
	glBufferData(GL_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, data, buffer_usage);
}

auto GLVertexBuffer::update(GLuint size_in_bytes, Vertex* data) const -> void {
	glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_bytes, data);
	//glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
}

GLVertexArray::GLVertexArray() {
	glCreateVertexArrays(1, &m_ID);
}

GLVertexArray::~GLVertexArray() {
	if (is_invalid == false) {
		glDeleteVertexArrays(1, &m_ID);
	}
}

auto GLVertexArray::bind() const -> void {
	glBindVertexArray(m_ID);
}

auto GLVertexArray::unbind() const -> void {
	glBindVertexArray(0);
}



auto GLVertexArray::set_layout(BufferLayout buffer_layout) const -> void {
	int vertex_buffer_index = 0;
	for (size_t i = 0; i != buffer_layout.m_elements.size(); i++) {
		BufferLayout::BufferElement& element = buffer_layout.m_elements[i];
		switch (element.type) {
		case SHADER_DATA_TYPE::FLOAT:
		case SHADER_DATA_TYPE::FLOAT_2:
		case SHADER_DATA_TYPE::FLOAT_3:
		case SHADER_DATA_TYPE::FLOAT_4:
		{
			glEnableVertexAttribArray(vertex_buffer_index);
			//glEnableVertexArrayAttrib(m_ID, vertex_buffer_index);
			glVertexAttribPointer(
				vertex_buffer_index,
				element.get_component_count(),
				SHADER_DATA_TYPE_to_openGL_type(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				buffer_layout.m_stride,
				//reinterpret_cast<const void*>(element.offset)
				(const void*)element.offset
			);
			vertex_buffer_index++;
		} break;
		default: __debugbreak();

		}
	}
}

GLIndexBuffer::GLIndexBuffer() {
	glCreateBuffers(1, &m_ID);
}

GLIndexBuffer::~GLIndexBuffer() {
	if (is_invalid == false) {
		glDeleteBuffers(1, &m_ID);
	}
}

GLIndexBuffer::GLIndexBuffer(GLIndexBuffer&& other) : m_ID(std::move(other.m_ID)) {
	other.is_invalid = true;
}

auto GLIndexBuffer::bind() const -> void {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

auto GLIndexBuffer::unbind() const -> void {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

auto GLIndexBuffer::reserve_in_GPU(GLuint size_in_bytes) const -> void {
	// if NULL is passed in as data, it only reserves size_in_bytes bytes.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
}

auto GLIndexBuffer::send_to_GPU(GLuint size_in_bytes, const GLuint* data) const -> void {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, data, GL_DYNAMIC_DRAW);
}

auto GLIndexBuffer::update(GLuint size_in_bytes, GLuint* data) const -> void {
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size_in_bytes, data);
	//glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
}

auto GLIndexBuffer::send_to_GPU(GLuint size_in_bytes, GLuint* data) const -> void {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, data, GL_DYNAMIC_DRAW);
}
