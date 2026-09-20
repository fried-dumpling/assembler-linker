# CLAUDE.md — src/assembler-linker/assembler

이 파일은 Claude Code가 이 디렉토리(`computer 32 (new)/src/assembler-linker/assembler`)에서 작업을 이어갈 때 빠르게 현재 상태를 파악하기 위한 참고 문서입니다. **이 프로젝트는 아직 미완성이며, 명시적으로 요청받기 전까지는 빌드/테스트를 수행하지 않습니다.**

## 디렉토리 구조 (2026-09-20 개편, 이후 사용자가 추가 개편)

원래 `src/assembler`였던 디렉토리를 `src/assembler-linker`로 이름을 바꾸고(Claude가 수행), 그 뒤 사용자가 직접 한 단계 더 정리해서 지금은 이런 모습:

```
src/assembler-linker/
  common/               — assembler/linker 공유 헤더
    format.hpp            오브젝트 파일 직렬화 포맷 (namespace format_data)
    iotool.hpp             파일 IO 유틸 (namespace iotools)
  assembler/            — 이 문서가 있는 디렉토리
    assembler.hpp / lexer.hpp / parser.hpp / evaluator.hpp / preprocesser.hpp
    main.cpp
    program/
    CLAUDE.md (이 파일)
  linker/               — 새로 추가된 링커 (사용자가 직접 작성 중, WIP)
    linker.hpp
    main.cpp
  build-asm.cmd          — 어셈블러 빌드 스크립트 (assembler\main.cpp → assembler.exe)
  build-lnk.cmd          — 링커 빌드 스크립트 (linker\main.cpp → linker.exe) — Claude가 build-asm.cmd 패턴 그대로 생성
```

`assembler.hpp`/`main.cpp`는 `#include "../common/format.hpp"`, `#include "../common/iotool.hpp"`로 참조 (사용자가 `common/`으로 옮기면서 직접 갱신함). `linker.hpp`/`linker/main.cpp`도 동일하게 `../common/`을 참조.

빌드는 `assembler-linker\build-asm.cmd`(어셈블러) / `assembler-linker\build-lnk.cmd`(링커) 각각 실행하면 됨 (MinGW `g++` 필요, PATH에 있어야 함). 두 스크립트는 동일한 패턴(스크립트 자기 위치 기준 상대경로, `where g++` 체크, 실패 시 exit code 1) — 서로 상대 스크립트를 주석으로 교차 참조함.

**참고**: 최초 `src/assembler` → `src/assembler-linker` 디렉토리 이름 변경(`mv`/rename) 자체는 이 세션 환경에서 "Device or resource busy"로 실패함(아마 파일 변경 감지용 워처가 옛 경로에 핸들을 잡고 있어서로 추정) — 그래서 새 트리를 만들고 파일을 개별적으로 옮기는 방식으로 우회했다. 그 결과 **빈 `src/assembler` 디렉토리가 그대로 남아있음** — 삭제도 같은 이유로 실패했으니, 나중에 편한 시점에 수동으로 지우면 됨(내용물 없음, 안전하게 삭제 가능).

`build-asm.cmd`/`build-lnk.cmd` 둘 다 실제로 실행해서 검증함(PowerShell로 호출 — git-bash에서 `cmd //c`로 부르면 인자 따옴표 처리가 꼬여서 실패하니 주의): 각각 `assembler.exe`/`linker.exe` 생성 확인, `assembler.exe`는 최소 테스트 프로그램(라벨+`addi`+`jmp`) 어셈블까지 정상 동작 확인, `linker.exe`는 인자 없이 실행해 크래시 없음만 확인(내부 로직은 사용자가 작성 중인 WIP라 검증 범위 밖).

## 개요

헤더 온리(header-only) C++ 커스텀 어셈블러. Windows/MSVC 대상. 자체 제작한 정규식 기반 렉서 생성기(`lexer.hpp`), LR 계열 파서 생성기(`parser.hpp`), 트리 순회 평가기 생성기(`evaluator.hpp`)를 이용해 `assembler.hpp` 안에서 실제 ISA-32 어셈블리 언어의 렉서/파서/평가기를 조립한다.

