#ifndef DPICP_FORMAT
#define DPICP_FORMAT

#include <string>
#include <vector>

namespace format_data {
	using u32 = unsigned __int32;
	using u8 = unsigned __int8;
	

	enum class SectionType {
		OUTER,
		TEXT,
		DATA,
		BSS,
		ROD
	};

	enum class OperationType {
		EOX,
		IDENTIFIER,
		CONSTANT,
		ADD, SUB, MULT, DIV, MOD, POW,
		SHIFTL, SHIFTR, BND, BOR, BNT, BXR,
		LESS, GREATER, LESSEQUAL, GREATEQUAL, EQUAL, NOTEQUAL,
		LAND, LOR, LNOT,
		NEG,
		SELECT
	};

	inline void push32(std::vector<u8>& list, u32 inst) {
		union {
			u32 u32data;
			u8 u8data[4];
		} conv;

		conv.u32data = inst;
		list.push_back(conv.u8data[3]);
		list.push_back(conv.u8data[2]);
		list.push_back(conv.u8data[1]);
		list.push_back(conv.u8data[0]);
	}

	typedef struct _Section {
		std::string name;
		u32 type;
		u32 start;
		u32 end;

		void getRaw(std::vector<u8>& out) {
			out.insert(out.end(), name.begin(), name.end());
			out.push_back(0);
			push32(out, type);
			push32(out, start);
			push32(out, end);
		}
	} Section;

	typedef struct _Mapper {
		std::string sectionName;
		u32 byteIndex;
		u32 offset;
		u32 size;
		u32 exprIndex;

		void getRaw(std::vector<u8>& out) {
			out.insert(out.end(), sectionName.begin(), sectionName.end());
			out.push_back(0);
			push32(out, byteIndex);
			push32(out, offset);
			push32(out, size);
			push32(out, exprIndex);
		}
	} Mapper;

	typedef struct _Operation {
		u32 type;
		u32 value;

		void getRaw(std::vector<u8>& out) {
			push32(out, type);
			push32(out, value);
		}
	} Operation;

	typedef struct _Identifier {
		std::string name;
		u32 value;

		void getRaw(std::vector<u8>& out) {
			out.insert(out.end(), name.begin(), name.end());
			out.push_back(0);
			push32(out, value);
		}
	} Identifier;

	typedef struct _Header {
		u32 sectionsStart, sectionsEnd;
		u32 binaryStart, binaryEnd;
		u32 mapperStart, mapperEnd;
		u32 expressionsStart, expressionsEnd;
		u32 definedIdentifierStart, definedIdentifierEnd;
		u32 usedIdentifierStart, usedIdentifierEnd;

		static u32 size() {
			return 13 * (u32)sizeof(u32);
		}

		void getRaw(std::vector<u8>& out) {
			push32(out, size());
			push32(out, sectionsStart);
			push32(out, sectionsEnd);
			push32(out, binaryStart);
			push32(out, binaryEnd);
			push32(out, mapperStart);
			push32(out, mapperEnd);
			push32(out, expressionsStart);
			push32(out, expressionsEnd);
			push32(out, definedIdentifierStart);
			push32(out, definedIdentifierEnd);
			push32(out, usedIdentifierStart);
			push32(out, usedIdentifierEnd);
		}
	} Header;

	typedef struct _Format {
		std::vector<Section> sections;
		std::vector<u8> binary;
		std::vector<Mapper> mapper;
		std::vector<std::vector<Operation>> expressions;
		std::vector<Identifier> definedIdentifier;
		std::vector<std::string> usedIdentifier;

		void clear() {
			sections.clear();
			binary.clear();
			mapper.clear();
			expressions.clear();
			definedIdentifier.clear();
			usedIdentifier.clear();
		}

		void getRaw(std::vector<u8>& out) {
			std::vector<u8> sectionsRaw, mapperRaw, expressionsRaw, definedIdentifierRaw, usedIdentifierRaw;

			for (auto it = sections.begin(); it != sections.end(); ++it)
				it->getRaw(sectionsRaw);
			for (auto it = mapper.begin(); it != mapper.end(); ++it)
				it->getRaw(mapperRaw);
			for (auto it = expressions.begin(); it != expressions.end(); ++it) {
				for (auto si = it->begin(); si != it->end(); ++si)
					si->getRaw(expressionsRaw);
			}
			for (auto it = definedIdentifier.begin(); it != definedIdentifier.end(); ++it)
				it->getRaw(definedIdentifierRaw);
			for (auto it = usedIdentifier.begin(); it != usedIdentifier.end(); ++it) {
				usedIdentifierRaw.insert(usedIdentifierRaw.end(), it->begin(), it->end());
				usedIdentifierRaw.push_back(0);
			}

			Header header = {};
			u32 offset = Header::size();

			header.sectionsStart = offset; offset += (u32)sectionsRaw.size(); header.sectionsEnd = offset;
			header.binaryStart = offset; offset += (u32)binary.size(); header.binaryEnd = offset;
			header.mapperStart = offset; offset += (u32)mapperRaw.size(); header.mapperEnd = offset;
			header.expressionsStart = offset; offset += (u32)expressionsRaw.size(); header.expressionsEnd = offset;
			header.definedIdentifierStart = offset; offset += (u32)definedIdentifierRaw.size(); header.definedIdentifierEnd = offset;
			header.usedIdentifierStart = offset; offset += (u32)usedIdentifierRaw.size(); header.usedIdentifierEnd = offset;

			header.getRaw(out);
			out.insert(out.end(), sectionsRaw.begin(), sectionsRaw.end());
			out.insert(out.end(), binary.begin(), binary.end());
			out.insert(out.end(), mapperRaw.begin(), mapperRaw.end());
			out.insert(out.end(), expressionsRaw.begin(), expressionsRaw.end());
			out.insert(out.end(), definedIdentifierRaw.begin(), definedIdentifierRaw.end());
			out.insert(out.end(), usedIdentifierRaw.begin(), usedIdentifierRaw.end());
		}
	} Format;
}

#endif
