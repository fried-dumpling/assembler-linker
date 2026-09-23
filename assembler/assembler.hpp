#ifndef DPICP_ASSEMBLER
#define DPICP_ASSEMBLER

#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <array>
#include <stack>
#include <queue>
#include <map>
#include <set>

#include "lexer.hpp"
#include "preprocesser.hpp"
#include "parser.hpp"
#include "evaluator.hpp"

#include "../common/format.hpp"
#include "../common/iotool.hpp"

namespace assembler {
	namespace lexer {
		using u64 = unsigned __int64;

		TOKENSTART(TokenType)
			add, addc, addi,
			sub, subc, subi,
			bxr, bxri,
			bor, bori,
			bnd, bndi,
			shiftl, shiftlc, shiftli,
			shiftr, shiftrc, shiftri,
			rol, roli, ror, rori,
			cmp, cmpi, test, testi,

			mov, set, sets,
			push, pop,

			ld, st,

			block,

			jmp, ijmp,
			call, ret,
			nop, brk,
			wait, halt,

			syscall, iret,
			intervec, interpri,
			stexp, ldexp,
			pctl, getabs,
			flush, inval,

			cpuid, panicvec, istack, intlevel, pagebase,
			enable, disable,
			L1D, L1I, TLB, L2, ALL,

			reg,
			gen,
			sbp, zero, one, full, pc, stack, flag,

			neg, pos, carry, carry4, overflow,

			_32B_, _16L_, _8L_, _8H_, _S16H_, _S16L_, _S8L_, _S8H_,

			text, data, bss,

			define,
			macro,
			end,

			macro_arg,
			identifier_unique,

			dot,
			comma,
			hash,

			openparen, closeparen,
			openbrace, closebrace,
			openbrack, closebrack,

			hexnum, decnum, octnum, binnum,

			colon, semicolon,

			plus, minus,
			star, dstar, slash, percent,
			tilde, ampersend, verticalbar, caret,
			leftshift, rightshift,
			less, greater, lessequal, greaterequal, equal, notequal,
			lognot, logampersend, logverticalbar, selector,

			curline,

			comment,

			whitespace,
			newline,

			character,
			string,
			identifier,

			TOKENEND();

		using LexerFactory = lexer_generator::LexerFactory<TokenType>;
		using LFCD = LexerFactory::CreateData;
		using Lexer = lexer_generator::Lexer<TokenType>;
		using Token = Lexer::Token;

		const LFCD CreateData = {
			{
				{ "add", TokenType::add },
				{ "addc", TokenType::addc },
				{ "addi", TokenType::addi },
				{ "sub", TokenType::sub },
				{ "subc", TokenType::subc },
				{ "subi", TokenType::subi },
				{ "bxr", TokenType::bxr },
				{ "bxri", TokenType::bxri },
				{ "bor", TokenType::bor },
				{ "bori", TokenType::bori },
				{ "bnd", TokenType::bnd },
				{ "bndi", TokenType::bndi },
				{ "shiftl", TokenType::shiftl },
				{ "shiftlc", TokenType::shiftlc },
				{ "shiftli", TokenType::shiftli },
				{ "shiftr", TokenType::shiftr },
				{ "shiftrc", TokenType::shiftrc },
				{ "shiftri", TokenType::shiftri },
				{ "rol", TokenType::rol },
				{ "roli", TokenType::roli},
				{ "ror", TokenType::ror},
				{ "rori", TokenType::rori},
				{ "cmp", TokenType::cmp },
				{ "cmpi", TokenType::cmpi },
				{ "test", TokenType::test },
				{ "testi", TokenType::testi },
				{ "set", TokenType::set },
				{ "sets", TokenType::sets },
				{ "mov", TokenType::mov },
				{ "block", TokenType::block },
				{ "push", TokenType::push },
				{ "pop", TokenType::pop },
				{ "ld", TokenType::ld },
				{ "st", TokenType::st },
				{ "jmp", TokenType::jmp },
				{ "ijmp", TokenType::ijmp },
				{ "call", TokenType::call },
				{ "ret", TokenType::ret },
				{ "nop", TokenType::nop },
				{ "brk", TokenType::brk },
				{ "wait", TokenType::wait },
				{ "halt", TokenType::halt },

				{ "syscall", TokenType::syscall},
				{ "iret", TokenType::iret},
				{ "intervec", TokenType::intervec},
				{ "interpri", TokenType::interpri},

				{ "stexp", TokenType::stexp },
				{ "ldexp", TokenType::ldexp },
				{ "pctl", TokenType::pctl },
				{ "getabs", TokenType::getabs },
				{ "flush", TokenType::flush },
				{ "inval", TokenType::inval },

				{ "istack", TokenType::istack },
				{ "panicvec", TokenType::panicvec },
				{ "cpuid", TokenType::cpuid },
				{ "intlevel", TokenType::intlevel },
				{ "pagebase", TokenType::pagebase },
				{ "enable", TokenType::enable },
				{ "disable", TokenType::disable },
				{ "L1D", TokenType::L1D },
				{ "L1I", TokenType::L1I },
				{ "TLB", TokenType::TLB },
				{ "L2", TokenType::L2 },
				{ "ALL", TokenType::ALL },

				{ "reg",TokenType::reg },
				{ "gen", TokenType::gen },
				{ "sbp", TokenType::sbp },
				{ "zero", TokenType::zero },
				{ "one", TokenType::one },
				{ "full", TokenType::full },
				{ "pc", TokenType::pc },
				{ "stack", TokenType::stack },
				{ "flag", TokenType::flag },

				{ "neg", TokenType::neg },
				{ "pos", TokenType::pos },
				{ "carry", TokenType::carry },
				{ "carry4", TokenType::carry4 },
				{ "overflow", TokenType::overflow },

				{ "32[bB]", TokenType::_32B_ },
				{ "16[lL]", TokenType::_16L_ },
				{ "8[lL]", TokenType::_8L_ },
				{ "8[hH]", TokenType::_8H_ },
				{ "[sS]16[hH]", TokenType::_S16H_ },
				{ "[sS]16[lL]", TokenType::_S16L_ },
				{ "[sS]8[lL]", TokenType::_S8L_ },
				{ "[sS]8[hH]", TokenType::_S8H_ },

				{ "\\.text", TokenType::text },
				{ "\\.data", TokenType::data },
				{ "\\.bss", TokenType::bss },

				{ "#define", TokenType::define },
				{ "#macro", TokenType::macro },
				{ "#end", TokenType::end },

				{ "\\.", TokenType::dot },
				{ ",", TokenType::comma },
				{ "#", TokenType::hash },
				{ "\\(", TokenType::openparen },
				{ "\\)", TokenType::closeparen },
				{ "\\{", TokenType::openbrace },
				{ "\\}", TokenType::closebrace },
				{ "\\[", TokenType::openbrack },
				{ "\\]", TokenType::closebrack },

				{ ":", TokenType::colon },
				{ ";", TokenType::semicolon },

				{ "\\+", TokenType::plus },
				{ "-", TokenType::minus },
				{ "\\*\\*", TokenType::dstar },
				{ "\\*", TokenType::star },
				{ "/", TokenType::slash },
				{ "%", TokenType::percent },
				{ "~", TokenType::tilde },
				{ "&&", TokenType::logampersend },
				{ "&", TokenType::ampersend },
				{ "\\|\\|", TokenType::logverticalbar },
				{ "\\|", TokenType::verticalbar },
				{ "^", TokenType::caret },
				{ "<<", TokenType::leftshift },
				{ ">>", TokenType::rightshift },
				{ "<=", TokenType::lessequal },
				{ ">=", TokenType::greaterequal },
				{ "<" , TokenType::less},
				{ ">" , TokenType::greater},
				{ "==", TokenType::equal },
				{ "!=", TokenType::notequal },
				{ "!", TokenType::lognot },
				{ "\\?", TokenType::selector },

				{ "$", TokenType::curline}
			},
			{
				{ "0x[0-9a-fA-F]+", TokenType::hexnum },
				{ "[0-9]+", TokenType::decnum },
				{ "0o[0-7]+", TokenType::octnum },
				{ "0b[01]+", TokenType::binnum },
 
				{ "#[0-9]+", TokenType::macro_arg },

				{ "[a-zA-Z_][a-zA-Z0-9_]*", TokenType::identifier },
				{ "##[a-zA-Z_][a-zA-Z0-9_]*", TokenType::identifier_unique },

				{ "\'([^\\\\\n\t\']|(\\\\([nt\\\\]|([0-9]{1,3}))))\'", TokenType::character},
				{ "\"[^\n\"]*([^\n\"]+(\\\\[\n\"])+)*\"", TokenType::string },

				{ ";[^\n]*", TokenType::comment },
				{ "/\\*[^*]*\\*+([^/*][^*]*\\*+)*/", TokenType::comment },

				{ "[ \t]+", TokenType::whitespace },
				{ "\n", TokenType::newline }
			}
		};

		std::unordered_map<TokenType, std::string> tokenStr{
			{ TokenType::add, "add" },
			{ TokenType::addc, "addc" },
			{ TokenType::addi, "addi" },
			{ TokenType::sub, "sub" },
			{ TokenType::subc, "subc" },
			{ TokenType::subi, "subi" },
			{ TokenType::bxr, "bxr" },
			{ TokenType::bxri, "bxri" },
			{ TokenType::bor, "bor" },
			{ TokenType::bori, "bori" },
			{ TokenType::bnd, "bnd" },
			{ TokenType::bndi, "bndi" },
			{ TokenType::shiftl, "shiftl" },
			{ TokenType::shiftlc, "shiftlc" },
			{ TokenType::shiftli, "shiftli" },
			{ TokenType::shiftr, "shiftr" },
			{ TokenType::shiftrc, "shiftrc" },
			{ TokenType::shiftri, "shiftri" },
			{ TokenType::cmp, "cmp" },
			{ TokenType::cmpi, "cmpi" },
			{ TokenType::test, "test" },
			{ TokenType::testi, "testi" },
			{ TokenType::mov, "mov" },
			{ TokenType::set, "set" },
			{ TokenType::sets, "sets" },
			{ TokenType::block, "block" },
			{ TokenType::push, "push" },
			{ TokenType::pop, "pop" },
			{ TokenType::ld, "ld" },
			{ TokenType::st, "st" },
			{ TokenType::jmp, "jmp" },
			{ TokenType::ijmp, "ijmp" },
			{ TokenType::call, "call" },
			{ TokenType::ret, "ret" },
			{ TokenType::nop, "nop" },
			{ TokenType::brk, "brk" },
			{ TokenType::wait, "wait" },
			{ TokenType::halt, "halt" },

			{ TokenType::syscall, "syscall" },
			{ TokenType::iret, "iret" },
			{ TokenType::intervec, "intervec" },
			{ TokenType::interpri, "interpri" },

			{ TokenType::stexp, "stexp" },
			{ TokenType::ldexp, "ldexp" },
			{ TokenType::pctl, "pctl" },
			{ TokenType::getabs, "getabs" },
			{ TokenType::flush, "flush" },
			{ TokenType::inval, "inval" },

			{ TokenType::istack, "istack" },
			{ TokenType::panicvec, "panicvec" },
			{ TokenType::cpuid, "cpuid" },
			{ TokenType::intlevel, "intlevel" },
			{ TokenType::pagebase, "pagebase" },
			{ TokenType::enable, "enable" },
			{ TokenType::disable, "disable" },
			{ TokenType::L1D, "L1D" },
			{ TokenType::L1I, "L1I" },
			{ TokenType::TLB, "TLB" },
			{ TokenType::L2, "L2" },
			{ TokenType::ALL, "ALL" },

			{ TokenType::reg, "reg" },
			{ TokenType::gen, "gen" },
			{ TokenType::sbp, "sbp" },
			{ TokenType::zero, "zero" },
			{ TokenType::one, "one" },
			{ TokenType::full, "full" },
			{ TokenType::pc, "pc" },
			{ TokenType::stack, "stack" },
			{ TokenType::flag, "flag" },

			{ TokenType::neg, "neg" },
			{ TokenType::pos, "pos" },
			{ TokenType::carry, "carry" },
			{ TokenType::carry4, "carry4" },
			{ TokenType::overflow, "overflow" },

			{ TokenType::_32B_, "32B" },
			{ TokenType::_16L_, "16L" },
			{ TokenType::_8L_, "8L" },
			{ TokenType::_8H_, "8H" },
			{ TokenType::_S16H_, "S16H" },
			{ TokenType::_S16L_, "S16L" },
			{ TokenType::_S8L_, "S8L" },
			{ TokenType::_S8H_, "S8H" },

			{ TokenType::text, ".text" },
			{ TokenType::data, ".data" },
			{ TokenType::bss, ".bss" },

			{ TokenType::define, "define" },
			{ TokenType::macro, "macro" },
			{ TokenType::end, "end" },

			{ TokenType::macro_arg, "macro_arg" },
			{ TokenType::identifier_unique, "identifier_unique" },

			{ TokenType::dot, "dot" },
			{ TokenType::comma, "comma" },
			{ TokenType::hash, "hash" },
			{ TokenType::openparen, "openparen" },
			{ TokenType::closeparen, "closeparen" },
			{ TokenType::openbrace, "openbrace" },
			{ TokenType::closebrace, "closebrace" },
			{ TokenType::openbrack, "openbrack" },
			{ TokenType::closebrack, "closebrack" },

			{ TokenType::colon, "colon" },
			{ TokenType::semicolon, "semicolon" },
			{ TokenType::plus, "plus" },
			{ TokenType::minus, "minus" },
			{ TokenType::dstar, "dstar" },
			{ TokenType::star, "star" },
			{ TokenType::slash, "slash" },
			{ TokenType::percent, "percent" },
			{ TokenType::tilde, "tilde" },
			{ TokenType::ampersend, "ampersend" },
			{ TokenType::verticalbar, "verticalbar" },
			{ TokenType::caret, "caret" },
			{ TokenType::leftshift, "leftshift" },
			{ TokenType::rightshift, "rightshift" },

			{ TokenType::less, "less" },
			{ TokenType::greater, "greater" },
			{ TokenType::lessequal, "lessequal" },
			{ TokenType::greaterequal, "greaterequal" },
			{ TokenType::equal, "equal" },
			{ TokenType::notequal, "notequal" },
			{ TokenType::lognot, "lognot" },
			{ TokenType::logampersend, "logampersend" },
			{ TokenType::logverticalbar, "logverticalbar" },
			{ TokenType::selector, "selector" },

			{ TokenType::hexnum, "hexnum" },
			{ TokenType::decnum, "decnum" },
			{ TokenType::octnum, "octnum" },
			{ TokenType::binnum, "binnum" },
			{ TokenType::comment, "comment" },
			{ TokenType::whitespace, "whitespace" },

			{ TokenType::newline, "newline" },
			{ TokenType::identifier, "identifier" },
			{ TokenType::character, "character" },
			{ TokenType::string, "string" },

			{ TokenType::__epsilon, "epsilon" },
			{ TokenType::__unknown, "unknown" }
		};