- `assembler.hpp` — 실제 작업이 이루어지는 파일 (lexer/parser/preprocesser/evaluator 네임스페이스 포함). **이 세션에서 가장 많이 수정된 파일.**
- `lexer.hpp` — 정규식→DFA 렉서 생성기 (제네릭, longest-match/maximal-munch)
- `parser.hpp` — LR/LALR류 파서 생성기 (제네릭, `CreateData` 문법 테이블 기반)
- `evaluator.hpp` — AST 순회 평가기 생성기 (제네릭)
- `preprocesser.hpp` — `#define`/`#macro` 전처리기 (제네릭)
- `main.cpp` — 진입점
- `../common/iotool.hpp` — 파일 IO 유틸 (namespace `iotools`, assembler/linker 공유)
- `../common/format.hpp` — 오브젝트 파일 직렬화 포맷(`Section`/`Mapper`/`Operation`/`Identifier`/`Header`/`Format`), `assembler::format` 네임스페이스와 별개인 독립 `format_data` 네임스페이스, assembler/linker 공유

## 아키텍처: 파서 생성기 문법 테이블

`parser_generator::ParserFactory<TokenType, NonterminalType, ASTNodeType>::CreateData`는
`{ {Nonterminal, ASTNodeAttribute, index}, {RHS 심볼들...} }` 튜플의 리스트.
- 세 번째 필드(index)는 reduce된 AST 노드의 `.type`을 RHS 중 몇 번째 요소의 토큰/타입으로 채울지 지정 (`-1`이면 ASTNodeAttribute를 그대로 사용).
- RHS 각 원소는 `{symbol, keep, splice}` — `keep`이면 자식 노드로 남기고, `splice`면 그 자식의 하위 노드들을 부모에 병합(리스트 누적 패턴에 사용).

## 표현식(expression) 문법: 14단계 우선순위 체인

LR 문법 충돌 없이 단항/이항 연산자를 구분하기 위해 `expressionL0`~`expressionL13`까지 계층화되어 있다 (전통적인 충돌 없는 CFG 형태, 우선순위 선언 불필요):

| 레벨 | 연산자 | 비고 |
|---|---|---|
| L0 | `( expr )`, number, identifier | 괄호는 L13을 참조 |
| L1 | 단항 `~ ! + -` | `+`/`-`는 L4(이항)와 토큰 공유 |
| L2 | `**` (우결합 거듭제곱) | |
| L3 | `* / %` | |
| L4 | `+ -` (이항) | |
| L5 | `<< >>` | |
| L6 | `< > <= >=` (관계) | |
| L7 | `== !=` (동등) | |
| L8 | `&` (비트 AND) | |
| L9 | `^` (비트 XOR) | |
| L10 | `\|` (비트 OR) | |
| L11 | `&&` (논리 AND) | |
| L12 | `\|\|` (논리 OR) | |
| L13 | `?:` (삼항) | `expression`의 진입점 |

`+`/`-` 토큰이 단항(L1)과 이항(L4) 양쪽에 쓰이므로, 평가기 `switch(cur->type)`에서는 같은 case 라벨 안에서 `cur->child.size() == 1`로 런타임 분기한다.

## 렉서 토큰

`assembler.hpp`의 `lexer::TokenType`에 논리/비교 연산자 토큰이 전부 선언 및 정규식 규칙 연결 완료:
`less greater lessequal greaterequal equal notequal lognot logampersend logverticalbar selector` (+ 기존 `ampersend verticalbar caret leftshift rightshift`).

정규식 메타문자 이스케이프 필요: `[ { ? + * | ( ) .` 와 백슬래시. `& < > = !  ^`는 리터럴.

## 평가기 (`evaluator::functions`)

### 타입 분류

- `ExprType::CONSTANT` — 완전히 상수로 계산된 값
- `ExprType::LINEAR` — `value + Σ coeff[section_id]·base(section)` 형태의 아핀 결합 (섹션 베이스 주소에 대해 미확정)
- `ExprType::COMPLEX` — 그 외 전부 (현재 더 이상 처리/전개하지 않음)

`ExprData`:
```cpp
struct _ExprData {
    ExprType type;
    s32 value;
    std::unordered_map<size_t, s32> coeff;   // section_id -> 계수
    std::vector<OperationToken> operations;   // 후위표기(RPN) 연산 이력
};
```

`OperationType` (POS는 제거됨 — 단항 +는 아무 일도 하지 않으므로):
```cpp
enum class OperationType {
    IDENTIFIER, CONSTANT,
    ADD, SUB, MULT, DIV, MOD, POW,
    SHIFTL, SHIFTR, BND, BOR, BNT, BXR,
    LESS, GREATER, LESSEQUAL, GREATEQUAL, EQUAL, NOTEQUAL,
    LAND, LOR, LNOT,
    NEG,
    SELECT
};
```

