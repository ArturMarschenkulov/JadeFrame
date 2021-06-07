#include "glsl_parser.h"
#include <cassert>

static auto is_digit(const char c) -> bool {
	return c >= '0' && c <= '9';
}
static auto is_binary_digit(const char c) -> bool {
	return c >= '0' && c <= '1';
}
static auto is_octal_digit(const char c) -> bool {
	return c >= '0' && c <= '7';
}
static auto is_hexadecimal_digit(const char c) -> bool {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
static auto is_alpha(const char c) -> bool {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static auto is_alpha_numeric(const char c) -> bool {
	return is_alpha(c) || is_digit(c);
}



class CharCursor {
public:
	CharCursor();
	CharCursor(std::string& str);
	auto operator=(std::string& str)->CharCursor&;
	auto operator=(const std::string& str)->CharCursor&;
	auto get() const -> const char*;
	auto advance() const -> void;
	auto peek(int n) const -> char;
private:
	mutable const char* it;
};
CharCursor::CharCursor() {
}
CharCursor::CharCursor(std::string& str) {
	it = &str[0];
}
auto CharCursor::get() const -> const char* {
	return it;
}
auto CharCursor::advance() const -> void {
	it++;
}
auto CharCursor::peek(int n) const -> char {
	return *(it + n);
}
auto CharCursor::operator=(std::string& str) -> CharCursor& {
	it = &str[0];
	return *this;
}
auto CharCursor::operator=(const std::string& str) -> CharCursor& {
	it = &str[0];
	return *this;
}

static auto is_skip_token(const Token& token) -> bool {
	switch (token.type) {
		case TOKEN_TYPE::SKW_WHITESPACE:
		case TOKEN_TYPE::SKW_NEWLINE:
		case TOKEN_TYPE::SKW_COMMENT: return true;
		default: return false;
	}
}
struct Tokenizer {
	Tokenizer() {
		keyword_map["in"] = TOKEN_TYPE::KW_IN;
		keyword_map["out"] = TOKEN_TYPE::KW_OUT;
		keyword_map["uniform"] = TOKEN_TYPE::KW_UNIFORM;
		keyword_map["layout"] = TOKEN_TYPE::KW_LAYOUT;

		keyword_map["const"] = TOKEN_TYPE::KW_CONST;
		keyword_map["void"] = TOKEN_TYPE::KW_VOID;
		keyword_map["sampler2D"] = TOKEN_TYPE::KW_SAMPLER2D;
		keyword_map["float"] = TOKEN_TYPE::KW_FLOAT;
		keyword_map["double"] = TOKEN_TYPE::KW_DOUBLE;
		keyword_map["vec2"] = TOKEN_TYPE::KW_VEC2;
		keyword_map["vec3"] = TOKEN_TYPE::KW_VEC3;
		keyword_map["vec4"] = TOKEN_TYPE::KW_VEC4;
		keyword_map["mat4"] = TOKEN_TYPE::KW_MAT4;

		keyword_map["return"] = TOKEN_TYPE::KW_RETURN;
	}

	mutable CharCursor start_cursor;
	mutable CharCursor current_cursor;
	std::map<std::string, TOKEN_TYPE> keyword_map;

	auto scan_tokens(const std::string source) const->std::vector<Token>;
	auto scan_token(const char c) const -> const Token;
};

static auto TOKEN_TYPE_is_type(TOKEN_TYPE type) {
	bool result;
	switch (type) {
		case TOKEN_TYPE::KW_VOID: result = true; break;
		case TOKEN_TYPE::KW_FLOAT: result = true; break;
		case TOKEN_TYPE::KW_DOUBLE: result = true; break;
		case TOKEN_TYPE::KW_VEC2: result = true; break;
		case TOKEN_TYPE::KW_VEC3: result = true; break;
		case TOKEN_TYPE::KW_VEC4: result = true; break;
		case TOKEN_TYPE::KW_MAT4: result = true; break;
		default: result = false;
	}
	return result;
}
static auto TOKEN_TYPE_is_qualifier(TOKEN_TYPE type) {
	bool result;
	switch (type) {
		case TOKEN_TYPE::KW_IN: result = true; break;
		case TOKEN_TYPE::KW_OUT: result = true; break;
		case TOKEN_TYPE::KW_UNIFORM: result = true; break;
		default: result = false;
	}
	return result;
}

static auto TOKEN_TYPE_is_directive(TOKEN_TYPE type) {
	bool result;
	switch (type) {
		case TOKEN_TYPE::D_VERSION: result = true; break;
		default: result = false;
	}
	return result;
}
GLSLParser::GLSLParser() {

}
auto GLSLParser::parse(const std::string& text) -> void {
	Tokenizer tok;
	std::vector<Token> tokens = tok.scan_tokens(text);
	//__debugbreak();
	parse(tokens);
}

auto GLSLParser::parse(const std::vector<Token>& tokens) -> void {
	const Token* start_ptr = tokens.data();
	const Token* current_ptr = tokens.data();

	while (current_ptr->type != TOKEN_TYPE::SKW_EOF) {
		const Token t = *current_ptr;

		if (TOKEN_TYPE_is_directive(t.type)) {

			if ((current_ptr + 1)->type == TOKEN_TYPE::L_NUMBER) {
				current_ptr++;
				m_version.m_number = std::stoi(current_ptr->lexeme);
			}
			if ((current_ptr + 1)->type == TOKEN_TYPE::L_IDENTIFIER) {
				current_ptr++;
				if (current_ptr->lexeme == "core") {
					m_version.m_profile = VersionNumber::PROFILE::CORE;
				} else if (current_ptr->lexeme == "compatibility") {
					m_version.m_profile = VersionNumber::PROFILE::COMPATIBILITY;
				} else {
					__debugbreak();
				}
			}
			current_ptr++;
			continue;
		} else {

			Variable v;
			if (t.type == TOKEN_TYPE::KW_LAYOUT) {
				if ((current_ptr + 1)->type == TOKEN_TYPE::P_LEFT_PAREN) {
					current_ptr++;
					while (current_ptr->type != TOKEN_TYPE::P_RIGHT_PAREN) {
						current_ptr++;
						if (current_ptr->type == TOKEN_TYPE::L_NUMBER) {
							v.m_layout = std::atoi(current_ptr->lexeme.c_str());
						}
					}
				}
				current_ptr++;
			}

			if (TOKEN_TYPE_is_qualifier(current_ptr->type)) {

				v.m_qualifier =
					current_ptr->type == TOKEN_TYPE::KW_IN ? Variable::QUALIFIER::IN :
					current_ptr->type == TOKEN_TYPE::KW_OUT ? Variable::QUALIFIER::OUT :
					current_ptr->type == TOKEN_TYPE::KW_UNIFORM ? Variable::QUALIFIER::UNIFORM :
					Variable::QUALIFIER::INVALID
				;
				current_ptr++;
			}
			//if (v.m_qualifier == Variable::QUALIFIER::INVALID) __debugbreak();

			if (TOKEN_TYPE_is_type(current_ptr->type)) {
				v.m_type =
					(current_ptr->type == TOKEN_TYPE::KW_VEC2) ? Variable::TYPE::VEC2 :
					(current_ptr->type == TOKEN_TYPE::KW_VEC3) ? Variable::TYPE::VEC3 :
					(current_ptr->type == TOKEN_TYPE::KW_VEC4) ? Variable::TYPE::VEC4 :
					(current_ptr->type == TOKEN_TYPE::KW_FLOAT) ? Variable::TYPE::FLOAT :
					(current_ptr->type == TOKEN_TYPE::KW_SAMPLER2D) ? Variable::TYPE::SAMPLER2D :
					(current_ptr->type == TOKEN_TYPE::KW_DOUBLE) ? Variable::TYPE::DOUBLE :
					(current_ptr->type == TOKEN_TYPE::KW_MAT4) ? Variable::TYPE::MAT4 :
					Variable::TYPE::INVALID
				;

				current_ptr++;
			} else {
				current_ptr++;
				continue;
			}
			//if (v.m_type == Variable::TYPE::INVALID) __debugbreak();
			
			if (current_ptr->type == TOKEN_TYPE::L_IDENTIFIER) {
				v.m_name = current_ptr->lexeme;
				current_ptr++;
			}
			if (current_ptr->type != TOKEN_TYPE::P_SEMICOLON) {
				current_ptr++;
				continue;
			}

			if (v.m_name != "") {
				m_variables.push_back(v);
			}

			current_ptr++;
		}

	#if 0
		if(false)
		{
			switch (t.type) {
				case TOKEN_TYPE::D_VERSION:
				{
					if ((current_ptr + 1)->type == TOKEN_TYPE::L_NUMBER) {
						current_ptr++;
						m_version.m_number = std::stoi(current_ptr->lexeme);
					}
					if ((current_ptr + 1)->type == TOKEN_TYPE::L_IDENTIFIER) {
						current_ptr++;
						if (current_ptr->lexeme == "core") {
							m_version.m_profile = VersionNumber::PROFILE::CORE;
						} else if (current_ptr->lexeme == "compatibility") {
							m_version.m_profile = VersionNumber::PROFILE::COMPATIBILITY;
						} else {
							__debugbreak();
						}
					}
				} break;
				case TOKEN_TYPE::KW_LAYOUT:
				{

					if ((current_ptr + 1)->type == TOKEN_TYPE::P_LEFT_PAREN) {
						current_ptr++;
						while (current_ptr->type != TOKEN_TYPE::P_RIGHT_PAREN) {
							current_ptr++;
							if (current_ptr->type == TOKEN_TYPE::L_NUMBER) {
								v.m_layout = std::atoi(current_ptr->lexeme.c_str());
							}
						}
					}
					if (TOKEN_TYPE_is_qualifier((current_ptr + 1)->type)) {
						current_ptr++;
						v.m_qualifier =
							current_ptr->type == TOKEN_TYPE::KW_IN ? Variable::QUALIFIER::IN :
							current_ptr->type == TOKEN_TYPE::KW_OUT ? Variable::QUALIFIER::OUT :
							current_ptr->type == TOKEN_TYPE::KW_UNIFORM ? Variable::QUALIFIER::UNIFORM :
							Variable::QUALIFIER::INVALID;

					}

					if (TOKEN_TYPE_is_type((current_ptr + 1)->type)) {
						current_ptr++;
						v.m_type =
							(current_ptr->type == TOKEN_TYPE::KW_VEC2) ? Variable::TYPE::VEC2 :
							(current_ptr->type == TOKEN_TYPE::KW_VEC3) ? Variable::TYPE::VEC3 :
							(current_ptr->type == TOKEN_TYPE::KW_VEC4) ? Variable::TYPE::VEC4 :
							(current_ptr->type == TOKEN_TYPE::KW_FLOAT) ? Variable::TYPE::FLOAT :
							(current_ptr->type == TOKEN_TYPE::KW_SAMPLER2D) ? Variable::TYPE::SAMPLER2D :
							(current_ptr->type == TOKEN_TYPE::KW_DOUBLE) ? Variable::TYPE::DOUBLE :
							(current_ptr->type == TOKEN_TYPE::KW_MAT4) ? Variable::TYPE::MAT4 :
							Variable::TYPE::INVALID;
					}
					if ((current_ptr + 1)->type == TOKEN_TYPE::L_IDENTIFIER) {
						current_ptr++;
						__debugbreak();
						v.m_name = current_ptr->lexeme;
					}

					m_variables.push_back(v);
				}break;
				case TOKEN_TYPE::KW_IN:
				case TOKEN_TYPE::KW_OUT:
				case TOKEN_TYPE::KW_UNIFORM:
				{
					Variable v;
					if (TOKEN_TYPE_is_qualifier((current_ptr + 1)->type)) {
						current_ptr++;
						v.m_qualifier =
							current_ptr->type == TOKEN_TYPE::KW_IN ? Variable::QUALIFIER::IN :
							current_ptr->type == TOKEN_TYPE::KW_OUT ? Variable::QUALIFIER::OUT :
							current_ptr->type == TOKEN_TYPE::KW_UNIFORM ? Variable::QUALIFIER::UNIFORM :
							Variable::QUALIFIER::INVALID;

					}

					if (TOKEN_TYPE_is_type((current_ptr + 1)->type)) {
						current_ptr++;
						v.m_type =
							(current_ptr->type == TOKEN_TYPE::KW_VEC2) ? Variable::TYPE::VEC2 :
							(current_ptr->type == TOKEN_TYPE::KW_VEC3) ? Variable::TYPE::VEC3 :
							(current_ptr->type == TOKEN_TYPE::KW_VEC4) ? Variable::TYPE::VEC4 :
							(current_ptr->type == TOKEN_TYPE::KW_FLOAT) ? Variable::TYPE::FLOAT :
							(current_ptr->type == TOKEN_TYPE::KW_SAMPLER2D) ? Variable::TYPE::SAMPLER2D :
							(current_ptr->type == TOKEN_TYPE::KW_DOUBLE) ? Variable::TYPE::DOUBLE :
							(current_ptr->type == TOKEN_TYPE::KW_MAT4) ? Variable::TYPE::MAT4 :
							Variable::TYPE::INVALID;
					}
					if ((current_ptr + 1)->type == TOKEN_TYPE::L_IDENTIFIER) {
						current_ptr++;
						__debugbreak();
						v.m_name = current_ptr->lexeme;
					}
					m_variables.push_back(v);
				} break;
				case TOKEN_TYPE::SKW_NEWLINE: break;
				default:
				{

				}break;
			}
		}
	#endif
	}
}


auto Tokenizer::scan_tokens(const std::string source) const -> std::vector<Token> {
	start_cursor = source;
	current_cursor = source;
	std::vector<Token> tokens;

	while (current_cursor.peek(0) != '\0') {
		start_cursor = current_cursor;
		Token token = scan_token(current_cursor.peek(0));
		if (is_skip_token(token) == false) {
			tokens.push_back(token);
		}
		current_cursor.advance();
	}
	start_cursor.advance();

	std::string text(start_cursor.get(), current_cursor.get() + 1);
	Token token_from_type{ TOKEN_TYPE::SKW_EOF, text };
	tokens.push_back(token_from_type);
	return tokens;
}

auto Tokenizer::scan_token(const char c) const -> const Token {
	TOKEN_TYPE token_type;
	switch (c) {
		case '+': token_type = TOKEN_TYPE::P_PLUS; break;
		case '-': token_type = TOKEN_TYPE::P_MINUS; break;
		case '*': token_type = TOKEN_TYPE::P_STAR; break;
		case '/':
		{
			if (current_cursor.peek(1) == '/') {
				current_cursor.advance();
				while (current_cursor.peek(1) != '\n' && current_cursor.peek(1) != '\0') {
					current_cursor.advance();
				}
				token_type = TOKEN_TYPE::SKW_COMMENT;
			} else {
				token_type = TOKEN_TYPE::P_SLASH;
			}
		} break;
		case '=':
		{
			if (current_cursor.peek(1) == '=') {
				current_cursor.advance();
				token_type = TOKEN_TYPE::P_EQUAL_EQUAL;
			} else {
				token_type = TOKEN_TYPE::P_EQUAL;
			}
		}break;
		case '<':
		{
			if (current_cursor.peek(1) == '=') {
				current_cursor.advance();
				token_type = TOKEN_TYPE::P_LESS_EQUAL;
			} else {
				token_type = TOKEN_TYPE::P_LESS;
			}
		}break;
		case '>':
		{
			if (current_cursor.peek(1) == '=') {
				current_cursor.advance();
				token_type = TOKEN_TYPE::P_GREATER_EQUAL;
			} else {
				token_type = TOKEN_TYPE::P_GREATER;
			}
		}break;
		case ',': token_type = TOKEN_TYPE::P_COMMA; break;
		case ';': token_type = TOKEN_TYPE::P_SEMICOLON; break;
		case '(': token_type = TOKEN_TYPE::P_LEFT_PAREN; break;
		case ')': token_type = TOKEN_TYPE::P_RIGHT_PAREN; break;
		case '{': token_type = TOKEN_TYPE::P_LEFT_BRACE; break;
		case '}': token_type = TOKEN_TYPE::P_RIGHT_BRACE; break;
		case '#':
		{
			auto prev_cursor = current_cursor;
			while (is_alpha(current_cursor.peek(1))) {
				current_cursor.advance();
			}

			std::string directive(prev_cursor.get(), current_cursor.get() + 1);
			TOKEN_TYPE type;
			if (directive == "#version") {
				type = TOKEN_TYPE::D_VERSION;
			}
			token_type = type;
		} break;
		case ' ':
		case '\r':
		case '\t': token_type = TOKEN_TYPE::SKW_WHITESPACE;  break;
		case '\n': token_type = TOKEN_TYPE::SKW_NEWLINE;  /*line++*/; break;


		default:
		{
			if (is_digit(c)) {
				while (is_digit(current_cursor.peek(1))) {
					current_cursor.advance();
				}
				token_type = TOKEN_TYPE::L_NUMBER;
			} else if (is_alpha(c)) {
				auto prev_cursor = current_cursor;
				while (is_alpha_numeric(current_cursor.peek(1))) {
					current_cursor.advance();
				}
				std::string identifier(prev_cursor.get(), current_cursor.get() + 1);
				TOKEN_TYPE type;
				if (keyword_map.find(identifier) == keyword_map.end()) {
					type = TOKEN_TYPE::L_IDENTIFIER;
					//__debugbreak();
				} else {
					type = keyword_map.at(identifier);
				}
				token_type = type;
			} else {
				token_type = TOKEN_TYPE::SKW_UNKNOWN;
			}

		}break;
	}
	std::string text(start_cursor.get(), current_cursor.get() + 1);

	return Token{ token_type, text };
}