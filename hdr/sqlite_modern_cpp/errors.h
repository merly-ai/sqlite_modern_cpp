#pragma once

#include <string>
#include <stdexcept>

#include <sqlite3.h>

namespace sqlite {

	class sqlite_exception: public std::runtime_error {
	public:
		sqlite_exception(const char* msg, std::string sql, std::string db_name, int code = -1): runtime_error(msg), code(code), sql(sql), db_name(db_name) {}
		sqlite_exception(int code, std::string sql, std::string db_name): runtime_error(sqlite3_errstr(code)), code(code), sql(sql), db_name(db_name) {}
		int get_code() const {return code & 0xFF;}
		int get_extended_code() const {return code;}
		std::string get_sql() const { return sql; }
		std::string get_db_name() const { return db_name; }
	private:
		int code;
		std::string sql;
		std::string db_name;
	};

	namespace errors {
		//One more or less trivial derived error class for each SQLITE error.
		//Note the following are not errors so have no classes:
		//SQLITE_OK, SQLITE_NOTICE, SQLITE_WARNING, SQLITE_ROW, SQLITE_DONE
		//
		//Note these names are exact matches to the names of the SQLITE error codes.
#define SQLITE_MODERN_CPP_ERROR_CODE(NAME,name,derived) \
		class name: public sqlite_exception { using sqlite_exception::sqlite_exception; };\
		derived
#define SQLITE_MODERN_CPP_ERROR_CODE_EXTENDED(BASE,SUB,base,sub) \
		class base ## _ ## sub: public base { using base::base; };
#include "lists/error_codes.h"
#undef SQLITE_MODERN_CPP_ERROR_CODE_EXTENDED
#undef SQLITE_MODERN_CPP_ERROR_CODE

		//Some additional errors are here for the C++ interface
		class more_rows: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class no_rows: public sqlite_exception { using sqlite_exception::sqlite_exception; };
		class more_statements: public sqlite_exception { using sqlite_exception::sqlite_exception; }; // Prepared statements can only contain one statement
		class invalid_utf16: public sqlite_exception { using sqlite_exception::sqlite_exception; };

		static void throw_sqlite_error(const int& error_code, const std::string &sql = "", const std::string &db_name = "") {
			switch(error_code & 0xFF) {
#define SQLITE_MODERN_CPP_ERROR_CODE(NAME,name,derived)     \
				case SQLITE_ ## NAME: switch(error_code) {          \
					derived                                           \
					default: throw name(error_code, sql, db_name); \
				}
#define SQLITE_MODERN_CPP_ERROR_CODE_EXTENDED(BASE,SUB,base,sub) \
					case SQLITE_ ## BASE ## _ ## SUB: throw base ## _ ## sub(error_code, sql, db_name);
#include "lists/error_codes.h"
#undef SQLITE_MODERN_CPP_ERROR_CODE_EXTENDED
#undef SQLITE_MODERN_CPP_ERROR_CODE
				default: throw sqlite_exception(error_code, sql, db_name);
			}
		}
	}
	namespace exceptions = errors;
}