`LinearKind` — `combineBinary`/`combineUnary`가 LINEAR 승격 규칙을 결정하는 데 사용:
```cpp
enum class LinearKind { NONE, ADD_LIKE, SUB_LIKE, SCALE_LIKE, NEG_LIKE };
```
- `ADD_LIKE`: LINEAR+CONST, CONST+LINEAR, LINEAR+LINEAR → LINEAR (계수 합산)
- `SUB_LIKE`: LINEAR-CONST, CONST-LINEAR(계수 반전), LINEAR-LINEAR(계수 차감, 같으면 CONST로 축소)
- `SCALE_LIKE`: LINEAR*CONST, CONST*LINEAR → LINEAR (계수 스케일)
- `NEG_LIKE`: 단항 `-`의 LINEAR 피연산자 → 계수 부호만 반전 (`scaleCoeff(coeff, -1)`)
- `NONE`: 그 외 조합(예: LINEAR*LINEAR, LINEAR>>CONST, LINEAR/LINEAR 등)은 전부 `ExprType::COMPLEX`

승격 로직 요약(`combineBinary`/`combineUnary`):
1. 둘 다 CONST → CONST (함수 포인터로 실제 연산 수행)
2. `LinearKind`에 해당하는 LINEAR 규칙 매칭 시 → 계수 계산 결과가 비어 있으면 CONST로, 아니면 LINEAR로 (`combineBinary`만; `combineUnary`의 NEG_LIKE는 항상 LINEAR 유지)
3. 그 외 전부 → COMPLEX (`value=0`, `coeff` 비움)
4. 모든 경우 공통: `operations`는 자식들의 `operations`를 이어붙이고 현재 연산자를 후위(postfix)로 push

### `calculateConst` (병합된 핵심 평가 함수)

과거의 `calculateConst`/`calculateLabel`/`calculateNonconst` 세 함수를 이 하나로 병합함. 구조:
- `switch(cur->type)`에서 각 연산자 case는 `operationType`/`operationText`/`function`(또는 `unaryFunction`)/`linearKind`를 지역 변수에 설정만 하고 공용 라벨로 `goto`.
  - 이항 연산자 → `binaryOp:` 라벨 (마지막 이항 case 뒤에 위치) → `child1exp && child2exp` 검사 후 `combineBinary` 호출
  - 단항 연산자(`~ ! 단항-`) → `unaryOp:` 라벨 (마지막 단항 case 뒤에 위치) → `child1exp` 검사 후 `combineUnary` 호출
  - 단항 `+`는 예외: `combineUnary`를 거치지 않고 자식의 `ExprData`를 그대로 복사(passthrough) — `operations` 스택에 아무것도 push하지 않음
- `?:`(selector): 조건이 CONST면 분기 중 하나를 그대로 채택(상수 폴딩), 아니면 COMPLEX로 강등하며 `SELECT` 토큰을 후위 이력에 push
- 숫자 리터럴(`hexnum/decnum/octnum/binnum/character`): `{ExprType::CONSTANT, v, {}, {{OperationType::CONSTANT, text, v}}}`로 직접 구성 — value 필드까지 채워서 push
- `identifier`: `idenifierValue` 테이블에서 찾아 있으면 복사, 없으면(전방 참조) 그냥 skip — 두 번째 패스에서 해소됨
- `index`: 값이 음수면 실패(false) 반환
- `label`: 현재 섹션 바이트 오프셋을 `value`로, `coeff[section_id]=1`인 LINEAR로 만들어 `idenifierValue`에 등록. (과거 `label_text`/`label_data`/`label_bss`로 나뉘어 있던 AST 노드 종류를 단일 `label`로 통일함 — 아래 "최근 변경" 참고)
- `allocate`/`allocate_zero`: 크기 표현식이 `ExprType::CONSTANT`가 아니면 `return false`(에러) — 요청대로 구현됨

### 드라이버 (`evaluate()`)

전방 라벨 참조를 지원하기 위해 `calculateConst`를 두 번 호출:
1. 1차: 라벨을 best-effort로 정의 (전방 참조는 조용히 skip)
2. 섹션 바이트 오프셋 재계산(`bssByte`/`dataByte`/`textByte` 재배치)
3. 2차: 모든 식별자가 해소된 상태로 재평가

## 알려진 미해결/범위 밖 이슈 (건드리지 말 것 — 사용자가 명시적으로 범위 밖이라 함)