		LexerFactory lexerFactory;
		Lexer lexer;

		inline void createLexer() {
			lexerFactory.setRules(CreateData);
			lexerFactory.update();
			lexer = lexerFactory.create();
		}

		inline void createLexer(const LexerFactory::UpdateData& data) {
			lexerFactory.directUpdate(data);
			lexer = lexerFactory.create();
		}

		inline void getFactoryData(LexerFactory::UpdateData& data) {
			lexerFactory.getData(data);
		}
	}

	namespace preprocesser {
		using u64 = unsigned __int64;

		using TokenType = lexer::TokenType;
		using Token = lexer::Token;

		using Preprocesser = preproccesser_generator::Preprocesser<Token>;
		using pPreprocFunc = Preprocesser::PreprocessFunction;
		using Vec = Preprocesser::Vec;
		using Iter = Preprocesser::Iter;

		namespace functions {
			int dec2int(std::string text) {
				bool neg = false;
				int ans = 0;

				auto it = text.begin();
				if (*it == '-') {
					neg = true;
					++it;
				}
				if (it != text.end() && *it == '0') ++it;
				if (it != text.end() && *it == 'd') ++it;

				for (it; it != text.end(); ++it) {
					ans *= 10;
					if ('0' <= *it && *it <= '9')
						ans += *it - '0';
				}
				if (neg)
					ans = -ans;
				return ans;
			}

			int macroArg2int(std::string text) {
				auto it = text.begin();
				if (it != text.end()) it++;

				int ans = 0;
				for (it; it != text.end(); ++it) {
					ans *= 10;
					if ('0' <= *it && *it <= '9')
						ans += *it - '0';
				}
				return ans;
			}

			enum class State {
				normal,
				readDefine_identifier,
				readDefine_token,

				readMacro_identifier,
				readMacro_argCount,
				readMacro_token,

				replace_getArg,
				replace_comma,
				replace
			};

			typedef struct _PreprocData {
				State state;

				std::string curReplaceString;
				std::vector<Token> curReplaceTokens;
				int curMacroArgCount;
				int depth;
				std::unordered_map<std::string, std::pair<std::vector<Token>, int>> replaceTable;

				std::vector<Token> curArg;
				std::vector<std::vector<Token>> args;
				int remainingArg;

				std::vector<Token> replaceTokens;

				int uniqueCounter;
			} PreprocData;

			bool handleReplace(Vec tokens, Iter it, void* vpdata) {
				PreprocData* data = (PreprocData*)vpdata;

				switch (data->state) {
				case State::replace_getArg:
					switch (it->type) {
					case TokenType::comma:
						if (data->depth == 1) {
							data->args.push_back(data->curArg);
							data->curArg.clear();
							data->remainingArg--;
						}
						else
							data->curArg.push_back(*it);
						it = tokens.erase(it);
						break;
					case TokenType::openparen:
						if (data->depth >= 1)
							data->curArg.push_back(*it);
						data->depth++;
						it = tokens.erase(it);
						break;
					case TokenType::closeparen:
						data->depth--; 
						if (!data->depth) {
							if (data->args.size()) {
								data->args.push_back(data->curArg);
								data->curArg.clear();
								data->remainingArg--;
							}
						}
						else
							data->curArg.push_back(*it);
						it = tokens.erase(it);
						break;
					default:
						data->curArg.push_back(*it);
						it = tokens.erase(it);
						break;
					}
					if (!data->remainingArg && !data->depth) {
						for (auto si = data->replaceTokens.begin(); si != data->replaceTokens.end(); ) {
							if (si->type == TokenType::macro_arg) {
								int id = macroArg2int(si->text) - 1;
								if (id < 0 || id >= data->args.size())
									return false;
								si = data->replaceTokens.erase(si);
								si = data->replaceTokens.insert(si, data->args[id].begin(), data->args[id].end());
							}
							else if (si->type == TokenType::identifier_unique) {
								Token tmp = {};
								tmp.text = si->text + "#" + std::to_string(data->uniqueCounter);
								tmp.type = TokenType::identifier;
								si = data->replaceTokens.erase(si);
								si = data->replaceTokens.insert(si, tmp);
							}
							else
								++si;
						}
						data->uniqueCounter++;
						it = tokens.insert(it, data->replaceTokens.begin(), data->replaceTokens.end());
						data->state = State::normal;
						return true;
					}
					break;

				case State::normal:
					switch (it->type) {
					case TokenType::define:
						data->state = State::readDefine_identifier;
						it = tokens.erase(it);
						break;

					case TokenType::macro:
						data->state = State::readMacro_identifier;
						it = tokens.erase(it);
						break;

					case TokenType::identifier: {
						auto find = data->replaceTable.find(it->text);
						if (find == data->replaceTable.end()) {
							++it;
							break;
						}

						it = tokens.erase(it);

						if (find->second.second == -1) {
							it = tokens.insert(it, find->second.first.begin(), find->second.first.end());
							break;
						}

						data->state = State::replace_getArg;
						data->replaceTokens = find->second.first;
						data->remainingArg = find->second.second;
						data->args.clear();
						data->depth = 0;

						break;
					}

					default:
						it++;
						break;
					}
					break;
				case State::readDefine_identifier:
					if (it->type == TokenType::identifier) {
						data->state = State::readDefine_token;
						data->curReplaceString = it->text;
						data->depth = 0;
						it = tokens.erase(it);
					}
					else
						return false;
					break;
				case State::readDefine_token:
					switch (it->type) {
					case TokenType::openparen:
						data->depth++;
						data->curReplaceTokens.push_back(*it);
						it = tokens.erase(it);
						break;
					case TokenType::closeparen:
						data->depth--;
						data->curReplaceTokens.push_back(*it);
						it = tokens.erase(it);
						break;
					default:
						data->curReplaceTokens.push_back(*it);
						it = tokens.erase(it);
						break;
					}
					if (!data->depth) {
						data->state = State::normal;
						data->replaceTable.insert({ data->curReplaceString, { data->curReplaceTokens, -1 } });
						data->curReplaceTokens.clear();
					}
					break;

				case State::readMacro_identifier:
					if (it->type == TokenType::identifier) {
						data->state = State::readMacro_argCount;
						data->curReplaceString = it->text;
						it = tokens.erase(it);
					}
					else
						return false;
					break;
				case State::readMacro_argCount:
					if (it->type == TokenType::decnum) {
						data->state = State::readMacro_token;
						data->curMacroArgCount = dec2int(it->text);
						it = tokens.erase(it);
					}
					else
						return false;
					break;
				case State::readMacro_token:
					switch (it->type) {
					case TokenType::end:
						data->state = State::normal;
						data->replaceTable.insert({ data->curReplaceString, { data->curReplaceTokens, data->curMacroArgCount } });
						data->curReplaceTokens.clear();
						it = tokens.erase(it);
						break;
					default:
						data->curReplaceTokens.push_back(*it);
						it = tokens.erase(it);
						break;
					}
					break;
				}

				return true;
			}

			bool removeUnused(Vec tokens, Iter it, void* vpdata) {
				PreprocData* data = (PreprocData*)vpdata;

				switch (it->type) {
				case TokenType::whitespace:
				case TokenType::newline:
				case TokenType::comment:
					it = tokens.erase(it);
					break;

				default:
					it++;
					break;
				}

				return true;
			}
		}

		void tagLine(std::vector<lexer::Token>& tokens) {
			int curLine = 1;
			for (auto it = tokens.begin(); it != tokens.end(); ++it) {
				it->line = curLine;
				if (it->type == lexer::TokenType::newline)
					curLine++;
			}
		}

		Preprocesser preprocesser;

		inline void setTokens(std::vector<lexer::Token>& tokens) {
			preprocesser.setTokens(tokens);
		}

		inline bool preprocess(functions::PreprocData& data) {
			data.state = functions::State::normal;
			data.depth = 0;
			data.uniqueCounter = 0;
			if (!preprocesser.preprocess(functions::removeUnused, &data)) return false;
			if (!preprocesser.preprocess(functions::handleReplace, &data)) return false;

			return true;
		}
	}

	namespace parser {
		using u64 = unsigned __int64;

		using TokenType = lexer::TokenType;

		NTSTART(NonterminalType)
			program,
			section,

			text_section,
			data_section,
			bss_section,
			outer_section,

			intruction,
			label,

			allocate,
			allocate_zero,

			reg_ex,

			reg,
			regid,
			regmode,
			reg_gen,
			reg_reg,

			flagid,

			ctlid,
			cacheid,

			index,

			immidate16,
			immidate8,

			expression,
			expressionL0,
			expressionL1,
			expressionL2,
			expressionL3,
			expressionL4,
			expressionL5,
			expressionL6,
			expressionL7,
			expressionL8,
			expressionL9,
			expressionL10,
			expressionL11,
			expressionL12,
			expressionL13,

			number,
			NTEND();

		enum class ASTNodeType {
			__epsilon = -1,
			__temp = 0,
			__accept = 1,

			program,

			text_section,
			data_section,
			bss_section,
			outer_section,

			allocate,
			allocate_zero,

			instruction_N,
			instruction_R,
			instruction_RR,
			instruction_RI,
			instruction_II,
			instruction_RII,
			instruction_IRI,
			instruction_RVRI,
			instruction_FVRI,

			instruction_IXP,
			instruction_RXR,
			instruction_CTL,
			instruction_CACHE,

			label,

			reg_ex,

			reg,
			regid,
			regmode,
			reg_index,

			flagid,

			ctlid,
			cacheid,

			index,

			immidate16,
			immidate8,

			expression,
			operator_,
			operation,

			hex,
			dec,
			oct,
			bin
		};

