#pragma once
#include <string>
#include <vector>
#include <map>

enum class TOKEN_TYPE {
	P_SEMICOLON, // ;
	P_EQUAL, // =
	P_LESS, // <
	P_GREATER, // >
	P_HASH, // #
	P_LEFT_PAREN, // (
	P_RIGHT_PAREN, // )
	P_LEFT_BRACE, // {
	P_RIGHT_BRACE, // }
	P_COMMA, // ,
	P_PLUS, // +
	P_MINUS, // -
	P_STAR,	// *
	P_SLASH, // /

	P_EQUAL_EQUAL, // ==
	P_LESS_EQUAL, // <=
	P_GREATER_EQUAL, // >=


	//Directives
	D_VERSION, // #version

	//Keywords, Qualifiers
	KW_IN,
	KW_OUT,
	KW_UNIFORM,
	KW_LAYOUT,
	//Keywords, Types
	KW_CONST,
	KW_VOID,
	KW_SAMPLER2D,
	KW_FLOAT,
	KW_DOUBLE,
	KW_VEC2,
	KW_VEC3,
	KW_VEC4,
	KW_MAT4,

	KW_RETURN,

	//Literals
	L_NUMBER,
	L_IDENTIFIER,

	//EXTRA
	SKW_EOF,
	SKW_WHITESPACE,
	SKW_TAB,
	SKW_NEWLINE,
	SKW_COMMENT,
	SKW_UNKNOWN,

};

struct Token {
	TOKEN_TYPE type = TOKEN_TYPE::SKW_UNKNOWN;
	std::string lexeme = "NOTHING";
};

struct GLSLParser {
	GLSLParser();
	auto parse(const std::string& text) -> void;
	auto parse_statement() -> void;

public: //Parser part
	struct VersionNumber {
		int m_number;
		enum class PROFILE {
			CORE,
			COMPATIBILITY,
		} m_profile;
	} m_version;
	struct Variable {
		std::string m_name;
		enum class QUALIFIER {
			IN,
			OUT,
			UNIFORM,
			INVALID,
		} m_qualifier = QUALIFIER::INVALID;
		enum class TYPE {
			BOOL,
			INT, UINT,
			FLOAT, DOUBLE,
			SAMPLER2D,

			BVEC1, BVEC2, BVEC3, BVEC4,
			IVEC1, IVEC2, IVEC3, IVEC4,
			UVEC1, UVEC2, UVEC3, UVEC4,
			VEC1, VEC2, VEC3, VEC4,
			DVEC1, DVEC2, DVEC3, DVEC4,

			MAT4,
			INVALID,
		} m_type = TYPE::INVALID;
		int m_layout = -1; //NOTE: -1 means there is no layout. Consider a different approach.
	};
	std::vector<Variable> m_variables;
	auto parse(const std::vector<Token>& tokens) -> void;


};