- **`curLine` 토큰 처리**: lexer에 `$` → `TokenType::curline` 토큰은 정의되어 있지만, 파서 문법이나 평가기 어디에도 아직 연결되지 않음. 사용자가 "이번 작업 범위 아님, 무시"라고 명시적으로 답변함 — 향후 별도 요청 없이는 임의로 구현하지 말 것.
- **`evaluate()` 드라이버의 미정의·불일치 참조**: `functions::calculateSize`가 선언 없이 호출됨(존재하지 않는 함수), `evaluate()` 끝부분과 `AssemblerDump`/`assemble()`에서 `data.textSection`/`data.dataSection`/`data.bssSection`이 `EvalData` 구조체에 없는 필드로 참조됨. `evaluateCode` 내부는 전부 `data->sections[sectionId].data` 형태로 통일됨(아래 참고) — 남은 문제는 `evaluate()`의 최종 `rawData` 조립부와 `AssemblerDump`/`assemble()`뿐. 전부 이 세션 이전부터 있던 기존 버그로, 명시적 요청 전까지 손대지 않음.
- ~~**`label_bss`**: `label_text`/`label_data`와 달리 `calculateConst`에서 아직 케이스가 없음.~~ → label 종류 통일로 해결됨(아래 참고).
- ~~**`pushByteN` 인자 순서 의심**~~ → 11번 항목에서 수정됨. 다만 `DataValue::length`가 어디서도 설정되지 않는 문제는 여전히 남아있음(`allocate`의 "data length" 표현식에서 채워져야 할 것으로 보이나 `calculateConst`에서 아직 연결 안 됨).
- **재배치 정보(`operationMap`/`operationVector`) 소비 루틴 없음**: `evaluateCode`가 기록만 할 뿐, 이 정보를 읽어 실제로 라벨 주소를 패치하는 후속 패스는 아직 없음(향후 구현 필요, 현재는 WIP 스키마).

## 이번 세션에서 진행된 작업 순서 (최근 → 과거 역순 아님, 시간순)

1. 논리/비교 토큰을 lexer 정규식 규칙 + parser 우선순위 체인(L6/L7/L11/L12/L13)에 연결
2. `calculateNonconst` 제거, `calculateConst`+`calculateLabel`을 하나로 병합 + CONST/LINEAR/COMPLEX 분류 규칙 구현 + `operations` 후위 기록 + `allocate`류 CONST 강제
3. `calculateConst`의 switch를 인수-설정 후 `goto binaryOp`/`goto unaryOp` 공용 처리 구조로 리팩터링
4. 단항 `+`/`-` 구현 (전부 `s32` 연산으로 통일, 별도 u32 특별 처리 없음)
5. `LinearKind`에 `negLike` 추가, `combineUnary`에 적용 (LINEAR 단항 `-` → `scaleCoeff(coeff, -1)`)
6. 숫자 리터럴 처리 시 `OperationToken{CONSTANT, text, value}`를 명시적으로 push하도록 수정
7. `OperationType::POS` 및 `LinearKind::POS_LIKE` 완전 제거, 단항 `+`는 `operations` 스택에 아무것도 남기지 않는 순수 passthrough로 변경 (현재 코드 상태에 최종 반영됨, grep으로 잔존 참조 없음 확인)
8. AST의 label 종류(`label_text`/`label_data`/`label_bss`)를 단일 `label`(Nonterminal/ASTNodeType 둘 다)로 통일. `text_section`/`data_section`/`bss_section` 문법 규칙이 이제 전부 `NT::label`을 직접 참조하고, 래퍼 문법 규칙 3개는 삭제됨. 부수효과로 `getSection`의 `ASTNodeType::label + TokenType::__end` case가 이제 실제로 매치됨(예전엔 실제 라벨 노드 타입이 label_text/label_data/label_bss였기 때문에 매치되지 않던 기존 버그였음), `calculateConst`의 두 케이스(`label_text`/`label_data`)도 하나(`label`)로 병합되며 부수적으로 `label_bss`도 동일 처리됨(예전 비대칭 이슈 해결).
9. `evaluateCode`에서 존재하지 않는 `data->textSection` 필드를 참조하던 나머지 `push32(...)` 호출 9곳(`instruction_RI/RR/II/RII/IRI/RVRI/FVRI/IXP/RXR`)을 전부 `data->sections[sectionId].data` 형태로 통일 (이미 `instruction_N`과 `allocate` case는 이 패턴을 쓰고 있었음).
10. (1차 시도, 이후 11번으로 대체됨) `EvalData`의 재배치 스키마가 처음엔 `InstValue{data, index}`/`DataValue{length, data, index}` 형태(재배치 대상 인덱스 하나만 표현, `combineIndex`로 전파)였다가, 사용자가 직접 `allocate` case를 `ReplaceData{index, offset, size}` 기반의 `.replace` 벡터 방식으로 고쳐놓아 재작성함(11번 참고).
11. `ReplaceData{index, offset, size}`(비트 단위 offset/size + `operationVector`로의 index) 기반 재배치 스키마로 전환. `InstValue`/`DataValue`는 이제 `std::vector<ReplaceData> replace`를 가지며, `Section::operationMap`은 `unordered_multimap<u32(섹션 내 바이트 위치), ReplaceData>`. `evaluateCode`의 `case allocate`(사용자가 직접 재작성한 템플릿: CONSTANT면 `.data`=값, 아니면 `.data`=0 + `ReplaceData{index=operationVector.size(), offset=0, size=32}`를 `.replace`에 push + `operations`를 `operationVector`에 push)를 참고해 나머지 모든 case에도 동일 패턴을 반영:
    - `immidate16`/`immidate8`/`reg_index`: `allocate`와 동일하게 `ExprData`를 직접 검사 — 비 CONSTANT일 때 `ReplaceData.size`는 각 필드 폭(`immidate16`=16, `immidate8`=8, `reg_index`=32)으로 설정
    - `instruction_N/R/RI/RR/II/RII/IRI/RVRI/FVRI/IXP/RXR`, `reg`: 새 헬퍼 `appendReplace(dest, src, shift)`로 자식의 `.replace` 항목들을 각 필드가 최종 값에서 놓이는 비트 시프트만큼 `offset`을 보정해 병합(`.data` 계산에 쓰인 것과 동일한 `<<` 상수를 그대로 재사용) — 여러 필드가 동시에 재배치 대상이어도 모두 보존됨(이전 `combineIndex` 방식의 "최대 하나만 전파" 단순화보다 정확함)
    - 바이트 push 직전에 새 헬퍼 `registerReplace(data, sectionId, replace)`로 현재 `sections[sectionId].data.size()`(push 전 위치)를 키로 `.replace`의 각 항목을 `operationMap`에 등록
    - `reg_ex`/`regid`/`regmode`: 고정 테이블 조회라 `ExprData`/재배치 없음 — `.data`만 채움(`.replace`는 기본 빈 벡터로 둠)
    - `allocate` case의 `pushByteN` 호출 인자 순서를 시그니처(`pushByteN(list, data, size)`)에 맞게 `(sections[...].data, dataValue[cur].data, dataValue[cur].length)`로 수정(기존엔 data/length가 뒤바뀌어 있었음 — 아래 "알려진 이슈"의 관련 항목도 해결됨)
