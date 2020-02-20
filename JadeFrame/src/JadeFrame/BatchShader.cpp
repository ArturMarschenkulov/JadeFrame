#include "BatchShader.h"


BatchShader::BatchShader()
	: m_ID()
	, m_shaderTypes() {
	vertexShaderSource =
		R"(
			#version 450 core
			layout (location = 0) in vec3 v_Pos;
			layout (location = 1) in vec4 v_Col;

			out vec4 f_Col;

			uniform mat4 MVP;

			void main() {
				f_Col = v_Col;
				gl_Position = MVP * vec4(v_Pos.x, v_Pos.y, v_Pos.z, 1.0);
			}
		)";

	fragmentShaderSource =
		R"(
			#version 450 core
			in vec4 f_Col;
			out vec4 o_Col;
			void main() {
				o_Col = f_Col;
				//o_Col = vec4(1.0f, 0.5f, 0.2f, 1.0f);
			}
		)";
}
void BatchShader::init() {

	this->compile(GL_VERTEX_SHADER, vertexShaderSource);
	this->compile(GL_FRAGMENT_SHADER, fragmentShaderSource);
	this->link();
	this->validate();
	this->updateShaderVariables(GL_ACTIVE_UNIFORMS);
	this->updateShaderVariables(GL_ACTIVE_ATTRIBUTES);
	glDetachShader(m_ID, m_shaderTypes[0]);
	glDetachShader(m_ID, m_shaderTypes[1]);
}

void BatchShader::use() {
	glUseProgram(this->m_ID);
}

GLuint BatchShader::compile(GLenum type, const std::string& codeSource) {
	GLuint shaderID = glCreateShader(type);
	const GLchar* shaderCode = codeSource.c_str();
	glShaderSource(shaderID, 1, &shaderCode, nullptr);
	glCompileShader(shaderID);

	switch(type) {
	case GL_VERTEX_SHADER:   m_shaderTypes[0] = shaderID; break;
	case GL_FRAGMENT_SHADER: m_shaderTypes[1] = shaderID; break;
	case GL_GEOMETRY_SHADER: m_shaderTypes[2] = shaderID; break;
	default:; break;
	}

	GLint isCompiled = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 512;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar infoLog[512];
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &infoLog[0]);
		glDeleteShader(shaderID);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED" << infoLog << std::endl;
		return 0;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::COMPILATION_SUCCEEDED" << std::endl;
	}
	return shaderID;
}

void BatchShader::link() {

	m_ID = glCreateProgram();
	glAttachShader(m_ID, m_shaderTypes[0]);
	glAttachShader(m_ID, m_shaderTypes[1]);


	glLinkProgram(m_ID);
	GLint isLinekd = GL_FALSE;
	glGetProgramiv(m_ID, GL_LINK_STATUS, &isLinekd);
	if(isLinekd == GL_FALSE) {
		char infoLog[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::LINKING_SUCCEEDED\n" << std::endl;
	}
}

void BatchShader::validate() {
	glValidateProgram(m_ID);

	GLint isValidated = GL_FALSE;
	glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (int*)&isValidated);
	if(isValidated == GL_FALSE) {
		char infoLog[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n" << infoLog << std::endl;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::VALIDATION_SUCCEEDED\n" << std::endl;
	}
}


GLint BatchShader::getUniformLocation(const std::string& name) const {
	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if(location == -1){
		std::cout << "Location of " << name << " can not be found" << std::endl;
		__debugbreak();
	}
	return location;
	//if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
	//	return m_uniformLocationCache[name];
	//} else {
	//	GLint location = glGetUniformLocation(m_ID, name.c_str());
	//	m_uniformLocationCache[name] = location;
	//	return location;
	//}

		// read from uniform/attribute array as originally obtained from OpenGL
	//for(unsigned int i = 0; i < m_uniforms.size(); ++i) {
	//	if(m_uniforms[i].name == name)
	//		return m_uniforms[i].location;
	//}
	//std::cout << "Location of " << name << " can not be found" << std::endl;
	//__debugbreak();
	//return -1;
}


void BatchShader::setUniform1i(const std::string& name, int value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl;
}
void BatchShader::setUniform1f(const std::string& name, float value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl;
}
void BatchShader::setUniform2f(const std::string& name, const Vec2& value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl;
}
void BatchShader::setUniform3f(const std::string& name, const Vec3& value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl;
}
void BatchShader::setUniform4f(const std::string& name, const Vec4& value) {
	int loc = getUniformLocation(name);
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}
void BatchShader::setUniformMatrix4fv(const std::string& name, const Mat4& mat) const {
	GLint loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

void BatchShader::updateShaderVariables(int shaderType) {
	int num;
	glGetProgramiv(m_ID, shaderType, &num);

	// iterate over all active uniforms
	char buffer[128];
	if(shaderType == GL_ACTIVE_UNIFORMS) {
		m_uniforms.resize(num);
		for(unsigned int i = 0; i < num; ++i) {
			GLenum glType;
			glGetActiveUniform(m_ID, i, sizeof(buffer), 0, &m_uniforms[i].size, &glType, buffer);
			m_uniforms[i].name = std::string(buffer);
			m_uniforms[i].type = SHADER_TYPE::BOOL;
			m_uniforms[i].location = glGetUniformLocation(m_ID, buffer);
		}
	} else if(shaderType == GL_ACTIVE_ATTRIBUTES) {
		m_attributes.resize(num);
		for(unsigned int i = 0; i < num; ++i) {
			GLenum glType;
			glGetActiveAttrib(m_ID, i, sizeof(buffer), 0, &m_attributes[i].size, &glType, buffer);
			m_attributes[i].name = std::string(buffer);
			m_attributes[i].type = SHADER_TYPE::BOOL;
			m_attributes[i].location = glGetAttribLocation(m_ID, buffer);
		}
	} else {
		std::cout << __FUNCTION__ << " shaderType is false" << std::endl;
		__debugbreak();
	}
}