		std::unordered_map<NonterminalType, std::string> nonterminalStr{
			{ NonterminalType::__accept, "accept" },

			{ NonterminalType::program, "program" },
			{ NonterminalType::section, "section" },

			{ NonterminalType::text_section, "text_section" },
			{ NonterminalType::data_section, "data_section" },
			{ NonterminalType::bss_section, "bss_section" },
			{ NonterminalType::outer_section, "outer_section" },

			{ NonterminalType::intruction, "intruction" },
			{ NonterminalType::label, "label" },
			{ NonterminalType::allocate, "allocate" },
			{ NonterminalType::allocate_zero, "allocate_zero" },

			{ NonterminalType::reg_ex, "reg_ex"},

			{ NonterminalType::reg, "reg" },
			{ NonterminalType::regid, "regid" },
			{ NonterminalType::regmode, "regmode" },
			{ NonterminalType::reg_gen, "reg_gen" },
			{ NonterminalType::reg_reg, "reg_reg" },

			{ NonterminalType::flagid, "flagid" },

			{ NonterminalType::ctlid, "ctlid" },
			{ NonterminalType::cacheid, "cacheid" },

			{ NonterminalType::index, "index" },

			{ NonterminalType::immidate16, "immidate16" },
			{ NonterminalType::immidate8, "immidate8" },

			{ NonterminalType::expression, "expression" },

			{ NonterminalType::expressionL0, "expressionL0" },
			{ NonterminalType::expressionL1, "expressionL1" },
			{ NonterminalType::expressionL2, "expressionL2" },
			{ NonterminalType::expressionL3, "expressionL3" },
			{ NonterminalType::expressionL4, "expressionL4" },
			{ NonterminalType::expressionL5, "expressionL5" },
			{ NonterminalType::expressionL6, "expressionL6" },
			{ NonterminalType::expressionL7, "expressionL7" },
			{ NonterminalType::expressionL8, "expressionL8" },
			{ NonterminalType::expressionL9, "expressionL9" },
			{ NonterminalType::expressionL10, "expressionL10" },
			{ NonterminalType::expressionL11, "expressionL11" },
			{ NonterminalType::expressionL12, "expressionL12" },
			{ NonterminalType::expressionL13, "expressionL13" },

			{ NonterminalType::number, "number" }
		};

		std::unordered_map<ASTNodeType, std::string> asttypeStr = {
			{ ASTNodeType::__epsilon, "epsilon" },
			{ ASTNodeType::__temp, "temp" },
			{ ASTNodeType::__accept, "accept" },

			{ ASTNodeType::program, "program" },

			{ ASTNodeType::text_section, "text_section" },
			{ ASTNodeType::data_section, "data_section" },
			{ ASTNodeType::bss_section, "bss_section" },
			{ ASTNodeType::outer_section, "outer_section" },

			{ ASTNodeType::instruction_N, "instruction_N" },
			{ ASTNodeType::instruction_R, "instruction_R" },
			{ ASTNodeType::instruction_RR, "instruction_RR" },
			{ ASTNodeType::instruction_RI, "instruction_RI" },
			{ ASTNodeType::instruction_II, "instruction_II" },
			{ ASTNodeType::instruction_RII, "instruction_RII" },
			{ ASTNodeType::instruction_IRI, "instruction_IRI" },
			{ ASTNodeType::instruction_RVRI, "instruction_RVRI" },
			{ ASTNodeType::instruction_FVRI, "instruction_FVRI" },
			{ ASTNodeType::instruction_CTL, "instruction_CTL" },
			{ ASTNodeType::instruction_CACHE, "instruction_CACHE" },

			{ ASTNodeType::allocate, "allocate" },
			{ ASTNodeType::allocate_zero, "allocate_zero" },
			{ ASTNodeType::label, "label" },

			{ ASTNodeType::reg_ex, "reg_ex" },

			{ ASTNodeType::reg, "reg" },
			{ ASTNodeType::regid, "regid" },
			{ ASTNodeType::regmode, "regmode" },
			{ ASTNodeType::reg_index, "reg_index" },

			{ ASTNodeType::flagid, "flagid" },

			{ ASTNodeType::ctlid, "ctlid" },
			{ ASTNodeType::cacheid, "cacheid" },

			{ ASTNodeType::index, "index" },

			{ ASTNodeType::immidate16, "immidate16" },
			{ ASTNodeType::immidate8, "immidate8" },

			{ ASTNodeType::expression, "expression" },
			{ ASTNodeType::operator_, "operator" },
			{ ASTNodeType::operation, "operation" },

			{ ASTNodeType::hex, "hex" },
			{ ASTNodeType::dec, "dec" },
			{ ASTNodeType::oct, "oct" },
			{ ASTNodeType::bin, "bin" }
		};

		using ParserFactory = parser_generator::ParserFactory<TokenType, NonterminalType, ASTNodeType>;
		using PFCD = ParserFactory::CreateData;
		using Parser = parser_generator::Parser<TokenType, NonterminalType, ASTNodeType>;
		using Tree = parser_generator::PTNode;
		using NT = NonterminalType;
		using TT = TokenType;
		using AT = ASTNodeType;