12. `pctl`/`getabs`/`flush`/`inval` 명령어 추가 (사용자 스펙: `pctl [enable(=1)/disable(=0)]`, `getabs [src-reg],[dest-reg]`, `[flush/inval] [base-reg],[imm8],[id]` — `id ∈ {ALL=0x80, L2=0x8, TLB=0x4, L1D=0x2, L1I=0x1}`):
    - lexer: `pctl/getabs/flush/inval/enable/disable/L1D/L1I/TLB/L2/ALL` 리터럴 규칙 + tokenStr 추가 (토큰 자체는 이전부터 `TokenType`에 선언만 되어 있었음)
    - 새 논터미널/AST 타입: `ctlid`(enable/disable), `cacheid`(ALL/L2/TLB/L1D/L1I), `instruction_CTL`(pctl 전용, 오퍼랜드 없이 opcode에 변이값만 더함 — `ld.<regid>`/`jmp.<flagid>`의 RVRI/FVRI와 같은 "opcode에 변형 더하기" 패턴이지만 reg/imm 필드가 아예 없는 축소판), `instruction_CACHE`(flush/inval 전용, reg+imm8+cacheid 3필드 — RII과 같은 모양이지만 RII는 intervec에서 이미 절반만 연결된 상태라 건드리지 않고 새로 만듦)
    - `getabs`는 새 AST 타입 없이 기존 `instruction_RR`(reg,reg)을 그대로 재사용
    - `instructionBase`: `pctl=0x7C`(2슬롯: disable=0x7C, enable=0x7D), `getabs=0x7E`, `flush=0x7F`, `inval=0x80`. 새 테이블 `ctlBase`(disable=0,enable=1), `cacheBase`(L1I=0x1,L1D=0x2,TLB=0x4,L2=0x8,ALL=0x80)
    - `getSection`/`calculateConst`(섹션 바이트 +4)/`evaluateCode`(인코딩+`operationMap` 등록+`push32`) 세 곳 모두에 `instruction_CTL`/`instruction_CACHE` case 추가; `ctlid`/`cacheid`는 고정 테이블 조회라 `.replace`는 항상 빈 벡터
    - opcode 값(0x7C~0x80)과 오퍼랜드 형식은 레포에 별도 ISA 문서가 없어 전부 사용자가 대화 중 직접 지정한 스펙을 그대로 따름 — 향후 ISA 문서가 별도로 생기면 그쪽을 우선 참고할 것
13. `evaluateCode`의 `instruction_*` case들(N/R/RI/RR/II/RII/IRI/RVRI/FVRI/IXP/RXR/CTL/CACHE, 13개)에서 반복되던 공통 꼬리(`registerReplace` + `push32`)를 `calculateConst`의 `binaryOp:`/`unaryOp:`와 같은 goto 병합 패턴으로 리팩터링. 각 case는 `.data` 계산과 `appendReplace`만 하고 `goto pushInstruction;`으로 점프(마지막 case인 `instruction_CACHE`만 라벨로 자연스럽게 fall-through), `pushInstruction:` 라벨 하나에서 `registerReplace`+`push32`+`break` 공통 처리. `allocate` case는 `pushByteN`(가변 길이)을 써서 꼬리 모양이 달라 이 병합에서 제외.
14. 사용자가 직접 새로 추가한 `assembler::format` 네임스페이스(오브젝트 파일 출력 포맷 직렬화 레이어 — `Section`/`Mapper`/`Operation`/`Identifier`/`Header`/`Format` 구조체, `formatData(Format&, const EvalData&)`)의 `Header::getRaw`/`Format::getRaw`를 채움:
    - `Header::getRaw`: 먼저 헤더 자신의 크기(`size()` = 13개 u32 필드 × 4바이트 = 52, `push32`로 기록) → 6개 구역(sections/binary/mapper/expressions/definedIdentifier/usedIdentifier)의 start/end 인덱스 쌍을 순서대로 `push32`
    - `Format::getRaw`: 각 구역을 임시 버퍼에 먼저 직렬화(`sections`/`mapper`/`definedIdentifier`는 각 원소의 기존 `getRaw` 호출, `binary`는 이미 raw라 그대로, `usedIdentifier`는 각 문자열 뒤에 `'\0'` 하나씩 push해 이어붙임, `expressions`는 `vector<vector<Operation>>`이라 자체적으로 길이 구분이 안 되므로 각 하위 벡터 앞에 원소 개수를 `push32`로 붙이고 그다음 각 `Operation::getRaw` 나열— 이 count-prefix 방식은 사용자가 명시하지 않아 직접 판단한 부분) → 각 임시 버퍼 크기로 `Header`의 start/end를 누적 계산(헤더 자신의 크기만큼 오프셋에서 시작) → `header.getRaw(out)`로 헤더를 맨 앞에 쓴 뒤 6개 구역을 순서대로 이어붙임
    - ~~`formatData()`의 `expressions` 채우는 루프가 `copy`를 계산만 하고 `out.expressions`에 push하지 않는 기존 미완성 코드~~ → 15번에서 해결됨