		const PFCD CreateData = {
			{ { NT::__accept, AT::__accept, -1 }, { { NT::program, true, false } } },

			{ { NT::program, AT::program, -1}, { { NT::section, true, true } } },

			{ { NT::section, AT::text_section, -1 }, { { TT::text, false , false }, { NT::text_section, true, false }, { NT::section, true, true } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false }, { NT::data_section, true, false }, { NT::section, true, true } } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false }, { NT::bss_section, true, false }, { NT::section, true, true } } },
			{ { NT::section, AT::text_section, -1 }, { { TT::text, false, false }, { TT::identifier, true, false}, { NT::text_section, true, false }, { NT::section, true, true } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false }, { TT::identifier, true, false}, { NT::data_section, true, false }, { NT::section, true, true } } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false }, { TT::identifier, true, false}, { NT::bss_section, true, false }, { NT::section, true, true } } },
			{ { NT::section, AT::outer_section, -1 }, { { NT::outer_section, true, false }, { NT::section, true, true } } },

			{ { NT::section, AT::text_section, -1 }, { { TT::text, false, false }, { NT::text_section, true, false } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false }, { NT::data_section, true, false} } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false }, { NT::bss_section, true, false } } },
			{ { NT::section, AT::outer_section, -1 }, { { NT::outer_section, true, false } } },

			{ { NT::section, AT::text_section, -1 }, { { TT::text, false , false }, { NT::section, true, true } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false }, { NT::section, true, true } } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false }, { NT::section, true, true } } },

			{ { NT::section, AT::text_section, -1 }, { { TT::text, false, false } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false } } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false } } },

			{ { NT::data_section, AT::data_section, -1 }, { { NT::allocate, true, false }, { NT::data_section, true, true } } },
			{ { NT::data_section, AT::data_section, -1 }, { { NT::label, true, false }, { NT::data_section, true, true } } },

			{ { NT::data_section, AT::data_section, -1 }, { { NT::allocate, true, false } } },
			{ { NT::data_section, AT::data_section, -1 }, { { NT::label, true, false } } },

			{ { NT::bss_section, AT::bss_section, -1 }, { { NT::allocate_zero, true, false }, { NT::bss_section, true, true } } },
			{ { NT::bss_section, AT::bss_section, -1 }, { { NT::label, true, false }, { NT::bss_section, true, true } } },

			{ { NT::bss_section, AT::bss_section, -1 }, { { NT::allocate_zero, true, false } } },
			{ { NT::bss_section, AT::bss_section, -1 }, { { NT::label, true, false } } },

			{ { NT::text_section, AT::text_section, -1 }, { { NT::intruction, true, false }, { NT::text_section, true, true } } },
			{ { NT::text_section, AT::text_section, -1 }, { { NT::label, true, false }, { NT::text_section, true, true } } },

			{ { NT::text_section, AT::text_section, -1 }, { { NT::intruction, true, false } } },
			{ { NT::text_section, AT::text_section, -1 }, { { NT::label, true, false } } },

			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::add, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::addc, true, false }, {NT::reg, true, false }, { TT::comma, false, false } ,{ NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::addi, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::sub, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::subc, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::subi, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::bxr, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::bxri, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::bor, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::bori, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::bnd, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::bndi, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::shiftl, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::shiftlc, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::shiftli, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::shiftr, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::shiftrc, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::shiftri, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::ror, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::rori, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::rol, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::roli, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::cmp, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::cmpi, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::test, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::testi, true, false }, {NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },

			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::set, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::sets, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, {NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RR, -1 }, { { TT::mov, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, {NT::reg, true, false } } },

			{ { NT::intruction, AT::instruction_N, -1 }, { { TT::block, true, false } } },

			{ { NT::intruction, AT::instruction_R, -1 }, { { TT::push, true, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_R, -1 }, { { TT::pop, true, false }, {NT::reg, true, false } } },

			{ { NT::intruction, AT::instruction_RVRI, -1 }, { { TT::ld, true, false }, { TT::dot, false, false }, { NT::regid, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RVRI, -1 }, { { TT::st, true, false }, { TT::dot, false, false }, { NT::regid, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },

			{ { NT::intruction, AT::instruction_FVRI, -1 }, { { TT::jmp, true, false }, { TT::dot, false, false }, { NT::flagid, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_FVRI, -1 }, { { TT::ijmp, true, false }, { TT::dot, false, false }, { NT::flagid, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_RI, -1 }, { { TT::call, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate16, true, false } } },
			{ { NT::intruction, AT::instruction_N, -1 }, { { TT::ret, true, false } } },
			{ { NT::intruction, AT::instruction_N, -1 }, { { TT::nop, true, false } } },
			{ { NT::intruction, AT::instruction_N, -1},  { { TT::brk, true, false } } },
			{ { NT::intruction, AT::instruction_N, -1 }, { { TT::wait, true, false } } },
			{ { NT::intruction, AT::instruction_N, -1 }, { { TT::halt, true, false } } },

			{ { NT::intruction, AT::instruction_N, -1}, { { TT::syscall, true, false } } },
			{ { NT::intruction, AT::instruction_N, -1}, { { TT::iret, true, false } } },
			{ { NT::intruction, AT::instruction_RII, -1}, { { TT::intervec, true, false }, { TT::reg, true, false }, { TT::comma, false, false },  { NT::immidate8, true, false }, { TT::comma, false, false }, { NT::immidate8, true, false } } },
			{ { NT::intruction, AT::instruction_IXP, -1}, { { TT::interpri, true, false }, { NT::immidate8, true, false }, { TT::comma, false, false },  { NT::immidate8, true, false }  } },

			{ { NT::intruction, AT::instruction_RXR, -1}, { { TT::ldexp, true, false}, { NT::reg, true, false } , { TT::comma, false, false }, { NT::reg_ex, true, false } } },
			{ { NT::intruction, AT::instruction_RXR, -1}, { { TT::stexp, true, false}, { NT::reg, true, false } , { TT::comma, false, false }, { NT::reg_ex, true, false } } },

			{ { NT::intruction, AT::instruction_CTL, -1}, { { TT::pctl, true, false }, { NT::ctlid, true, false } } },

			{ { NT::intruction, AT::instruction_RR, -1}, { { TT::getabs, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::reg, true, false } } },

			{ { NT::intruction, AT::instruction_CACHE, -1}, { { TT::flush, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate8, true, false }, { TT::comma, false, false }, { NT::cacheid, true, false } } },
			{ { NT::intruction, AT::instruction_CACHE, -1}, { { TT::inval, true, false }, { NT::reg, true, false }, { TT::comma, false, false }, { NT::immidate8, true, false }, { TT::comma, false, false }, { NT::cacheid, true, false } } },

			{ { NT::reg_ex, AT::reg_ex, -1}, { { TT::istack, true, false } } },
			{ { NT::reg_ex, AT::reg_ex, -1}, { { TT::panicvec, true, false } } },
			{ { NT::reg_ex, AT::reg_ex, -1}, { { TT::cpuid, true, false } } },
			{ { NT::reg_ex, AT::reg_ex, -1}, { { TT::intlevel, true, false } } },
			{ { NT::reg_ex, AT::reg_ex, -1}, { { TT::pagebase, true, false } } },

			{ { NT::ctlid, AT::ctlid, -1 }, { { TT::enable, true, false } } },
			{ { NT::ctlid, AT::ctlid, -1 }, { { TT::disable, true, false } } },

			{ { NT::cacheid, AT::cacheid, -1 }, { { TT::ALL, true, false } } },
			{ { NT::cacheid, AT::cacheid, -1 }, { { TT::L2, true, false } } },
			{ { NT::cacheid, AT::cacheid, -1 }, { { TT::TLB, true, false } } },
			{ { NT::cacheid, AT::cacheid, -1 }, { { TT::L1D, true, false } } },
			{ { NT::cacheid, AT::cacheid, -1 }, { { TT::L1I, true, false } } },

			{ { NT::reg, AT::reg, 0 }, { { NT::regid, true, false }, { TT::dot, false, false }, { NT::regmode, true, false } } },

			{ { NT::regid, AT::__epsilon, 0 }, { { NT::reg_gen, true, true } } },
			{ { NT::regid, AT::__epsilon, 0 }, { { NT::reg_reg, true, true } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::sbp, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::zero, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::one, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::full, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::pc, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::stack, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::flag, true, false } } },

			{ { NT::reg_gen, AT::reg_index, -1 }, { { TT::gen, true, false }, { NT::index, true, false } } },
			{ { NT::reg_reg, AT::reg_index, -1 }, { { TT::reg, true, false }, { NT::index, true, false } } },

			{ { NT::regmode, AT::regmode, -1 }, { { TT::_32B_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_16L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S16H_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_8L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_8H_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S16L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S8L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S8H_, true, false } } },

			{ { NT::flagid, AT::flagid, -1 }, { { TT::zero, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::neg, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::pos, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::carry, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::carry4, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::overflow, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::one, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::gen, true, false } } },

			{ { NT::immidate16, AT::immidate16, -1 }, { { NT::expression, true, false } } },
			{ { NT::immidate8, AT::immidate8, -1 }, { { NT::expression, true, false } } },

			{ { NT::label, AT::label, 0 }, { { TT::identifier, false, false }, { TT::colon, false, false } } },

			{ { NT::allocate, AT::allocate, 0 }, { { NT::expression, true, false }, { TT::comma, false, false },  { NT::expression, true, false } } },
			{ { NT::allocate_zero, AT::allocate_zero, 0 }, { { NT::expression, true, false } } },

			{ { NT::index, AT::index, -1 }, { { TT::openbrack, false, false }, { NT::expression, true, false }, { TT::closebrack, false, false } } },

			{ { NT::expression, AT::__epsilon, 0 }, { { NT::expressionL13, true, true } } },

			{ { NT::expressionL13, AT::__epsilon, 1 }, { { NT::expressionL12, true, false }, { TT::selector, false, false }, { NT::expressionL13, true, false }, { TT::colon, false, false }, { NT::expressionL13, true, false } } },
			{ { NT::expressionL13, AT::__epsilon, 0 }, { { NT::expressionL12, true, true } } },

			{ { NT::expressionL12, AT::__epsilon, 1 }, { { NT::expressionL12, true, false }, { TT::logverticalbar, false, false }, { NT::expressionL11, true, false } } },
			{ { NT::expressionL12, AT::__epsilon, 0 }, { { NT::expressionL11, true, true } } },

			{ { NT::expressionL11, AT::__epsilon, 1 }, { { NT::expressionL11, true, false }, { TT::logampersend, false, false }, { NT::expressionL10, true, false } } },
			{ { NT::expressionL11, AT::__epsilon, 0 }, { { NT::expressionL10, true, true } } },

			{ { NT::expressionL10, AT::__epsilon, 1 }, { { NT::expressionL10, true, false }, { TT::verticalbar, false, false }, { NT::expressionL9, true, false } } },
			{ { NT::expressionL10, AT::__epsilon, 0 }, { { NT::expressionL9, true, true } } },

			{ { NT::expressionL9, AT::__epsilon, 1 }, { { NT::expressionL9, true, false }, { TT::caret, false, false }, { NT::expressionL8, true, false } } },
			{ { NT::expressionL9, AT::__epsilon, 0 }, { { NT::expressionL8, true, true } } },

			{ { NT::expressionL8, AT::__epsilon, 1 }, { { NT::expressionL8, true, false }, { TT::ampersend, false, false }, { NT::expressionL7, true, false } } },
			{ { NT::expressionL8, AT::__epsilon, 0 }, { { NT::expressionL7, true, true } } },

			{ { NT::expressionL7, AT::__epsilon, 1 }, { { NT::expressionL7, true, false }, { TT::equal, false, false }, { NT::expressionL6, true, false } } },
			{ { NT::expressionL7, AT::__epsilon, 1 }, { { NT::expressionL7, true, false }, { TT::notequal, false, false }, { NT::expressionL6, true, false } } },
			{ { NT::expressionL7, AT::__epsilon, 0 }, { { NT::expressionL6, true, true } } },

			{ { NT::expressionL6, AT::__epsilon, 1 }, { { NT::expressionL6, true, false }, { TT::less, false, false }, { NT::expressionL5, true, false } } },
			{ { NT::expressionL6, AT::__epsilon, 1 }, { { NT::expressionL6, true, false }, { TT::greater, false, false }, { NT::expressionL5, true, false } } },
			{ { NT::expressionL6, AT::__epsilon, 1 }, { { NT::expressionL6, true, false }, { TT::lessequal, false, false }, { NT::expressionL5, true, false } } },
			{ { NT::expressionL6, AT::__epsilon, 1 }, { { NT::expressionL6, true, false }, { TT::greaterequal, false, false }, { NT::expressionL5, true, false } } },
			{ { NT::expressionL6, AT::__epsilon, 0 }, { { NT::expressionL5, true, true } } },

			{ { NT::expressionL5, AT::__epsilon, 1 }, { { NT::expressionL5, true, false }, { TT::leftshift, false, false }, { NT::expressionL4, true, false } } },
			{ { NT::expressionL5, AT::__epsilon, 1 }, { { NT::expressionL5, true, false }, { TT::rightshift, false, false }, { NT::expressionL4, true, false } } },
			{ { NT::expressionL5, AT::__epsilon, 0 }, { { NT::expressionL4, true, true } } },

			{ { NT::expressionL4, AT::__epsilon, 1 }, { { NT::expressionL4, true, false }, { TT::plus, false, false }, { NT::expressionL3, true, false } } },
			{ { NT::expressionL4, AT::__epsilon, 1 }, { { NT::expressionL4, true, false }, { TT::minus, false, false }, { NT::expressionL3, true, false } } },
			{ { NT::expressionL4, AT::__epsilon, 0 }, { { NT::expressionL3, true, true } } },

			{ { NT::expressionL3, AT::__epsilon, 1 }, { { NT::expressionL3, true, false }, { TT::star, false, false }, { NT::expressionL2, true, false } } },
			{ { NT::expressionL3, AT::__epsilon, 1 }, { { NT::expressionL3, true, false }, { TT::slash, false, false }, { NT::expressionL2, true, false } } },
			{ { NT::expressionL3, AT::__epsilon, 1 }, { { NT::expressionL3, true, false }, { TT::percent, false, false }, { NT::expressionL2, true, false} } },
			{ { NT::expressionL3, AT::__epsilon, 0 }, { { NT::expressionL2, true, true } } },

			{ { NT::expressionL2, AT::__epsilon, 1 }, { { NT::expressionL1, true, false }, { TT::dstar, false, false }, { NT::expressionL2, true, false } } },
			{ { NT::expressionL2, AT::__epsilon, 0 }, { { NT::expressionL1, true, true } } },

			{ { NT::expressionL1, AT::__epsilon, 0 }, { { TT::tilde, false, false }, { NT::expressionL0, true, false } } },
			{ { NT::expressionL1, AT::__epsilon, 0 }, { { TT::lognot, false, false }, { NT::expressionL0, true, false } } },
			{ { NT::expressionL1, AT::__epsilon, 0 }, { { TT::plus, false, false }, { NT::expressionL0, true, false } } },
			{ { NT::expressionL1, AT::__epsilon, 0 }, { { TT::minus, false, false }, { NT::expressionL0, true, false } } },
			{ { NT::expressionL1, AT::__epsilon, 0 }, { { NT::expressionL0, true, true } } },

			{ { NT::expressionL0, AT::__epsilon, 1 }, { { TT::openparen, false, false }, { NT::expressionL13, true, true }, { TT::closeparen, false, false } } },
			{ { NT::expressionL0, AT::__epsilon, 0 }, { { NT::number, false , false } } },
			{ { NT::expressionL0, AT::__epsilon, 0 }, { { TT::identifier, false , false } } },

			{ { NT::number, AT::__epsilon, 0 }, { { TT::hexnum, false, false } } },
			{ { NT::number, AT::__epsilon, 0 }, { { TT::decnum, false, false } } },
			{ { NT::number, AT::__epsilon, 0 }, { { TT::octnum, false, false } } },
			{ { NT::number, AT::__epsilon, 0 }, { { TT::binnum, false, false } } },
			{ { NT::number, AT::__epsilon, 0 }, { { TT::character, false, false } } }
		};

		ParserFactory parserFactory;
		Parser parser;

		inline void createParser(void) {
			parserFactory.setRules(CreateData);

			parserFactory.update();
			parser = parserFactory.create();
		}

		inline void createParser(ParserFactory::UpdateData& data) {
			parserFactory.directUpdate(data);
			parser = parserFactory.create();
		}

		inline void getFactoryData(ParserFactory::UpdateData& data) {
			parserFactory.getData(data);
		}
	}

	namespace evaluator {
		using u64 = unsigned __int64;
		using u32 = unsigned __int32;
		using s32 = __int32;
		using u8 = unsigned __int8;

		using TokenType = lexer::TokenType;
		using NonterminalType = parser::NonterminalType;
		using ASTNodeType = parser::ASTNodeType;
		using AST = parser::Parser::ASTNode;

		using Evaluator = evaluator_generator::Evaluator<TokenType, NonterminalType, ASTNodeType>;
		using pEvalFunc = Evaluator::EvaluateFunction;

		namespace functions {
			using SectionType = format_data::SectionType;
			using OperationType = format_data::OperationType;
			
			enum class ExprType {
				CONSTANT,
				LINEAR,
				COMPLEX
			};
			
			
			typedef struct _ReplaceData {
				u32 index;
				u32 size;
				u32 offset;
			} ReplaceData;

			typedef struct _Section {
				SectionType type;
				std::string name;
				std::vector<u8> data;
				std::unordered_multimap<u32, ReplaceData> operationMap;
			} Section;
			
			typedef struct _OperationToken {
				OperationType type;
				std::string text;
				s32 value;
			} OperationToken;
			
			typedef struct _HashOperation {
				size_t operator()(const std::vector<OperationToken>& k) const {
					size_t h = 0;
					for (auto it = k.cbegin(); it != k.cend(); ++it) {
						size_t h1 = std::hash<OperationType>()(it->type);
						size_t h2 = std::hash<s32>()(it->value);
						size_t h3 = h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
						h <<= 1;
						h += h3;
					}
					return h;
				}
			} HashOperation;

			typedef struct _EqualOperation {
				bool  operator()(const std::vector<OperationToken>& A, const std::vector<OperationToken>& B) const {
					if (A.size() != B.size())
						return false;
					auto ai = A.cbegin();
					auto bi = B.cbegin();
					while (ai != A.cend()) {
						if (ai->value != bi->value || ai->type != bi->type)
							return false;
						++ai;
						++bi;
					}
					return true;
				}
			} EqualOperation;

			typedef struct _ExprData {
				ExprType type;
				s32 value;
				std::unordered_map<size_t, s32> coeff;
				std::vector<OperationToken> operations;
			} ExprData;

			enum class LinearKind {
				NONE,
				ADD_LIKE,
				SUB_LIKE,
				SCALE_LIKE,
				NEG_LIKE
			};

			std::unordered_map<size_t, s32> addCoeff(const std::unordered_map<size_t, s32>& a, const std::unordered_map<size_t, s32>& b) {
				std::unordered_map<size_t, s32> result = a;
				for (auto it = b.cbegin(); it != b.cend(); ++it) {
					s32 v = result[it->first] + it->second;
					if (v == 0) result.erase(it->first);
					else result[it->first] = v;
				}
				return result;
			}
			
			std::unordered_map<size_t, s32> subCoeff(const std::unordered_map<size_t, s32>& a, const std::unordered_map<size_t, s32>& b) {
				std::unordered_map<size_t, s32> result = a;
				for (auto it = b.cbegin(); it != b.cend(); ++it) {
					s32 v = result[it->first] - it->second;
					if (v == 0) result.erase(it->first);
					else result[it->first] = v;
				}
				return result;
			}

			std::unordered_map<size_t, s32> scaleCoeff(const std::unordered_map<size_t, s32>& a, s32 factor) {
				std::unordered_map<size_t, s32> result;
				for (auto it = a.cbegin(); it != a.cend(); ++it) {
					s32 v = it->second * factor;
					if (v != 0) result[it->first] = v;
				}
				return result;
			}

			typedef struct _InstValue {
				u32 data;
				std::vector<ReplaceData> replace;

				_InstValue(void) : data(0) {}
			} InstValue;

			typedef struct _DataValue {
				u32 length;
				u32 data;
				std::vector<ReplaceData> replace;
				
				_DataValue(void) : length(0), data(0) {}
			} DataValue;

			typedef struct _EvalData {
				std::string fileName; // set before evaluation; read by format::formatData() (via mangleLabel) when building the serialized Format, not used during evaluation itself -- idenifierValue/expressionValue stay keyed by the raw label name throughout evaluation

				std::unordered_map<std::string, ExprData> idenifierValue;
				std::unordered_map<AST*, ExprData> expressionValue;
				std::unordered_map<AST*, InstValue> instructionValue;
				std::unordered_map<AST*, DataValue> dataValue;

				std::vector<AST*> sectioningArr;
				std::unordered_map<AST*, size_t> sectionIdMap;
				std::vector<u32> sectionByte;

				std::vector<Section> sections;
				std::vector<std::vector<OperationToken>> operationVector;
				std::vector<u32> compressionIndex;
			} EvalData;

			// Labels are mangled with the (output-binary-based) file name so that identically
			// named labels defined in different object files don't collide once the linker merges
			// their symbol tables (see linker::evaluate::functions::mapExpr's cross-file lookup).
			// '#' can't appear in a source identifier, so this can never collide with a real name.
			// Empty fileName leaves the label unmangled (e.g. for ad-hoc/debug evaluation). Applied
			// only in format::formatData(), not during evaluation -- calculateConst's identifier
			// lookups stay on the raw label name, since idenifierValue is keyed by it directly.
			inline std::string mangleLabel(const std::string& fileName, const std::string& label) {
				return fileName.empty() ? label : (fileName + "#" + label);
			}

			int hex2int(std::string text) {
				bool neg = false;
				int ans = 0;

				auto it = text.begin();
				if (*it == '-') {
					neg = true;
					++it;
				}
				if (it != text.end()) ++it;
				if (it != text.end()) ++it;

				for (it; it != text.end(); ++it) {
					ans *= 16;
					if ('0' <= *it && *it <= '9')
						ans += *it - '0';
					else if ('a' <= *it && *it <= 'f')
						ans += *it - 'a' + 10;
					else
						ans += *it - 'A' + 10;
				}
				if (neg)
					ans = -ans;
				return ans;
			}

			int dec2int(std::string text) {
				bool neg = false;
				int ans = 0;

				auto it = text.begin();
				if (*it == '-') {
					neg = true;
					++it;
				}
				if (it != text.end() && *it == '0') ++it;
				if (it != text.end() && *it == 'd') ++it;

				for (it; it != text.end(); ++it) {
					ans *= 10;
					if ('0' <= *it && *it <= '9')
						ans += *it - '0';
				}
				if (neg)
					ans = -ans;
				return ans;
			}

			int oct2int(std::string text) {
				bool neg = false;
				int ans = 0;

				auto it = text.begin();
				if (*it == '-') {
					neg = true;
					++it;
				}
				if (it != text.end()) ++it;
				if (it != text.end()) ++it;

				for (it; it != text.end(); ++it) {
					ans *= 8;
					if ('0' <= *it && *it <= '7')
						ans += *it - '0';
				}
				if (neg)
					ans = -ans;
				return ans;
			}

			int bin2int(std::string text) {
				bool neg = false;
				int ans = 0;

				auto it = text.begin();
				if (*it == '-') {
					neg = true;
					++it;
				}
				if (it != text.end()) ++it;
				if (it != text.end()) ++it;

				for (it; it != text.end(); ++it) {
					ans *= 2;
					if ('0' <= *it && *it <= '1')
						ans += *it - '0';
				}
				if (neg)
					ans = -ans;
				return ans;
			}

			int char2int(std::string text) {
				int ans = 0;

				auto it = text.begin();
				if (it != text.end()) ++it;

				if (*it == '\\') {
					++it;

					switch (*it) {
					case 'n': ans = '\n'; break;
					case 't': ans = '\t'; break;
					case '\\': ans = '\\'; break;
					case '\'': ans = '\''; break;
					case '\"': ans = '\"'; break;
					default:
						for (it; *it != '\''; ++it) {
							ans *= 10;
							ans += (*it - '0');
						}
					}
				}
				else {
					ans = *it;
				}

				return ans;
			}

			int pow(int base, u32 power) {
				int ans = 1;
				for (u32 i = 0; i < power; i++)
					ans *= base;
				return ans;
			}

			std::map<TokenType, u32> instructionBase = {
				{ TokenType::add, 0x0 },
				{ TokenType::addc, 0x1 },
				{ TokenType::addi, 0x2},
				{ TokenType::sub, 0x3 },
				{ TokenType::subc, 0x4 },
				{ TokenType::subi, 0x5 },
				{ TokenType::bxr, 0x6 },
				{ TokenType::bxri, 0x7 },
				{ TokenType::bor, 0x8 },
				{ TokenType::bori, 0x9 },
				{ TokenType::bnd, 0xA },
				{ TokenType::bndi, 0xB },
				{ TokenType::rol, 0xC },
				{ TokenType::roli, 0xD },
				{ TokenType::ror, 0xE },
				{ TokenType::rori, 0xF },
				{ TokenType::shiftl, 0x10 },
				{ TokenType::shiftlc, 0x11 },
				{ TokenType::shiftli, 0x12 },
				{ TokenType::shiftr, 0x13 },
				{ TokenType::shiftrc, 0x14 },
				{ TokenType::shiftri, 0x15 },
				{ TokenType::cmp, 0x16 },
				{ TokenType::cmpi, 0x17 },
				{ TokenType::test, 0x18 },
				{ TokenType::testi, 0x19 },

				{ TokenType::set, 0x1A },
				{ TokenType::sets, 0x1B },
				{ TokenType::mov, 0x1C },

				{ TokenType::block, 0x1D },

				{ TokenType::pop, 0x1E },
				{ TokenType::push, 0x1F },

				{ TokenType::ld, 0x20 },
				{ TokenType::st, 0x40 },

				{ TokenType::jmp, 0x60 },
				{ TokenType::ijmp, 0x68 },
				{ TokenType::call, 0x70 },
				{ TokenType::ret, 0x71 },
				{ TokenType::nop, 0x72 },
				{ TokenType::brk, 0x73 },
				{ TokenType::wait, 0x74 },
				{ TokenType::halt, 0x75 },

				{ TokenType::syscall, 0x76 },
				{ TokenType::iret, 0x77 },
				{ TokenType::intervec, 0x78 },
				{ TokenType::interpri, 0x79 },

				{ TokenType::ldexp, 0x7A },
				{ TokenType::stexp, 0x7B },

				{ TokenType::pctl, 0x7C },
				{ TokenType::getabs, 0x7E },
				{ TokenType::flush, 0x7F },
				{ TokenType::inval, 0x80 }
			};

			std::map<TokenType, u32> regExBase = {
				{ TokenType::cpuid, 0x0 },
				{ TokenType::panicvec, 0x1 },
				{ TokenType::istack, 0x2 },
				{ TokenType::intlevel, 0x3 },
				{ TokenType::pagebase, 0x4 }
			};

			std::map<TokenType, u32> regBase = {
				{ TokenType::reg, 0x0 },
				{ TokenType::gen, 0x0 },
				{ TokenType::sbp, 0x19 },
				{ TokenType::zero, 0x1A },
				{ TokenType::one, 0x1B },
				{ TokenType::full, 0x1C },
				{ TokenType::pc, 0x1D },
				{ TokenType::stack, 0x1E },
				{ TokenType::flag, 0x1F },

				{ TokenType::_32B_, 0 },
				{ TokenType::_16L_, 1 },
				{ TokenType::_8L_, 2 },
				{ TokenType::_8H_, 3 },
				{ TokenType::_S16H_, 4 },
				{ TokenType::_S16L_, 5 },
				{ TokenType::_S8L_, 6 },
				{ TokenType::_S8H_, 7 }
			};

			std::map<TokenType, u32> flagBase = {
				{ TokenType::zero, 0 },
				{ TokenType::neg, 1 },
				{ TokenType::pos, 2 },
				{ TokenType::carry, 3 },
				{ TokenType::carry4, 4 },
				{ TokenType::overflow, 5 },
				{ TokenType::one, 6 },
				{ TokenType::gen, 7 },
			};

			std::map<TokenType, u32> ctlBase = {
				{ TokenType::disable, 0 },
				{ TokenType::enable, 1 }
			};

			std::map<TokenType, u32> cacheBase = {
				{ TokenType::L1I, 0x1 },
				{ TokenType::L1D, 0x2 },
				{ TokenType::TLB, 0x4 },
				{ TokenType::L2, 0x8 },
				{ TokenType::ALL, 0x80 }
			};

			bool calculateSection(AST* cur, void* vpData) {
				EvalData* data = (EvalData*)vpData;

				SectionType type = SectionType::OUTER;
				switch (cur->type) {
					case (u64)ASTNodeType::text_section + (u64)TokenType::__end: type = SectionType::TEXT; goto process_section;
					case (u64)ASTNodeType::data_section + (u64)TokenType::__end: type = SectionType::DATA; goto process_section;
					case (u64)ASTNodeType::bss_section + (u64)TokenType::__end: type = SectionType::BSS; goto process_section;
					case (u64)ASTNodeType::outer_section + (u64)TokenType::__end: type = SectionType::OUTER; goto process_section;
						process_section: {
							size_t index = data->sections.size();
							for (auto it = data->sectioningArr.cbegin(); it != data->sectioningArr.cend(); ++it)
								data->sectionIdMap.insert({*it, index});

							Section nSection;
							nSection.type = type;
							nSection.name = (!cur->child.empty() && cur->child[0]->type == (u64)TokenType::identifier) ? cur->child[0]->text : "";
							data->sections.push_back(nSection);
							data->sectionByte.push_back(0);
							break;
						}

					case (u64)ASTNodeType::instruction_N + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_R + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_RI + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_RR + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_II + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_RII + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_IRI + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_RVRI + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_FVRI + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_IXP + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_RXR + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_CTL + (u64)TokenType::__end:
					case (u64)ASTNodeType::instruction_CACHE + (u64)TokenType::__end:
					case (u64)ASTNodeType::label + (u64)TokenType::__end:
						data->sectioningArr.push_back(cur);
						break;
				}

				return true;
			}

			ExprData combineBinary(const ExprData& a, const ExprData& b, OperationType opType, const std::string& opText, s32(*constFn)(s32, s32), LinearKind kind) {
				ExprData r;
				r.operations = a.operations;
				r.operations.insert(r.operations.end(), b.operations.begin(), b.operations.end());
				r.operations.push_back({ opType, opText });

				if (a.type == ExprType::CONSTANT && b.type == ExprType::CONSTANT) {
					r.type = ExprType::CONSTANT;
					r.value = constFn(a.value, b.value);
					return r;
				}

				bool linearOk = false;
				if (kind == LinearKind::ADD_LIKE) {
					if (a.type == ExprType::LINEAR && b.type == ExprType::CONSTANT) {
						r.coeff = a.coeff; 
						r.value = a.value + b.value; 
						linearOk = true;
					}
					else if (a.type == ExprType::CONSTANT && b.type == ExprType::LINEAR) {
						r.coeff = b.coeff; 
						r.value = a.value + b.value; 
						linearOk = true;
					}
					else if (a.type == ExprType::LINEAR && b.type == ExprType::LINEAR) {
						r.coeff = addCoeff(a.coeff, b.coeff); 
						r.value = a.value + b.value; 
						linearOk = true;
					}
				}
				else if (kind == LinearKind::SUB_LIKE) {
					if (a.type == ExprType::LINEAR && b.type == ExprType::CONSTANT) {
						r.coeff = a.coeff; 
						r.value = a.value - b.value; 
						linearOk = true;
					}
					else if (a.type == ExprType::CONSTANT && b.type == ExprType::LINEAR) {
						r.coeff = scaleCoeff(b.coeff, -1);
						r.value = a.value - b.value;
						linearOk = true;
					}
					else if (a.type == ExprType::LINEAR && b.type == ExprType::LINEAR) {
						r.coeff = subCoeff(a.coeff, b.coeff); 
						r.value = a.value - b.value; 
						linearOk = true;
					}
				}
				else if (kind == LinearKind::SCALE_LIKE) {
					if (a.type == ExprType::LINEAR && b.type == ExprType::CONSTANT) {
						r.coeff = scaleCoeff(a.coeff, b.value); 
						r.value = a.value * b.value; 
						linearOk = true;
					}
					else if (a.type == ExprType::CONSTANT && b.type == ExprType::LINEAR) {
						r.coeff = scaleCoeff(b.coeff, a.value); 
						r.value = a.value * b.value; 
						linearOk = true;
					}
				}

				if (linearOk)
					r.type = r.coeff.empty() ? ExprType::CONSTANT : ExprType::LINEAR;
				else {
					r.type = ExprType::COMPLEX;
					r.value = 0;
				}
				return r;
			}

			ExprData combineUnary(const ExprData& a, OperationType opType, const std::string& opText, s32(*constFn)(s32), LinearKind kind) {
				ExprData r;
				r.operations = a.operations;
				r.operations.push_back({ opType, opText });

				if (a.type == ExprType::CONSTANT) {
					r.type = ExprType::CONSTANT;
					r.value = constFn(a.value);
					return r;
				}

				bool linearOk = false;
				if (kind == LinearKind::NEG_LIKE) {
					if (a.type == ExprType::LINEAR) {
						r.coeff = scaleCoeff(a.coeff, -1);
						r.value = -a.value;
						linearOk = true;
					}
				}

				if (linearOk)
					r.type = ExprType::LINEAR;
				else {
					r.type = ExprType::COMPLEX;
					r.value = 0;
				}
				return r;
			}

			bool calculateConst(AST* cur, void* vpData) {
				EvalData* data = (EvalData*)vpData;

				bool child1exp = (cur->child.size() >= 1) ? data->expressionValue.find(cur->child[0]) != data->expressionValue.end() : false;
				bool child2exp = (cur->child.size() >= 2) ? data->expressionValue.find(cur->child[1]) != data->expressionValue.end() : false;
				bool child3exp = (cur->child.size() >= 3) ? data->expressionValue.find(cur->child[2]) != data->expressionValue.end() : false;
				
				OperationType operationType;
				std::string operationText;
				s32(*function)(s32, s32);
				s32(*unaryFunction)(s32);
				LinearKind linearKind;				

				switch (cur->type) {
				case (u64)TokenType::verticalbar: operationType = OperationType::BOR; operationText = "|"; function = [](s32 a, s32 b) { return a | b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::caret: operationType = OperationType::BXR; operationText = "^"; function = [](s32 a, s32 b) { return a ^ b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::ampersend: operationType = OperationType::BND; operationText = "&"; function = [](s32 a, s32 b) { return a & b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::leftshift: operationType = OperationType::SHIFTL; operationText = "<<"; function = [](s32 a, s32 b) { return a << b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::rightshift: operationType = OperationType::SHIFTR; operationText = ">>"; function = [](s32 a, s32 b) { return a >> b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::plus:
					if (cur->child.size() == 1) {
						if (child1exp) data->expressionValue[cur] = data->expressionValue[cur->child[0]];
						else return false;
						break;
					}
					operationType = OperationType::ADD; operationText = "+"; function = [](s32 a, s32 b) { return a + b; }; linearKind = LinearKind::ADD_LIKE; goto binaryOp;
				case (u64)TokenType::minus:
					if (cur->child.size() == 1) { operationType = OperationType::NEG; operationText = "-"; unaryFunction = [](s32 a) { return -a; }; linearKind = LinearKind::NEG_LIKE; goto unaryOp; }
					operationType = OperationType::SUB; operationText = "-"; function = [](s32 a, s32 b) { return a - b; }; linearKind = LinearKind::SUB_LIKE; goto binaryOp;
				case (u64)TokenType::star: operationType = OperationType::MULT; operationText = "*"; function = [](s32 a, s32 b) { return a * b; }; linearKind = LinearKind::SCALE_LIKE; goto binaryOp;
				case (u64)TokenType::slash: operationType = OperationType::DIV; operationText = "/"; function = [](s32 a, s32 b) { return a / b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::percent: operationType = OperationType::MOD; operationText = "%"; function = [](s32 a, s32 b) { return a % b; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::dstar: operationType = OperationType::POW; operationText = "**"; function = [](s32 a, s32 b) -> s32 { return pow(a, (u32)b); }; linearKind = LinearKind::NONE; goto binaryOp;

				case (u64)TokenType::less: operationType = OperationType::LESS; operationText = "<"; function = [](s32 a, s32 b) -> s32 { return a < b ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::greater: operationType = OperationType::GREATER; operationText = ">"; function = [](s32 a, s32 b) -> s32 { return a > b ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::lessequal: operationType = OperationType::LESSEQUAL; operationText = "<="; function = [](s32 a, s32 b) -> s32 { return a <= b ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::greaterequal: operationType = OperationType::GREATEQUAL; operationText = ">="; function = [](s32 a, s32 b) -> s32 { return a >= b ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::equal: operationType = OperationType::EQUAL; operationText = "=="; function = [](s32 a, s32 b) -> s32 { return a == b ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::notequal: operationType = OperationType::NOTEQUAL; operationText = "!="; function = [](s32 a, s32 b) -> s32 { return a != b ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;

				case (u64)TokenType::logampersend: operationType = OperationType::LAND; operationText = "&&"; function = [](s32 a, s32 b) -> s32 { return (a != 0 && b != 0) ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;
				case (u64)TokenType::logverticalbar: operationType = OperationType::LOR; operationText = "||"; function = [](s32 a, s32 b) -> s32 { return (a != 0 || b != 0) ? 1 : 0; }; linearKind = LinearKind::NONE; goto binaryOp;

				binaryOp:
					if (child1exp && child2exp)
						data->expressionValue[cur] = combineBinary(data->expressionValue[cur->child[0]], data->expressionValue[cur->child[1]], operationType, operationText, function, linearKind);
					else
						return false;
					break;

				case (u64)TokenType::tilde: operationType = OperationType::BNT; operationText = "~"; unaryFunction = [](s32 a) { return ~a; }; linearKind = LinearKind::NONE; goto unaryOp;
				case (u64)TokenType::lognot: operationType = OperationType::LNOT; operationText = "!"; unaryFunction = [](s32 a) -> s32 { return a == 0 ? 1 : 0; }; linearKind = LinearKind::NONE; goto unaryOp;

				unaryOp:
					if (child1exp)
						data->expressionValue[cur] = combineUnary(data->expressionValue[cur->child[0]], operationType, operationText, unaryFunction, linearKind);
					else
						return false;
					break;

				case (u64)TokenType::selector: {
					if (child1exp && child2exp && child3exp) {
						ExprData& condV = data->expressionValue[cur->child[0]];
						ExprData& thenV = data->expressionValue[cur->child[1]];
						ExprData& elseV = data->expressionValue[cur->child[2]];

						if (condV.type == ExprType::CONSTANT) {
							data->expressionValue[cur] = (condV.value != 0) ? thenV : elseV;
						}
						else {
							ExprData r;
							r.type = ExprType::COMPLEX;
							r.value = 0;
							r.operations = condV.operations;
							r.operations.insert(r.operations.end(), thenV.operations.begin(), thenV.operations.end());
							r.operations.insert(r.operations.end(), elseV.operations.begin(), elseV.operations.end());
							r.operations.push_back({ OperationType::SELECT, "?:" });
							data->expressionValue[cur] = r;
						}
					}
					break;
				}

				case (u64)TokenType::hexnum: { s32 v = hex2int(cur->text); data->expressionValue[cur] = { ExprType::CONSTANT, v, {}, { { OperationType::CONSTANT, cur->text, v } } }; break; }
				case (u64)TokenType::decnum: { s32 v = dec2int(cur->text); data->expressionValue[cur] = { ExprType::CONSTANT, v, {}, { { OperationType::CONSTANT, cur->text, v } } }; break; }
				case (u64)TokenType::octnum: { s32 v = oct2int(cur->text); data->expressionValue[cur] = { ExprType::CONSTANT, v, {}, { { OperationType::CONSTANT, cur->text, v } } }; break; }
				case (u64)TokenType::binnum: { s32 v = bin2int(cur->text); data->expressionValue[cur] = { ExprType::CONSTANT, v, {}, { { OperationType::CONSTANT, cur->text, v } } }; break; }
				case (u64)TokenType::character: { s32 v = char2int(cur->text); data->expressionValue[cur] = { ExprType::CONSTANT, v, {}, { { OperationType::CONSTANT, cur->text, v } } }; break; }

				case (u64)TokenType::identifier: {
					auto find = data->idenifierValue.find(cur->text);
					if (find != data->idenifierValue.end())
						data->expressionValue[cur] = find->second;
					break;
				}

				case (u64)ASTNodeType::index + (u64)TokenType::__end:
					if (child1exp) {
						if (data->expressionValue[cur->child[0]].value < 0)
							return false;
						data->expressionValue[cur] = data->expressionValue[cur->child[0]];
					}
					break;

				case (u64)ASTNodeType::instruction_N + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_R + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_RI + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_RR + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_II + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_RII + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_IRI + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_RVRI + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_FVRI + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_IXP + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_RXR + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_CTL + (u64)TokenType::__end:
				case (u64)ASTNodeType::instruction_CACHE + (u64)TokenType::__end: {
					size_t section_id = data->sectionIdMap[cur];
					data->sectionByte[section_id] += 4;
					break;
				}

				case (u64)ASTNodeType::label + (u64)TokenType::__end: {
					size_t section_id = data->sectionIdMap[cur];

					ExprData nData;
					nData.type = ExprType::LINEAR;
					nData.value = data->sectionByte[section_id];
					nData.coeff[section_id] = 1;
					nData.operations.push_back({ OperationType::IDENTIFIER, cur->text });

					data->idenifierValue[cur->text] = nData;
					break;
				}

				case (u64)ASTNodeType::allocate + (u64)TokenType::__end: {
					size_t section_id = data->sectionIdMap[cur];

					auto find = data->expressionValue.find(cur->child[0]);
					if (find == data->expressionValue.end())
						return false;
					if (find->second.type != ExprType::CONSTANT)
						return false;

					data->sectionByte[section_id] += find->second.value;
					break;
				}

				case (u64)ASTNodeType::allocate_zero + (u64)TokenType::__end: {
					size_t section_id = data->sectionIdMap[cur];

					auto find = data->expressionValue.find(cur->child[0]);
					if (find == data->expressionValue.end())
						return false;
					if (find->second.type != ExprType::CONSTANT)
						return false;

					data->sectionByte[section_id] += find->second.value;
					break;
				}
				}

				return true;
			}

			void push32(std::vector<u8>& list, u32 inst) {
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

			void pushByteN(std::vector<u8>& list, u32 data, int size) {
				union {
					u32 u32data;
					u8 u8data[4];
				} conv;
				conv.u32data = data;

				for (size; size > 4; size--) {
					list.push_back(conv.u8data[(size-1)%4]);
				}

				switch (size) {
				case 4: list.push_back(conv.u8data[3]);
				case 3: list.push_back(conv.u8data[2]);
				case 2: list.push_back(conv.u8data[1]);
				case 1: list.push_back(conv.u8data[0]);
				}
			}

			void appendReplace(std::vector<ReplaceData>& dest, const std::vector<ReplaceData>& src, u32 shift) {
				for (ReplaceData rep : src) {
					rep.offset += shift;
					dest.push_back(rep);
				}
			}

			void registerReplace(EvalData* data, u32 sectionId, const std::vector<ReplaceData>& replace, u32 byte) {
				u32 basePos = (u32)data->sections[sectionId].data.size() + byte - 1;
				for (auto it = replace.cbegin(); it != replace.cend(); ++it)
					data->sections[sectionId].operationMap.insert({ basePos-it->offset/8, ReplaceData{it->index, it->size, it->offset%8}});
			}

			bool evaluateCode(AST* cur, void* vpData) {
				EvalData* data = (EvalData*)vpData;
				u32 sectionId = data->sectionIdMap[cur];

				switch (cur->type) {
				case (u64)ASTNodeType::allocate + (u64)TokenType::__end: {
					if (data->expressionValue.find(cur->child[1]) == data->expressionValue.end())
						return false;

					ExprData& expr = data->expressionValue[cur->child[1]];
					if (expr.type == ExprType::CONSTANT)
						data->dataValue[cur].data = expr.value;
					else {
						ReplaceData rep = {};
						rep.index = data->operationVector.size();
						rep.offset = 0;
						rep.size = 32;
						data->dataValue[cur].data = 0;
						data->dataValue[cur].replace.push_back(rep);
						data->operationVector.push_back(expr.operations);
					}

					registerReplace(data, sectionId, data->dataValue[cur].replace, data->dataValue[cur].length/8);
					pushByteN(data->sections[sectionId].data, data->dataValue[cur].data, data->dataValue[cur].length);
					break;
				}

				case (u64)ASTNodeType::instruction_N + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_R + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_RI + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_RR + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data << 8);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 8);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_II + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_RII + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data << 8) + (data->instructionValue[cur->child[3]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 8);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[3]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_IRI + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data << 8) + (data->instructionValue[cur->child[3]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 8);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[3]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_RVRI + (u64)TokenType::__end:
					data->instructionValue[cur].data = ((instructionBase[(TokenType)cur->child[0]->type] + data->instructionValue[cur->child[1]].data) << 24) + (data->instructionValue[cur->child[2]].data << 16) + (data->instructionValue[cur->child[3]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 24);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[3]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_FVRI + (u64)TokenType::__end:
					data->instructionValue[cur].data = ((instructionBase[(TokenType)cur->child[0]->type] + flagBase[(TokenType)cur->child[1]->child[0]->type]) << 24) + (data->instructionValue[cur->child[2]].data << 16) + (data->instructionValue[cur->child[3]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[3]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_IXP + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 8) + (data->instructionValue[cur->child[2]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 8);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_RXR + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 0);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_CTL + (u64)TokenType::__end:
					data->instructionValue[cur].data = ((instructionBase[(TokenType)cur->child[0]->type] + data->instructionValue[cur->child[1]].data) << 24);
					goto pushInstruction;
				case (u64)ASTNodeType::instruction_CACHE + (u64)TokenType::__end:
					data->instructionValue[cur].data = (instructionBase[(TokenType)cur->child[0]->type] << 24) + (data->instructionValue[cur->child[1]].data << 16) + (data->instructionValue[cur->child[2]].data << 8) + (data->instructionValue[cur->child[3]].data);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 16);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[2]].replace, 8);

				pushInstruction:
					registerReplace(data, sectionId, data->instructionValue[cur].replace, 4);
					push32(data->sections[sectionId].data, data->instructionValue[cur].data);
					break;

				case (u64)ASTNodeType::reg_ex + (u64)TokenType::__end:
					data->instructionValue[cur].data = regExBase[(TokenType)cur->child[0]->type];
					break;
				case (u64)ASTNodeType::reg + (u64)TokenType::__end:
					data->instructionValue[cur].data = (data->instructionValue[cur->child[0]].data << 3) + data->instructionValue[cur->child[1]].data;
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[0]].replace, 3);
					appendReplace(data->instructionValue[cur].replace, data->instructionValue[cur->child[1]].replace, 0);
					break;
				case (u64)ASTNodeType::regid + (u64)TokenType::__end:
					data->instructionValue[cur].data = regBase[(TokenType)cur->child[0]->type];
					break;
				case (u64)ASTNodeType::reg_index + (u64)TokenType::__end: {
					if (data->expressionValue.find(cur->child[1]) == data->expressionValue.end())
						return false;

					ExprData& expr = data->expressionValue[cur->child[1]];
					if (expr.type == ExprType::CONSTANT)
						data->instructionValue[cur].data = regBase[(TokenType)cur->child[0]->type] + expr.value;
					else {
						ReplaceData rep = {};
						rep.index = data->operationVector.size();
						rep.offset = 0;
						rep.size = 5;
						data->instructionValue[cur].data = regBase[(TokenType)cur->child[0]->type];
						data->instructionValue[cur].replace.push_back(rep);
						data->operationVector.push_back(expr.operations);
					}
					break;
				}
				case (u64)ASTNodeType::regmode + (u64)TokenType::__end:
					data->instructionValue[cur].data = regBase[(TokenType)cur->child[0]->type];
					break;

				case (u64)ASTNodeType::ctlid + (u64)TokenType::__end:
					data->instructionValue[cur].data = ctlBase[(TokenType)cur->child[0]->type];
					break;
				case (u64)ASTNodeType::cacheid + (u64)TokenType::__end:
					data->instructionValue[cur].data = cacheBase[(TokenType)cur->child[0]->type];
					break;

				case (u64)ASTNodeType::immidate16 + (u64)TokenType::__end: {
					if (data->expressionValue.find(cur->child[0]) == data->expressionValue.end())
						return false;

					ExprData& expr = data->expressionValue[cur->child[0]];
					if (expr.type == ExprType::CONSTANT)
						data->instructionValue[cur].data = expr.value & 0xFFFF;
					else {
						ReplaceData rep = {};
						rep.index = data->operationVector.size();
						rep.offset = 0;
						rep.size = 16;
						data->instructionValue[cur].data = 0;
						data->instructionValue[cur].replace.push_back(rep);
						data->operationVector.push_back(expr.operations);
					}
					break;
				}
				case (u64)ASTNodeType::immidate8 + (u64)TokenType::__end: {
					if (data->expressionValue.find(cur->child[0]) == data->expressionValue.end())
						return false;

					ExprData& expr = data->expressionValue[cur->child[0]];
					if (expr.type == ExprType::CONSTANT)
						data->instructionValue[cur].data = expr.value & 0xFF;
					else {
						ReplaceData rep = {};
						rep.index = data->operationVector.size();
						rep.offset = 0;
						rep.size = 8;
						data->instructionValue[cur].data = 0;
						data->instructionValue[cur].replace.push_back(rep);
						data->operationVector.push_back(expr.operations);
					}
					break;
				}
				}

				return true;
			}

			inline bool collapseExpr(EvalData* data) {
				std::unordered_map<std::vector<OperationToken>, size_t, HashOperation, EqualOperation> checked;
				u32 compressIndex = 0;
				for (size_t i = 0; i < data->operationVector.size(); i++) {
					auto& it = data->operationVector[i];
					auto found = checked.find(it);
					if (found == checked.end()) {
						checked.insert({it, i});
						data->compressionIndex.push_back(compressIndex++);
						continue;
					}
					data->compressionIndex.push_back((u32)-1);

					size_t idx = found->second;
					for (auto si = data->sections.begin(); si != data->sections.end(); ++si) {
						for (auto ti = si->operationMap.begin(); ti != si->operationMap.end(); ++ti) {
							if (ti->second.index == i)
								ti->second.index = idx;
						}
					}
				}
				return true;
			}
		}

		Evaluator evaluator;

		inline void setTree(AST* tree) {
			evaluator.setTree(tree);
		}

		inline bool evaluate(functions::EvalData& data) {
			if (!evaluator.evaluate(functions::calculateSection, &data)) return false;
			if (!evaluator.evaluate(functions::calculateConst, &data)) return false;
			if (!evaluator.evaluate(functions::evaluateCode, &data)) return false;
			functions::collapseExpr(&data);

			return true;
		}
	}

	namespace format {
		using u32 = unsigned __int32;
		using u8 = unsigned __int8;

		using EvalData = evaluator::functions::EvalData;
		using Format = format_data::Format;
		using Section = format_data::Section;
		using Operation = format_data::Operation;
		using Mapper = format_data::Mapper;
		using Identifier = format_data::Identifier;

		Format format;

		inline bool formatData(const EvalData& data) {
			format.clear();

			std::unordered_map<std::string, u32> usedIdentifierIndex;

			for (size_t i = 0; i < data.operationVector.size(); i++) {
				if (data.compressionIndex[i] == (u32)-1)
					continue;

				std::vector<Operation> copy;
				for (auto si = data.operationVector[i].cbegin(); si != data.operationVector[i].cend(); ++si) {
					Operation nOper = {};
					nOper.type = (u32)si->type;

					if (si->type == evaluator::functions::OperationType::IDENTIFIER) {
						std::string mangled = evaluator::functions::mangleLabel(data.fileName, si->text);
						auto find = usedIdentifierIndex.find(mangled);
						if (find == usedIdentifierIndex.end()) {
							u32 idx = (u32)format.usedIdentifier.size();
							usedIdentifierIndex[mangled] = idx;
							format.usedIdentifier.push_back(mangled);
							nOper.value = idx;
						}
						else
							nOper.value = find->second;
					}
					else
						nOper.value = si->value;

					copy.push_back(nOper);
				}

				Operation eox = {};
				eox.type = (u32)evaluator::functions::OperationType::EOX;
				eox.value = 0;
				copy.push_back(eox);

				format.expressions.push_back(copy);
			}

			for (auto it = data.idenifierValue.cbegin(); it != data.idenifierValue.cend(); ++it) {
				Identifier nId = {};
				nId.name = evaluator::functions::mangleLabel(data.fileName, it->first);
				if (it->second.coeff.empty()) {
					nId.section = "";
					nId.sectionIndex = (u32)-1;
				}
				else {
					size_t section_id = it->second.coeff.cbegin()->first;
					nId.section = data.sections[section_id].name;
					nId.sectionIndex = (u32)section_id;
				}
				nId.value = (u32)it->second.value;
				format.definedIdentifier.push_back(nId);
			}

			u32 base = 0;
			for (auto it = data.sections.cbegin(); it != data.sections.cend(); ++it) {
				base = (u32)format.binary.size();

				for (auto si = it->operationMap.cbegin(); si != it->operationMap.cend(); ++si) {
					Mapper nMap = {};
					nMap.sectionName = it->name;
					nMap.byteIndex = base + si->first;
					nMap.offset = si->second.offset;
					nMap.size = si->second.size;
					nMap.exprIndex = si->second.index;

					format.mapper.push_back(nMap);
				}

				Section nSec = {};
				nSec.name = it->name;
				nSec.type = (u32)it->type;
				nSec.start = base;
				nSec.end = base + (u32)it->data.size();
				format.sections.push_back(nSec);
				format.binary.insert(format.binary.end(), it->data.begin(), it->data.end());
			}

			return true;
		}

		inline void getRaw(std::vector<u8>& out) {
			format.getRaw(out);
		}
	}

	const unsigned long long version = 400;

	namespace cache {
		using u64 = unsigned __int64;
		using u8 = unsigned __int8;

		using LexerData = lexer::LexerFactory::UpdateData;
		using ParserData = parser::ParserFactory::UpdateData;

		inline void pack(std::vector<u64>& dest, const std::vector<u8>& src) {
			int count = 0;
			u64 tmp = 0;
			for (auto it = src.cbegin(); it != src.cend(); ++it) {
				tmp = (tmp << 8) + *it;

				count++; count %= 8;
				if (!count) {
					dest.push_back(tmp);
					tmp = 0;
				}
			}
			if (count) {
				tmp = (tmp << (8 * (8 - count)));
				dest.push_back(tmp);
			}
		}

		inline void unpack(std::vector<u8>& dest, const std::vector<u64>& src) {
			for (auto it = src.cbegin(); it != src.cend(); ++it) {
				u64 tmp = *it;
				for (int i = 0; i < 8; i++) {
					dest.push_back((u8)(tmp >> 56));
					tmp <<= 8;
				}
			}
		}

		void compress(std::vector<u8>& out, const std::vector<u8>& in) {
			u8 byte = in.front();
			int count = -1;
			for (auto it = in.cbegin(); it != in.cend(); ++it) {
				if (byte != *it || count >= 255) {
					out.push_back(byte);
					out.push_back((u8)count);
					byte = *it;
					count = 0;
					continue;
				}
				count++;
			}
			out.push_back(byte);
			out.push_back((u8)count);
		}

		void decompress(std::vector<u8>& out, const std::vector<u8>& in) {
			for (auto it = in.cbegin(); it != in.cend(); ) {
				u8 byte = *it; ++it;
				u8 count = *it; ++it;
				for (int i = 0; i <= (int)count; i++)
					out.push_back(byte);
			}
		}

		void BinaryToLexer(LexerData& out, const std::vector<u64>& in, typename std::vector<u64>::const_iterator& it) {
			using GraphID = lexer_generator::graph::ID;

			u64 tableSize = *it; it++;
			u64 endDataSize = *it; it++;

			GraphID** table = new GraphID * [tableSize];
			for (size_t i = 0; i < tableSize; i++) {
				table[i] = new GraphID[128];
				for (size_t j = 0; j < 128; j++) {
					table[i][j] = (GraphID)*it;
					it++;
				}
			}

			std::vector<std::array<u64, 3>> endData;
			for (size_t i = 0; i < endDataSize; i++) {
				std::array<u64, 3> tmp;
				for (size_t j = 0; j < 3; j++) {
					tmp[j] = *it;
					it++;
				}
				endData.push_back(tmp);
			}

			out.table.size = tableSize;
			out.table.table = table;
			for (size_t i = 0; i < endDataSize; i++)
				out.endData.push_back({ (GraphID)endData[i][0], { (lexer::TokenType)endData[i][1], endData[i][2] } });
		}

		void LexerToBinary(std::vector<u64>& out, const LexerData& in) {
			u64 tableSize = in.table.size;
			u64 endDataSize = in.endData.size();

			std::vector<std::array<u64, 3>> endData;
			for (auto it = in.endData.cbegin(); it != in.endData.cend(); ++it)
				endData.push_back({ (u64)it->first, (u64)it->second.token, (u64)it->second.priority });

			out.push_back(tableSize);
			out.push_back(endDataSize);
			for (size_t i = 0; i < tableSize; i++) {
				for (size_t j = 0; j < 128; j++)
					out.push_back(in.table.table[i][j]);
			}
			for (auto it = endData.cbegin(); it != endData.cend(); ++it) {
				out.push_back((*it)[0]);
				out.push_back((*it)[1]);
				out.push_back((*it)[2]);
			}
		}

		void BinaryToParser(ParserData& out, const std::vector<u64>& in, typename std::vector<u64>::const_iterator& it) {
			u64 tableSize = *it; it++;
			u64 actionTableSize = *it; it++;
			u64 gotoTableSize = *it; it++;
			u64 vectorSize = *it; it++;
			std::vector<u64> vectorTailSize;
			for (size_t i = 0; i < vectorSize; i++) {
				vectorTailSize.push_back(*it);
				it++;
			}

			out.table.size = tableSize;
			out.table.actionSize = actionTableSize;
			out.table.gotoSize = gotoTableSize;

			out.table.actionTable = new parser_generator::convert::Action * [tableSize];
			for (size_t i = 0; i < tableSize; i++) {
				out.table.actionTable[i] = new parser_generator::convert::Action[actionTableSize];
				for (size_t j = 0; j < actionTableSize; j++) {
					out.table.actionTable[i][j].arg = *it; it++;
					out.table.actionTable[i][j].type = (parser_generator::convert::ActionType)*it; it++;
				}
			}

			out.table.gotoTable = new parser_generator::convert::Goto * [tableSize];
			for (size_t i = 0; i < tableSize; i++) {
				out.table.gotoTable[i] = new parser_generator::convert::Goto[gotoTableSize];
				for (size_t j = 0; j < gotoTableSize; j++) {
					out.table.gotoTable[i][j].state = *it; it++;
				}
			}

			for (size_t i = 0; i < tableSize; i++) {
				int count = 0;
				for (size_t j = 0; j < gotoTableSize; j++) {
					out.table.gotoTable[i][j].error = (*it >> (63 - count)) & 1;
					count++; count %= 64;
					if (!count)
						it++;
				}
				if (count) 
					it++;
			}

			using Element = parser_generator::convert::Element;

			for (size_t i = 0; i < vectorSize; i++) {
				Element head = (Element)*it; it++;
				std::vector<Element> tmp;
				for (size_t j = 0; j < vectorTailSize[i]; j++) {
					tmp.push_back((Element)*it); it++;
				}
				out.grammerVec.push_back({ head, tmp });
			}

			for (size_t i = 0; i < vectorSize; i++) {
				Element head1 = (Element)*it; it++;
				int head2 = (int)*it; it++;
				std::vector<std::pair<bool, bool>> tmp;
				int count = 0;
				for (size_t j = 0; j < vectorTailSize[i]; j++) {
					bool first = (*it >> (63 - count)) & 1; count++;
					bool second = (*it >> (63 - count)) & 1; count++;
					tmp.push_back({ first, second });
					count %= 32;
					if (!count)
						it++;
				}
				if (count)
					it++;
				out.ASTActionVec.push_back({ {head1, head2}, tmp });
			}
		}

		void ParserToBinary(std::vector<u64>& out, const ParserData& in) {
			u64 tableSize = in.table.size;
			u64 actionTableSize = in.table.actionSize;
			u64 gotoTableSize = in.table.gotoSize;
			u64 vectorSize = in.grammerVec.size();
			std::vector<u64> vectorTailSize;
			for (auto it = in.grammerVec.cbegin(); it != in.grammerVec.cend(); ++it)
				vectorTailSize.push_back(it->second.size());

			std::vector<u64> grammerVec;
			std::vector<u64> ASTActionVec;

			for (auto it = in.grammerVec.cbegin(); it != in.grammerVec.cend(); ++it) {
				grammerVec.push_back(it->first);
				u64 tmp = 0;
				int count = 0;
				for (auto si = it->second.cbegin(); si != it->second.cend(); ++si)
					grammerVec.push_back(*si);
			}

			for (auto it = in.ASTActionVec.cbegin(); it != in.ASTActionVec.cend(); ++it) {
				ASTActionVec.push_back(it->first.first);
				ASTActionVec.push_back(it->first.second);
				u64 tmp = 0;
				int count = 0;
				for (auto si = it->second.cbegin(); si != it->second.cend(); ++si) {
					tmp = (tmp << 1) + (si->first ? 1 : 0);
					tmp = (tmp << 1) + (si->second ? 1 : 0);
					count++; count %= 32;
					if (!count) {
						ASTActionVec.push_back(tmp);
						tmp = 0;
					}
				}
				if (count) {
					tmp = tmp << (2 * (32 - count));
					ASTActionVec.push_back(tmp);
				}
			}

			out.push_back(tableSize);
			out.push_back(actionTableSize);
			out.push_back(gotoTableSize);
			out.push_back(vectorSize);
			for (size_t i = 0; i < vectorSize; i++)
				out.push_back(vectorTailSize[i]);

			for (size_t i = 0; i < tableSize; i++) {
				for (size_t j = 0; j < actionTableSize; j++) {
					out.push_back(in.table.actionTable[i][j].arg);
					out.push_back((u64)in.table.actionTable[i][j].type);
				}
			}

			for (size_t i = 0; i < tableSize; i++) {
				for (size_t j = 0; j < gotoTableSize; j++) {
					out.push_back(in.table.gotoTable[i][j].state);
				}
			}

			for (size_t i = 0; i < tableSize; i++) {
				u64 tmp = 0;
				int count = 0;
				for (size_t j = 0; j < gotoTableSize; j++) {
					tmp = (tmp << 1) + (in.table.gotoTable[i][j].error ? 1 : 0);
					count++; count %= 64;
					if (!count) {
						out.push_back(tmp);
						tmp = 0;
					}
				}
				if (count) {
					tmp = tmp << (64 - count);
					out.push_back(tmp);
				}
			}

			for (auto it = grammerVec.cbegin(); it != grammerVec.cend(); ++it)
				out.push_back(*it);
			for (auto it = ASTActionVec.cbegin(); it != ASTActionVec.cend(); ++it)
				out.push_back(*it);
		}

		void clearLexerData(LexerData& in) {
			if (in.table.table != NULL) {
				for (size_t i = 0; i < in.table.size; i++)
					in.table.table[i];
				delete[] in.table.table;
			}
			in.table.size = 0;
			in.table.table = NULL;

			in.endData.clear();
		}

		void clearParserData(ParserData& in) {
			if (in.table.actionTable != NULL) {
				for (size_t i = 0; i < in.table.size; i++)
					delete[] in.table.actionTable[i];
				delete[] in.table.actionTable;
			}
			if (in.table.gotoTable != NULL) {
				for (size_t i = 0; i < in.table.size; i++)
					delete[] in.table.gotoTable[i];
				delete[] in.table.gotoTable;
			}
			in.table.size = 0;
			in.table.actionSize = 0;
			in.table.gotoSize = 0;
			in.table.actionTable = NULL;
			in.table.gotoTable = NULL;

			in.ASTActionVec.clear();
			in.grammerVec.clear();
		}

		u64 BinaryToASM(ParserData& parseOut, LexerData& lexOut, const std::vector<u8>& in) {
			std::vector<u64> binary;
			pack(binary, in);
			auto it = binary.cbegin();
			u64 version = *it; it++;

			if (version != assembler::version)
				return version;

			BinaryToLexer(lexOut, binary, it);
			BinaryToParser(parseOut, binary, it);

			return version;
		}

		void ASMToBinary(std::vector<u8>& out, const ParserData& parseIn, const LexerData& lexIn) {
			std::vector<u64> binary;
			binary.push_back(version);
			LexerToBinary(binary, lexIn);
			ParserToBinary(binary, parseIn);
			unpack(out, binary);
		}
	}

	using u8 = unsigned __int8;
	using u32 = unsigned __int32;
	using u64 = unsigned __int64;

	void createAssembler(void) {
		std::vector<u8> buff;
		std::vector<u8> temp;
		if (iotools::readBinaryFile(iotools::directory + iotools::executable + "_table.bin", buff)) {
			if (buff.size() < 8)
				goto createDefault;

			cache::LexerData lexData;
			cache::ParserData parseData;
			temp.reserve(buff.size());
			cache::decompress(temp, buff);
			u64 fileVer = cache::BinaryToASM(parseData, lexData, temp);

			if (fileVer == assembler::version) {
				lexer::createLexer(lexData);
				parser::createParser(parseData);

				cache::clearLexerData(lexData);
				cache::clearParserData(parseData);
				return;
			}
		}
		
	createDefault:
		lexer::createLexer();
		parser::createParser();

		cache::LexerData lexData;
		cache::ParserData parseData;

		lexer::getFactoryData(lexData);
		parser::getFactoryData(parseData);
		buff.clear();
		temp.clear();

		cache::ASMToBinary(buff, parseData, lexData);
		temp.reserve(buff.size());
		cache::compress(temp, buff);
		iotools::writeBinaryFile(iotools::directory + iotools::executable + "_table.bin", temp);

		cache::clearLexerData(lexData);
		cache::clearParserData(parseData);
	}

	typedef struct _AssemblerDump {
		enum flag {
			none = 0,
			getToken = 1 << 0,
			getPreproc = 1 << 1,
			getParse = 1 << 2,
			getAST = 1 << 3,
			getEval = 1 << 4,
			getBin = 1 << 5
		};

		u32 flags;

		bool parseSuccess = false, preprocSuccess = false, evalSuccess = false;
		std::vector<lexer::Token> tokens;
		std::vector<preprocesser::Token> preprocTokens;
		std::string errorMessage;
		parser::Parser::ASTNode* pAST;
		std::vector<evaluator::functions::Section> sectionDump;
	} AssemblerDump;

	int assemble(const std::string& text, std::vector<u8>& binary, AssemblerDump& dump, const std::string& fileName = "") {
		using TokenType = lexer::TokenType;
		using Token = lexer::Token;
		using Parser = parser::Parser;

		std::vector<Token> tokens;
		lexer::lexer.lex(tokens, text);
		preprocesser::tagLine(tokens);
		std::vector<Token> tokenBackup = tokens;

		if (dump.flags & AssemblerDump::getToken) {
			dump.tokens = tokens;
		}

		preprocesser::functions::PreprocData preprocData;
		preprocesser::setTokens(tokens);
		bool validPreprocess = preprocesser::preprocess(preprocData);
		dump.preprocSuccess = validPreprocess;

		if (dump.flags & AssemblerDump::getPreproc || !validPreprocess) {
			dump.preprocTokens = tokens;
		}

		if (!validPreprocess) {
			return -1;
		}

		Parser::ASTNode* pAST;
		int errorLine = 0;
		bool validParse = parser::parser.parse(pAST, &errorLine, tokens);
		dump.parseSuccess = validParse;

		if (dump.flags & AssemblerDump::getAST || !validParse) {
			dump.pAST = pAST;
		}

		if (dump.flags & AssemblerDump::getEval || !validParse) {
			if (!dump.parseSuccess) {
				dump.errorMessage = "error at line ";
				dump.errorMessage += std::to_string(errorLine);
				dump.errorMessage += ": ";
				for (auto it = tokenBackup.cbegin(); it->line <= errorLine && it != tokenBackup.cend(); ++it) {
					if (it->line == errorLine) {
						if (it->type == lexer::TokenType::newline)
							break;
						dump.errorMessage += it->text;
					}
				}

				dump.errorMessage += "\nunexpected token: ";
				dump.errorMessage += pAST->child[0]->text;
			}
		}

		if (!validParse) {
			parser::parser.delAST(pAST);
			return -2;
		}

		evaluator::functions::EvalData data;
		data.fileName = fileName;
		evaluator::setTree(pAST);
		bool validEvaluate = evaluator::evaluate(data);
		dump.evalSuccess = validEvaluate;

		if (!validEvaluate) {
			parser::parser.delAST(pAST);
			return -3;
		}
		

		std::vector<u8> raw8;
		format::formatData(data);
		format::getRaw(raw8);

		if (dump.flags & AssemblerDump::getBin) {
			dump.sectionDump = data.sections;
		}

		parser::parser.delAST(pAST);
		binary = raw8;

		return 0;
	}
}

#endif