18. **첫 실제 빌드/테스트 수행** (사용자가 명시적으로 요청함 — `g++ -std=c++17` / MinGW로 `main.cpp` 컴파일+링크, `assembler_test.exe`로 몇 개 최소 테스트 프로그램 실행). `formatData()`의 `usedIdentifier`/`definedIdentifier`를 채움:
    - `usedIdentifier`: `formatData()`가 `operationVector`→`Operation` 변환 루프를 도는 동안, `OperationType::IDENTIFIER` 타입 토큰을 만나면 그 `.text`(식별자 이름)를 `unordered_map<string,u32>`로 중복 제거하며 `format.usedIdentifier`에 등록하고, 해당 `Operation.value`를 그 인덱스로 덮어씀(기존엔 `.value`가 항상 0으로 버려지고 있었음). 같은 이름이 여러 표현식에서 재참조돼도 한 번만 등록됨(hex 덤프로 직접 확인).
    - `definedIdentifier`: `data.idenifierValue`(라벨 심볼 테이블)의 각 항목을 `Identifier{name, (u32)exprData.value}`로 변환해 push (라벨의 섹션-로컬 오프셋 값 그대로 사용 — coeff/섹션 정보는 별도로 `mapper`/`sections`가 담당한다고 가정).
    - 빌드해서 실제로 돌려보는 과정에서 이 세션의 여러 concurrent 변경이 겹치며 생긴 컴파일 에러/런타임 크래시 3건을 직접 만나서 고침(전부 `formatData` 자체보다는 그걸 실행하기 위해 거쳐야 하는 파이프라인의 문제였음):
      - `_EqualOperation::operator()`에 `const` 누락 → `unordered_map::find`가 컴파일 안 됨(컴파일 에러). `const` 추가로 해결.
      - 단항/이항 비교·논리 연산자 람다들(`< > <= >= == != && || !` , `**`)이 `int`를 리턴하는데 `s32(*)(s32,s32)` 함수 포인터에 대입 — 이 플랫폼(MinGW, `s32`=`long`)에서는 `int`↔`long` 함수 포인터가 호환 안 돼 컴파일 에러(`-fpermissive` 없이는 빌드 자체가 안 됨). 각 람다에 `-> s32` 명시적 리턴 타입을 붙여 해결.
      - `calculateSection`(구 `getSection`)이 `bool`을 리턴하는데 실제로는 `return`문이 하나도 없어 UB(운 나쁘면 false 반환) → `evaluate()`의 첫 단계로 새로 연결되면서 이 잠재 버그가 처음으로 드러남. 끝에 `return true;` 추가.
      - `.text`/`.data`/`.bss`를 이름 없이 쓰는 문법(문법상 허용되고, `program/`의 실제 샘플 전부가 이 형태)이 AST 노드에 자식이 0개인데 `calculateSection`이 무조건 `cur->child[0]->text`로 이름을 읽어 세그폴트 → `cur->child.empty() ? "" : cur->child[0]->text`로 가드.
      - `EvalData::sectionByte`가 어디서도 `push_back`/`resize`되지 않아 항상 빈 벡터인데 여러 곳에서 `sectionByte[section_id]`로 인덱싱 → 세그폴트. `calculateSection`이 새 섹션을 만들 때(`data->sections.push_back(nSection)` 직후) `data->sectionByte.push_back(0)`도 같이 하도록 수정해 두 벡터를 lockstep으로 유지.
      - (부수적으로 함께 고침) `AssemblerDump::parseSuccess/preprocSuccess/evalSuccess`가 초기값 없는 지역 변수라 해당 덤프 플래그(`-preproc` 등)를 안 켜고 성공한 경우 인디터미네이트 값이 출력됨 → 전부 `= false`로 기본값 지정. (단, `main.cpp`의 표시 조건 `if (dumpPreproc || !dump.preprocSuccess)` 자체는 "플래그를 안 켜면 항상 실패로 보임"이라는 근본적인 설계 문제라 이번엔 안 건드림 — `-preproc`/`-eval` 등 관련 플래그를 켜서 확인해야 정확한 성공/실패를 볼 수 있음.)
    - 위 수정들 덕분에 `program/` 밑 실제 샘플 없이도 만든 최소 테스트(`.text` + 라벨 + `add`/`addi`/`jmp`)로 preproc/parse/evaluate 전부 성공, `formatData`+`format::getRaw` 결과 바이너리를 직접 헥스덤프/파싱해서 `usedIdentifier`(중복 제거 확인 포함)/`definedIdentifier`/`expressions`/`mapper`/`Header`가 서로 정확히 들어맞음을 확인함.
    - **(이후 19번에서 해결됨) 당시 남은 문제**: `registerReplace`의 바이트 오프셋 계산 off-by-one, `formatData()`가 `format.sections`를 채우지 않던 문제.
19. 사용자가 18번의 두 남은 문제를 직접 수정한 뒤 "확인해줘"라고 요청 — 재검증 결과:
    - `registerReplace`가 `basePos = data.size() + byte - 1`로 수정됨(사용자 작업) → 16비트 필드/offset=0 케이스에서 mapper.byteIndex가 이제 명령어의 LSB 바이트(빅엔디안 4바이트 워드의 마지막 바이트)를 정확히 가리킴을 재검증 완료(`jmp.zero`/`jmp.one` 두 명령어 각각 byteIndex=7/11로, 각 워드의 마지막 바이트와 일치).
    - `formatData()`가 `Section nSec`을 만들어놓고 `format.sections.push_back(nSec)`을 빠뜨린 게 남아있어서(`format.sections`가 여전히 항상 빈 배열) 직접 추가해 완료함 — 재검증 결과 `sections` 구역에 `{name:"", start:0, end:12}` 1건이 정상적으로 들어감.
    - 두 테스트(라벨 1회/2회 참조) 모두 빌드 성공 + preproc/parse/evaluate 성공 + 헥스덤프로 `sections`/`binary`/`mapper`/`expressions`/`definedIdentifier`/`usedIdentifier` 6개 구역 전부 상호 일치 확인함.
20. 사용자가 `AssemblerDump::sectionDump`(구 `textSection`/`dataSection`/`bssSection`을 `std::vector<evaluator::functions::Section>`로 대체) + `assemble()`의 채우는 코드 + `main.cpp`의 `-bin` 덤프 출력 로직을 직접 추가 → "sectionDump 빌드 후 확인해줘" 요청으로 검증:
    - `main.cpp`의 섹션별 바이트 출력 이중 루프에서 안쪽 루프 변수가 `si`(바이트 이터레이터)인데 `printf("%02X ", *it)`로 바깥 루프 변수(`it`, `Section`)를 잘못 역참조하는 복붙 버그 발견 → `*si`로 수정.
    - 수정 후 빌드 성공(에러/경고 없음), `-bin`으로 실행해 `section: <name>` 아래 실제 섹션 바이트(`02 00 00 01 / 60 00 00 00 / 66 00 00 00`)가 올바르게 출력됨을 확인.
21. `assemble()`에서 `dump.preprocSuccess`/`dump.parseSuccess`/`dump.evalSuccess`를 각 단계의 실제 결과(`validPreprocess`/`validParse`/`validEvaluate`)로 **덤프 플래그와 무관하게 항상** 기록하도록 변경(이전엔 `dump.flags & AssemblerDump::getXXX`가 켜져 있거나 실패했을 때만 기록돼서, 관련 플래그 없이 성공하면 초기값 `false`가 그대로 노출되던 문제 — 18/20번에서 언급한 "관련 플래그를 켜야 정확한 성공/실패를 볼 수 있다"는 제약이 이걸로 해결됨). 무거운 부가 데이터(`preprocTokens`/`pAST`/`errorMessage`)는 계속 기존처럼 플래그(또는 실패 시)에만 채움 — success bool만 항상 갱신, 나머지는 그대로 lazy. `-bin`만 켜고 실행해 재확인: 성공 케이스에서 preproc 섹션은 표시 안 되고 parse/evaluate는 플래그 없이도 "success"로 정확히 뜸, 문법 오류 파일에서는 parse/evaluate 둘 다 "failed"+에러 메시지가 정상 출력됨.
17. `format` 네임스페이스 구조 정리:
    - `Section`/`Mapper`/`Operation`/`Identifier`/`Header`/`Format` 구조체 전부를 새 파일 `format.hpp`로 추출 (`namespace assembler::format`을 자체적으로 완성해서 선언 — `evaluator::functions::push32`에 의존하지 않도록 자체 `push32` 헬퍼를 복제해 넣어서, 다른 제네릭 헤더들(lexer.hpp/parser.hpp/evaluator.hpp)처럼 `assembler.hpp` 최상단 include 목록에 나란히 추가 가능하게 함). `assembler.hpp`는 이제 `#include "format.hpp"`만 하고, 같은 네임스페이스를 다시 열어 `EvalData` using과 `formatData()`/`getRaw()` 같은 assembler 쪽 로직만 남김.
    - 사용자가 먼저 concurrently 추가해둔 전역 `Format format;` 객체에 맞춰, `formatData()` 안의 지역 `out.` 참조를 전부 `format.`으로 바꿈(매개변수 `out`은 이미 제거되고 전역 `format` 객체를 쓰는 구조로 바뀌어 있었는데 본문이 아직 안 따라간 상태였음)
    - `format` 네임스페이스에 `inline void getRaw(std::vector<u8>& out) { format.getRaw(out); }` 추가 — 전역 `format` 객체를 직접 만지지 않고 바깥(`assemble()` 등)에서 직렬화 결과를 얻는 진입점. (네임스페이스 이름과 전역 변수 이름이 둘 다 `format`으로 같지만, 변수 선언이 네임스페이스 본문 내부 스코프라 합법적으로 컴파일됨 — `namespace foo { int foo; }` 패턴과 동일)
15. `OperationType`에 `EOX`(end of expression) 추가 — 항상 0번지(맨 앞)로, 이후 요소들은 한 칸씩 밀림. `formatData()`가 각 표현식의 연산 목록(`copy`) 끝에 `EOX` 토큰을 붙인 뒤 비로소 `out.expressions.push_back(copy)`로 실제 저장하도록 수정(14번에서 언급한 "계산만 하고 저장 안 하던" 미완성 코드가 이 참에 완성됨). `Format::getRaw`의 `expressions` 직렬화도 각 표현식 앞에 붙이던 개수 `push32` 프리픽스를 제거하고, `EOX`가 종결자 역할을 하므로 각 `Operation`을 그냥 순서대로 이어붙이기만 하도록 변경.

## 작업 방식 관련 주의사항

- **빌드/테스트를 절대 자동으로 수행하지 말 것** — 사용자가 "아직 완성본 아니니까 빌드나 테스트까지는 하지 말고"라고 명시함. 명시적 요청이 있을 때만 수행.
- 이 파일(`assembler.hpp`)은 사용자가 대화 밖에서 직접 동시 편집하는 경우가 있었음(과거 Edit 충돌 이력) — 편집 전에는 항상 최신 상태를 다시 Read할 것.
