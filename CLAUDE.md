# CLAUDE.md — src/assembler-linker

이 파일은 Claude Code가 이 디렉토리(`computer 32 (new)/src/assembler-linker`, 어셈블러+링커 프로젝트 루트)에서 작업을 이어갈 때 빠르게 현재 상태를 파악하기 위한 참고 문서입니다. (2026-09-20까지는 `assembler/` 서브폴더에 있었으나 이후 사용자가 루트로 옮김.) **이 프로젝트는 아직 미완성이며, 명시적으로 요청받기 전까지는 빌드/테스트를 수행하지 않습니다.**

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
22. (2026-09-21) `common/format.hpp`의 `loadRaw` 계열(역직렬화, `getRaw`의 대응 함수) 완성. 사용자가 이미 `read32` 헬퍼 + `Section`/`Mapper`/`Operation`/`Identifier`의 `loadRaw(iterator&)` 초안과 `Format::loadRaw(vector<u8>&)` 빈 스텁을 만들어뒀길래("현재 진행중인 방식" 확인 후) 그 패턴을 그대로 따라 나머지를 완성:
    - `read32`가 미완성이었음 — `union`을 `conv[i]`로 배열처럼 접근(컴파일 에러), 세미콜론 누락, `inline` 누락(공용 헤더라 여러 TU에 include될 걸 대비해 `push32`처럼 `inline` 필요) → 전부 수정.
    - `Section`/`Mapper`/`Identifier`의 `loadRaw`가 이름 문자열을 널 종단까지 읽고 나서 그 널바이트 자체를 건너뛰지 않아 다음 필드를 한 바이트 밀려 읽는 버그 → 각각 루프 뒤에 `++it` 추가.
    - `Mapper::loadRaw`가 존재하지 않는 필드 `name`에 쓰고 있었음(실제 필드명은 `sectionName`) → 수정.
    - `Identifier::loadRaw`가 존재하지 않는 `load32`를 호출(오타, `read32`여야 함) → 수정.
    - `Header`엔 `loadRaw`가 아예 없었음 → `getRaw`와 정확히 같은 순서로 저장된 크기(값은 버림, 재직렬화 시 `size()`로 다시 계산되므로) + 6개 구역 start/end 12개 필드를 읽는 함수 추가.
    - `Format::loadRaw`(빈 스텁)를 완성: `Header::loadRaw`로 헤더를 먼저 읽고, 헤더가 알려주는 각 구역의 절대 offset(`in.cbegin() + header.XStart` ~ `XEnd`)으로 구역별 독립 파싱 — `sections`/`mapper`/`definedIdentifier`는 구역 끝까지 반복하며 개별 구조체 `loadRaw` 호출, `binary`는 통째로 `assign`, `expressions`는 `EOX`(`OperationType::EOX`, type=0)를 만날 때까지 `Operation`을 읽어 한 표현식으로 묶는 걸 반복(기록 쪽이 표현식 끝마다 `EOX`를 붙이는 것과 대응), `usedIdentifier`는 구역 끝까지 널 종단 문자열을 반복해서 읽음. 매개변수명은 기존 스텁의 `out`(입력인데 출력 이름이라 오해 소지, 호출부가 아직 없어 안전하게 변경 가능했음)을 `in`으로 바꿈.
    - 검증: `assembler::assemble()`로 만든 원본 `format::format`을 `getRaw`로 직렬화 후 새 `Format`에 `loadRaw`로 역직렬화해서 6개 구역 전부(`sections`/`binary`/`mapper`/`expressions`/`definedIdentifier`/`usedIdentifier`) 원본과 필드별 비교하는 라운드트립 테스트를 두 케이스(라벨 1개+반복 참조 / `.data`+`.text` 혼합, 서로 다른 식별자 여러 개)로 작성해 둘 다 일치 확인("ROUND TRIP OK"). 테스트 파일은 임시 디렉토리에서만 작업 후 정리, 저장소엔 남기지 않음.

23. (2026-09-23) 두 가지 요청 처리 — "assembler에서 identifier struct에 section 이름 추가한거 반영해주고 linker의 stackMachine namespace 완성해줘":
    - **`Identifier.section` 반영**: 사용자가 `common/format.hpp`의 `_Identifier`에 `std::string section;` 필드를 직접 추가해뒀길래, `getRaw`/`loadRaw`에 name/value 사이 section 필드 직렬화(널 종단 문자열)를 추가하고, `assembler.hpp`의 `formatData()`에서 반쯤 쓰다 만 `nId.section = ` (세미콜론도 없이 끊긴 상태)을 완성함: `it->second.coeff`(라벨의 `ExprData.coeff`, section_id→계수 맵)에서 section_id를 뽑아 `data.sections[section_id].name`으로 매핑, coeff가 비어있으면(CONSTANT로 완전히 확정된 식별자) `""`.
    - 그 근처에서 컴파일을 막고 있던 별개 버그도 같이 고침: `formatData()`의 섹션 루프에서 멀티섹션 절대 오프셋 누적용 `u32 base` 선언 및 `base = format.binary.size()` 갱신이 통째로 빠져있었음(마지막으로 내가 만든 버전엔 있었는데 이후 편집 중 사라짐) → 복원, `mapper.byteIndex`도 `base +` 다시 붙임.
    - **라운드트립 검증**: `assembler::assemble()`로 실제 조립 후 `getRaw`→`loadRaw`로 `definedIdentifier`(`name`/`section`/`value`)까지 필드별 비교, 일치 확인.
    - **⚠️ 검증 중 발견한 별개의 기존 버그(이번 범위 밖, 손대지 않음)**: `.data`+`.text`를 함께 쓰는 프로그램을 평가해보면 `data.sections`에 섹션이 딱 1개(TEXT)만 들어가고 `.data`용 `Section`이 아예 안 생김 — `.data`에 정의된 식별자(`value` 등)도 전부 `coeff={0:1}`로 TEXT 섹션에 잘못 귀속됨. `calculateSection`(어디선가 `.data_section`/`.text_section` AST 노드를 개별적으로 처리해 `data->sections.push_back`해야 하는데, 실제로는 프로그램 전체에 걸쳐 한 번만 실행되는 것으로 보임 — 원인 미조사)의 섹션 카운팅 문제로 추정. **링커가 여러 섹션을 다루기 시작하면 반드시 먼저 고쳐야 함** — 지금 상태로는 `.data` 심볼이 `.text` 섹션 소속으로 잘못 직렬화됨.
    - **`stackMachine` 네임스페이스 완성** (`linker/linker.hpp` + `linker/stackmachine.hpp`): 사용자가 짜다 만 `SMOperation`(불완전한 enum, 세미콜론 없음)을 걷어내고 이미 있는 `format_data::OperationType`을 그대로 재사용(`OpType`로 별칭) — 별도 enum을 유지/매핑할 필요가 없어짐. 제네릭 엔진(`stack_machine_generator::StackMachine<Operation>`, `stackmachine.hpp`)도 여러 버그가 있어 같이 고침: `std::vector`엔 `.pop()`이 없음(`.pop_back()`이어야 함), 클래스 닫는 `}` 뒤 세미콜론 누락, `Operation::__const` 센티널이 실제로는 존재하지 않는 이름(→ `Operation::CONSTANT`로 통일), 인자 슬라이싱 방향이 뒤집혀 있어 RPN을 거꾸로 읽던 문제 → 입력을 앞에서부터 순회하며 러너스택에 쌓고, 연산자를 만나면 `Element.value`를 "이 연산자가 팝할 피연산자 개수(arity)"로 재해석해 그만큼 팝하는 방식으로 재작성(어셈블러가 만드는 원본 `OperationToken`은 연산자에 value를 안 채워 넣으므로, 이 arity는 `format_data::Operation`→`Element` 변환 단계인 `runOperation()`에서 `arity(OpType)` 테이블로 새로 채워 넣음 — BNT/LNOT/NEG는 1, SELECT는 3, 나머지는 2).
      - `function()`: `format_data::OperationType`의 산술/비교/논리 전체(ADD~SELECT, EOX/IDENTIFIER/CONSTANT 제외)를 구현 — 전부 `s32`로 캐스팅해서 연산 후 `u32`로 되돌림(부호 있는 나눗셈/시프트/비교가 정확하도록; 비트 연산·동등비교는 `u32`로 그대로).
      - `runOperation(operation, resolvedIdentifiers, result)`: `Format::expressions`에 저장된 형태 그대로(=`EOX`로 끝나는 `format_data::Operation` RPN 목록)를 받아 `Element` 목록으로 변환 후 실행. `IDENTIFIER` 타입은 `.value`가 `usedIdentifier`의 인덱스이므로, 호출자가 넘겨준 `resolvedIdentifiers`(인덱스별 실제 주소값, 심볼 테이블 병합 후 채워질 값)로 치환해서 `CONSTANT`처럼 취급 — 그래서 제네릭 엔진은 `CONSTANT` 하나만 "리프"로 알면 됨.
      - `linker::stackMachine`은 아직 아무 데서도 호출 안 됨(`linker::evaluate`/`link()`는 여전히 미완성 WIP — 문법 에러 다수: `std::Vector` 오타, `namespace format`이 `namespace evaluate`보다 먼저 나오는데 `evaluate::functions::EvalData`를 참조, 끊긴 함수 선언 등 — 이번 요청 범위 밖이라 손대지 않음). 그래서 `linker.hpp` 전체는 아직 안 빌드됨 — `stackMachine` 부분만 별도 스탠드얼론 테스트 파일로 격리해서 컴파일+실행 검증: `(2+3)*4-1=19`(이항 연산+우선순위), `IDENTIFIER` 치환 후 `+100`(=142), 단항 `NEG`(5→-5) 세 케이스 전부 통과.

24. (2026-09-23) "runOperation에는 stackMachineRun만 남기고 나머지 처리는 일단 castOperation로 옮겨줘" — 23번에서 만든 `stackMachine::runOperation`(포맷→Element 변환 + 실행을 한 함수에 다 담고 있었음)을 역할별로 분리:
    - `stackMachine::runOperation`은 이제 이미 만들어진 `std::vector<Element>`를 받아 `stackMachine.run(elements, function, result)` 한 줄만 함 — 순수 실행부만 남김.
    - 변환부(포맷 `Operation` 목록을 `EOX`까지 순회하며 `IDENTIFIER`는 `resolvedIdentifiers`로 치환, 연산자는 `stackMachine::arity()`로 팝 개수 채우기)는 `evaluate::functions::castOperation`(원래 빈 스텁)으로 옮김 — `namespace evaluate`가 심볼 테이블 병합을 담당할 계층이라 "포맷 표현 ↔ 실행 표현 변환"도 거기 있는 게 맞음. `stackMachine`은 이제 `format_data::Operation`을 전혀 몰라도 되는 순수 RPN 실행기로 남음.
    - `evaluate::functions`에 `u32`/`u8` 별칭이 아예 없어서(이미 그 자리에서 `std::vector<u8>`, `std::map<string,u32>` 등 기존 코드가 쓰고 있었는데도) `castOperation`을 넣으려니 컴파일이 안 되는 상태였음 → `using u32 = format_data::u32; using u8 = format_data::u8;` 추가(다른 `using SectionType = ...` 줄과 같은 자리, 같은 패턴).
    - `evaluate` 네임스페이스의 나머지(`mergeSection`/`EvalData`/`link()`)는 여전히 별개의 broken WIP라 전체 `linker.hpp` 빌드는 안 됨(23번에 적어둔 이유 그대로) — `stackMachine`+`castOperation` 조합만 독립 테스트로 격리해서 검증: `(2+3)*4` 후 단항 `NEG` = `-20`, `castOperation`이 6개 Element로 변환하고 `runOperation`이 그걸 실행해 정확한 결과 확인.

25. (2026-09-23) ".data+.text버그 분석해줘" — 23번에서 발견한 "`.data`+`.text`를 같이 쓰면 섹션이 1개로 합쳐지고 `.data` 식별자가 `.text` 섹션 소속으로 잘못 기록되는" 버그를 (수정은 안 하고) 원인만 분석. `/tmp` 스크래치 디렉토리에 소스 복사본을 만들어 디버그 프린트 삽입 → 빌드 → 실행 → 삭제하는 방식으로, 실제 저장소 파일은 전혀 건드리지 않고 진행함.
    - **확정된 근본 원인**: `calculateSection`의 섹션 헤더 케이스(`text_section`/`data_section`/`bss_section`/`outer_section`)가 다른 모든 AST 노드 케이스(`index`, `label`, `instruction_*` 등)와 달리 `+ (u64)TokenType::__end` 오프셋 없이 **맨 `ASTNodeType` 값 그대로** 비교하고 있음. 실제 실행 중 텍스트섹션에 해당하는 두 가지 서로 다른 AST 리듀스(래퍼 `NT::section` 규칙 자신 vs 안쪽에서 누적되는 `NT::text_section`/`NT::data_section` 바디 리스트 규칙 — 둘 다 같은 `AT::text_section`/`AT::data_section` 태그를 쓰지만 파서 테이블 상에서 서로 다른 `cur->type` 값(맨값 `3`/`4` 대 오프셋값 `143`/`144`)으로 나타남)이 있는데, 현재 스위치는 이 중 **맨값과 우연히 일치하는 것 하나만** 잡고 나머지는 전부 놓침 — 그래서 `.data`+`.text` 두 섹션이 있어도 `process_section`이 사실상 한 번만(그리고 어느 쪽이 잡히는지는 문법 규칙 매칭 우연에 좌우됨) 실행돼 `data->sections`가 항상 1개로 끝남. `sectioningArr`에 쌓인 라벨/명령어 노드들은 (그 하나뿐인) section index로만 매핑되니, `.data`에서 정의된 식별자도 전부 `.text` 섹션 소속(`coeff={0:1}`)으로 잘못 기록됨.
    - **검증**: 스크래치 복사본에서 4개 케이스 라벨에 `+ (u64)TokenType::__end`만 붙이고(다른 모든 AST 케이스와 동일한 패턴으로 맞춤) 캐시된 `_table.bin`도 지우고 재빌드 → `.data`+`.text` 프로그램에서 `data.sections.size()`가 1→2로 정상화, `value` 식별자가 DATA 섹션, `loop`/`start`가 TEXT 섹션으로 올바르게 분리됨을 확인. **이 수정은 스크래치 사본에만 적용했고 실제 저장소 파일(`assembler.hpp`)은 이번 턴에서 건드리지 않음** — 요청이 "분석"이었기 때문에 다음 턴에 명시적으로 요청 시 적용 예정.
    - **검증 중 발견한 부수 버그(같은 함수, 별개 원인)**: 위 수정을 적용하고 나니 섹션 이름이 이상하게 나옴 — `.data`/`.text`를 이름 없이 썼는데도 DATA 섹션 이름이 `"value"`, TEXT 섹션 이름이 `"start"`로 나옴(둘 다 그 섹션 안 첫 번째 식별자 이름을 잘못 가져온 것). 원인: `nSection.name = cur->child.empty() ? "" : cur->child[0]->text;`가 "이름 있는 섹션"(`{TT::identifier, true, false}`가 child[0])과 "이름 없는 섹션"(`{NT::text_section, true, false}` 바디가 child[0])을 구분 못 하고 항상 `child[0]->text`를 이름으로 취급함 — 이름 없는 쪽은 child가 비어있을 때만 정상(`""`)이고, 바디가 있으면(=섹션 안에 뭔가 있으면) 바디 리듀스 과정에서 위로 전파된 첫 자식의 텍스트를 실수로 이름처럼 읽음. 이건 이번 요청 범위(섹션 개수/식별자 소속) 밖이라 같이 고치진 않았지만, 위 근본 원인 수정 시 같이 손봐야 실제로 쓸만해짐(`cur->child[0]->type == (u64)TokenType::identifier`인지 확인해서 진짜 이름이 있을 때만 읽도록).

26. (2026-09-24) "해당 문제 해결해줘 이름 문제는 TT::identifier 인지로 구분하면 될 듯?" — 25번에서 분석만 하고 남겨뒀던 두 버그를 실제로 `assembler.hpp`에 적용:
    - `calculateSection`의 4개 섹션 헤더 케이스에 `+ (u64)TokenType::__end` 오프셋 추가(다른 모든 AST 케이스와 동일한 패턴으로 통일).
    - `nSection.name`을 `(!cur->child.empty() && cur->child[0]->type == (u64)TokenType::identifier) ? cur->child[0]->text : ""`로 변경 — 사용자가 제안한 대로 child[0]의 실제 토큰 타입이 `TT::identifier`일 때만(=진짜 이름 있는 섹션 규칙일 때만) 이름으로 읽고, 그 외(바디만 있거나 아예 비어있는 이름 없는 섹션)는 `""`.
    - 캐시된 `build/assembler_table.bin`도 같이 삭제(예전 문법 상태로 빌드된 캐시를 계속 재사용하면 25번에서처럼 소스와 실제 실행 결과가 어긋날 수 있어서 — 재빌드 시 자동으로 새로 생성됨).
    - **검증**: 세 가지 케이스로 확인 — ① 이름 없는 `.data`+`.text`: `sections.size()`가 2로 정상화, 둘 다 이름 `""`(정상, 진짜 이름이 없으므로), `value`는 DATA 섹션 coeff={0:1}, `loop`/`start`는 TEXT 섹션 coeff={1:1}로 내부적으로도 완전히 분리됨을 raw `EvalData`까지 내려가서 확인. ② 섹션 1개짜리 기존 케이스(회귀 테스트) 그대로 정상 동작. ③ 이름 있는 섹션(`.data mydata`/`.text mycode`) 문법은 파싱 자체가 안 됨(`assemble()=-2`) — 이건 오늘 고친 범위와 무관한 **별개의 기존 파서 이슈**로 보이며, 이번엔 안 건드림(이름 있는 섹션 자체를 아직 아무도 실제로 안 써봐서 지금까지 드러나지 않았던 것으로 추정).
    - **참고(오늘 범위 밖, 그냥 눈에 띈 것)**: 검증 중 DATA 섹션의 `data.sections[0].data`(실제 바이트)가 `bytes=0`으로 나옴 — `value: 4, 42` allocate가 섹션 개수/식별자 소속엔 이제 올바르게 반영되지만, 정작 그 섹션의 바이너리 내용 자체는 비어 있음. 별개 원인일 가능성이 높아 손대지 않았고, 기록만 해둠.

27. (2026-09-24) "merge sections구현해줘 기존 정렬 순서대로 진행하되 tesx/data/bss 구분하고(ROD는 read only data인데 아직 구현 안함) 이들 기준으로 section base 계산 후 identifier offset으로 계산해줘, assembler에서 identifier value를 binary offset으로 해놨는데 이를 section대비 상대 offset으로 바꾸어 linker에서 받으면 될 것 같아." — `linker::evaluate::functions::mergeSection` 구현. (중간에 사용자가 "그리고 파이프라인을 todo.txt보고 대충 확인해"라고 추가 요청 — 루트 `todo.txt`와 `digital computer 32/todo.txt` 둘 다 확인했으나 전부 CPU 하드웨어 파이프라인(ACU, LD/ST 버퍼, 페이징) 얘기뿐이고 어셈블러/링커 소프트웨어 파이프라인이나 메모리 레이아웃 컨벤션에 대한 내용은 전혀 없었음 — 그래서 base address는 관례대로 0, 순서는 TEXT→DATA→BSS로 진행.)
    - **assembler 쪽 사전 작업**: 사용자 말대로 라벨 값은 이미 섹션 상대 오프셋이었음(`calculateConst`의 `nData.value = data->sectionByte[section_id];`가 애초에 섹션별로 0부터 세는 카운터라서) — 그 자체는 안 고쳤음. 대신 `Identifier.section`(이름)만으로는 "이름 없는 섹션이 여러 개인 파일"에서 어느 섹션인지 구분이 안 되는 문제가 있어서(바로 25/26번에서 고친 버그와 완전히 같은 종류의 함정), `common/format.hpp`의 `_Identifier`에 `u32 sectionIndex`(→`Format::sections`의 인덱스, 상수면 `(u32)-1`) 필드를 추가하고 `getRaw`/`loadRaw`에 반영, `assembler.hpp`의 `formatData()`에서 `nId.sectionIndex`를 `coeff`의 section_id로 채움(이미 `data.sections`와 `format.sections`가 같은 순서/개수로 1:1 대응하는 걸 확인해뒀어서 그대로 재사용 가능).
    - **linker 쪽**: 사용자가 이미 스케치해둔 `_MapData`/`_SectionData`/`_FileData`/`_EvalData` 모양(중간에 여러 번 concurrent 수정됨)을 그대로 따라 채움:
      - Pass 1(파일별 통합): 같은 파일 안에서 이름+타입이 같은 섹션(`.text ... .text ...`처럼 소스에서 여러 번 나뉜 것)을 하나의 `SectionData`로 이어붙이고, 나중에 식별자 주소 계산에 쓸 "그 섹션 안에서 이 조각이 시작하는 로컬 오프셋"을 파일별로 기억해둠(`localBaseByFile`). `Format::mapper`(파일 안 절대 byteIndex)도 이 로컬 오프셋 기준으로 변환해 섹션별 `map`에 재배치. `expressions`는 그대로 복사(다음 단계인 "릴로케이션 적용"에서 쓸 재료로 남겨둠, 이번엔 안 풂).
      - Pass 2(배치): 각 파일의 섹션들을 원래 등장 순서대로 훑으면서 타입별(TEXT/DATA/BSS) 버킷에 이어붙임 — 버킷 자체의 최종 순서는 TEXT→DATA→BSS 고정(관례). ROD/OUTER 타입은 `continue`로 건너뜀(ROD 미구현). 각 파일 섹션의 최종 절대 베이스(`SectionData::sectionOffset`)와, 전체 배치 순서를 보여주는 `EvalData::texts/datas/bsses`(절대 start/end) 둘 다 채움.
      - Pass 3(식별자 해석): `finalAddress = sectionOffset(그 섹션) + localBase(그 파일 안에서의 조각 오프셋) + identifier.value`, 섹션이 없는(상수) 식별자는 `value` 그대로. 결과는 `FileData::definedLabel`(파일별 이름→최종 절대주소 맵)에 저장.
    - **부딪힌 버그와 수정**:
      - `namespace format`의 `using EvalData = evaluate::functions::EvalData;`가 `evaluate`보다 먼저 선언돼 있어 전방참조 컴파일 에러 → 아무 데서도 안 쓰이는 걸 확인하고 그냥 삭제.
      - `<unordered_map>` include 누락(`_EvalData::files` 등에서 이미 쓰고 있었는데도 최상단 include 목록에 없었음) → 추가.
      - **핵심 버그**: `FileData::sections`를 처음엔 섹션 **이름만**으로 키를 잡았는데(사용자가 원래 `std::map<std::string, SectionData>`로 선언), 테스트해보니 파일 안에 이름 없는(`""`) `.data`와 `.text`가 같이 있으면 둘이 같은 키로 충돌해서 **하나로 합쳐져 버림**(25/26번에서 고친 것과 완전히 같은 유형의 버그가 이번엔 linker 쪽에서 재발). `(타입, 이름)` 쌍을 키로 쓰는 `SectionKey = std::pair<u32, std::string>`로 바꿔서 해결.
    - **검증**: 진짜 저장소 파일은 그대로 두고 `/tmp` 스크래치 사본에서 `link()`(별개의, 여전히 미완성인 함수)만 주석 처리한 뒷채 격리 빌드 — 가상의 object 2개(`file1.o`: 이름 없는 `.data`(4바이트)+`.text`(8바이트, 라벨 2개), `file2.o`: 이름 없는 `.text`(4바이트, 라벨 1개))를 직접 만들어 `mergeSection` 실행 → `textBase=0, dataBase=12`(file1.text 8B + file2.text 4B), `totalBin`이 TEXT 블록(file1+file2 순서대로) 다음 DATA 블록 순으로 정확히 이어짐, 식별자 4개(`start=0, loop=4, main=8, value=12`) 전부 기대값과 일치 확인.
    - **오늘 범위 밖으로 남겨둔 것**: 릴로케이션 실제 적용(각 섹션의 `map`/`expressions`를 돌면서 `stackMachine`/`castOperation`으로 값을 계산해 `calcBin`에 패치하는 단계) — `mergeSection`은 주소만 계산하고 아직 아무 바이트도 패치 안 함. `int link(...)` 자체도 여전히 문법 에러투성이 WIP라 손 안 댐. 이름 있는 섹션 문법 파싱 자체가 안 되는 별개 이슈(25번에서 발견)도 여전히 미해결.

28. (2026-09-24) "현재 evaldata변경점 적용해줘, 같은 이름의 section은 따로 만든 vector로 합치고 새로만든 nameMap이용해서 원래 순서인 vector의 index를 참조하게해줘, iterator는 일단 위 주석에 어떤역할인지 명시하고 그 형식을 깊이에 따라 [it, si, ti, qi, pi, hi ... etc]로 고정해줘" — 사용자가 27번 이후 `EvalData`/`FileData`를 다시 concurrent로 바꿔둔 상태(`FileData::sections`가 `map<SectionKey,...>`→`vector<SectionData>`+`nameMap`으로, `EvalData::files`가 `unordered_map`→`vector`로, `EvalData::sections`가 새로 추가됨)에 맞춰 `mergeSection` 전체를 다시 씀:
    - **5단계 구조로 재작성**: (1) 파일별로 같은 (type,name) 섹션을 `FileData::sections`(vector, 첫 등장 순서 유지) + `nameMap`(type+name → 인덱스)으로 통합 + mapper 재배치 + `localBase`(파일별×원본섹션인덱스: 통합된 섹션 안에서 그 조각이 시작하는 위치) 기록 (2) 그 위에서 한 번 더 — 모든 파일에 걸쳐 같은 (type,name)을 `EvalData::sections`(전역 병합, `SectionData::sectionOffset`엔 일단 "병합 블롭 안에서의 상대 위치"만 임시로 저장) (3) `EvalData::sections`를 순서대로(맵 키가 (type,name)이라 자동으로 TEXT→DATA→BSS 그룹핑됨) text/data/bss 버킷에 배치하고 `texts/datas/bsses` 배치 원장 + 절대 base 계산 (4) 각 파일 섹션의 임시 오프셋을 방금 구한 전역 절대 base로 패치 (5) 식별자 주소 = 파일 섹션의 절대 base + `localBase` + 식별자 자체 값.
    - **iterator 네이밍 컨벤션 적용**: `mergeSection` 안에서 깊이 1=`it`, 2=`si`, 3=`ti`, 4=`qi` 고정(더 깊어지면 `pi`,`hi`...) — 어떤 컨테이너를 도는지와 무관하게 중첩 깊이로만 이름을 정함(서로 다른 블록에서 같은 깊이면 같은 이름 재사용, C++ for문 스코프가 분리돼 있어 충돌 없음). 함수 맨 위에 이 규칙 자체를 설명하는 주석 추가. 위치 기반 배열(`localBase`)처럼 진짜 인덱스가 필요한 곳은 이터레이터에서 `it - container.cbegin()`으로 유도해서 씀(별도 카운터 변수 안 둠).
    - **다시 부딪힌 핵심 버그**: `FileData::nameMap`이 사용자 선언대로 `unordered_map<std::string, u32>`(이름만 키)라서, 파일 안에 이름 없는(`""`) `.data`와 `.text`가 같이 있으면 **또** 충돌해서 하나로 합쳐짐(27번에서 `SectionKey`로 고쳤던 것과 완전히 같은 문제가 이번 재작성에서 재발 — 테스트로 실제로 재현: TEXT/DATA 바이트 개수가 서로 뒤바뀜). `nameMap`의 선언 타입(`string` 키)은 그대로 두고, 대신 저장하는 **키 내용**을 `nameMapKey(type, name)`(=`"<type번호>:<이름>"` 합성 문자열)로 바꿔서 해결 — 사용자가 선언한 필드 타입 자체는 안 건드리고 키 구성 방식만 고침.
    - **검증**: `/tmp` 스크래치에서 27번과 동일한 가상 object 2개로 재테스트 — `textBase=0, dataBase=12`, `totalBin`이 TEXT(file1 8B+file2 4B)→DATA(file1 4B) 순으로 정확, `EvalData::sections`에 TEXT/DATA 각각 정확한 크기로 병합됨 확인, 식별자 4개 전부 기대값과 일치. 실제 저장소 파일도 `-fsyntax-only`로 재확인 — 여전히 별개인 `int link(...)`(미완성 WIP) 이외엔 에러 없음.

29. (2026-09-24) "추가한 convertData함수에서 files로 캐스팅, ... mergeByName에서 ... evalData의 외부 sections vector에 합쳐 저장, 이후 merge by type에서 type으로 합침. 적용해줘." — 사용자가 28번 이후 다시 concurrent로 `convertData`/`mergeByName`(빈 스텁) + `mergeByType`(예전 `mergeSection` 그대로, 근데 시그니처에서 `objects` 파라미터가 빠져서 이제 컴파일 안 되는 상태)로 쪼개둔 걸 요청대로 완성:
    - **파이프라인 3분할**: `convertData(data, objects)` — objects→`EvalData::files` 변환(파일별 (type,name) 섹션 통합 + mapper 재배치 + 식별자를 `PendingLabel`(섹션 로컬 오프셋, 아직 최종 주소 아님)로 스테이징). `mergeByName(data)` — 전 파일에 걸쳐 같은 (type,name)을 `EvalData::sections`(새로 **vector**로 바뀜, `EvalData::nameMap`으로 조회)에 합침, 각 파일 섹션의 `sectionOffset`엔 일단 "합쳐진 블롭 안에서의 상대 위치"만 임시 저장. `mergeByType(data)` — `EvalData::sections`를 타입별(TEXT→DATA→BSS)로 배치 + 절대 base 계산 + 각 파일 섹션 오프셋을 절대값으로 패치 + `PendingLabel`을 최종 절대주소로 풀어 `definedLabel`에 기록.
    - **키 mangling**: `sectionKey()`/`nameMapKey()` 헬퍼 함수를 없애고, 요청한 대로 호출부마다 직접 `"#" + std::to_string(type) + "_" + name` 문자열을 만들어 씀(`#`은 식별자에 못 쓰는 문자라 진짜 이름과 절대 안 겹침). `EvalData::sections`가 `map<SectionKey,...>`→`vector<SectionData>`+`EvalData::nameMap`(unordered_map<string,u32>)으로 바뀌면서, `FileData`와 완전히 대칭인 구조가 됨(파일 레벨 통합과 파일 간 병합이 같은 패턴을 재사용).
    - **`asU32`/`asS32` 제거**: `stackMachine::function()`에서 전부 `(u32)`/`(s32)` 직접 캐스트로 교체, 헬퍼 함수 삭제.
    - **iterator 네이밍 구분**: for문을 실제로 도는 이터레이터만 `it`(깊이1)/`si`(2)/`ti`(3)/`qi`(4) 유지, `.find()` 결과처럼 루프를 돌지 않는 건 용도에 맞게 개명(`slotIt`=이 (type,name)이 이미 등록됐는지 찾는 슬롯, `mergedIt`=병합된 전역 섹션을 찾는 결과).
    - **식별자 해석 재설계**: 예전엔 `mergeSection` 한 함수가 `objects`를 직접 참조하면서 식별자를 바로 최종 주소로 풀었는데, `mergeByType`엔 이제 `objects`가 없어서 그 자리에서 못 풀게 됐음 → `convertData` 단계에서 식별자를 완전히 못 풀린 채로 `FileData::pendingLabels`(식별자 이름 → `PendingLabel{섹션인덱스, 그 섹션 안에서의 로컬 오프셋}`)에 스테이징해두고, `mergeByType`이 모든 섹션의 최종 절대 base를 알게 된 시점에 한 번에 풀어서 `definedLabel`에 채우는 방식으로 재설계(사용자가 이번 메시지에서 식별자 얘기는 안 했지만, 파이프라인이 끊기지 않게 하려면 필요해서 판단해서 추가함 — 이 부분은 확인 필요할 수 있음).
    - **검증**: `/tmp` 스크래치에서 이전과 동일한 가상 object 2개로 `convertData`→`mergeByName`→`mergeByType` 순서로 실행 → `textBase=0, dataBase=12`, `totalBin` 순서 정확, `EvalData::sections`에 TEXT/DATA 각각 올바른 크기로 병합, 식별자 4개 전부 기대값과 일치. 실제 저장소 파일도 `-fsyntax-only` 재확인 — 여전히 별개인 `int link(...)`(미완성 WIP, 손 안 댐) 이외엔 에러 없음.

30. (2026-09-24) "castOeration 이용해서 마지막으로 exprMapping해주고 ... link함수 대충 완성 후(그냥 eval, no log info?, binary ret) main에서 파일 받아서 link호출 한 뒤 build&test해줘" — 링커 파이프라인의 마지막 단계와 실제 진입점을 완성하고 처음으로 진짜 end-to-end 빌드/테스트를 돌림. (사용자가 이미 `evaluate(data, objects)` 드라이버를 `convertData→mergeByName→mergeByType→mapExpr` 순서로 만들어뒀고, `mapExpr`는 빈 스텁이었음 — 이게 요청한 "exprMapping".)
    - **`FileData`에 `usedIdentifier` 추가**: `mapExpr`가 `castOperation`을 쓰려면 파일별 식별자 이름 테이블이 필요한데 안 옮겨져 있었음 → `convertData`에서 `obj.usedIdentifier`를 그대로 복사해오도록 추가.
    - **`mapExpr` 구현**: 파일별로 (1) 그 파일의 `usedIdentifier` 각각을 전체 파일들의 `definedLabel`에서 찾아 전역 최종 주소로 미리 해석해두고(파일 간 심볼 참조 지원 — 다만 현재 어셈블러엔 `.extern` 같은 게 없어서 "이 파일에 없는 심볼"은 애초에 조립 단계에서 막힘, 그래도 구조상 지원됨) (2) 그 파일의 각 섹션(`SectionData::map`)에 기록된 relocation마다 `castOperation`(RPN → `stackMachine::Element`, IDENTIFIER는 위에서 구한 주소로 치환) → `stackMachine::runOperation`으로 최종 값 계산 → `data.totalBin`의 해당 절대 위치에 비트 단위로 패치(`reloc.offset`/`reloc.size`, 필드가 여러 바이트에 걸치면 앞쪽(더 낮은 주소) 바이트로 계속 이어감 — 어셈블러의 `registerReplace`가 쓰던 것과 같은 빅엔디안 LSB-바이트 컨벤션).
    - **`link()` 완성**: 예전 깨진 `int link(std::vector<u8>&)`(오브젝트 1개짜리, 미완성) 대신 `bool link(std::vector<u8>& outBinary, const std::vector<std::pair<std::string, std::vector<u8>>>& rawObjects)`로 재작성 — 각 raw object를 `Format::loadRaw`로 역직렬화해서 `objects` 리스트를 만들고, `evaluate::functions::evaluate()`(4단계 파이프라인) 한 번 돌려서 `data.totalBin`을 그대로 반환. 로그 없이 평가만(요청한 "그냥 eval, no log info, binary ret" 그대로).
    - **`main.cpp`**: `<출력 파일> <입력 오브젝트 파일...>` 인자 받아서 각 파일 읽고 `link()` 호출, 성공하면 결과를 출력 파일에 씀(어셈블러 `main.cpp`와 같은 스타일).
    - **첫 진짜 end-to-end 빌드+테스트**: `build-asm.cmd`/`build-lnk.cmd` 둘 다 재빌드 성공. `.text`만 있는 소스 2개를 각각 `assembler.exe`로 따로 조립(각 8바이트, 라벨 하나씩 참조하는 상대 점프 포함) → `linker.exe`로 두 오브젝트를 합침 → 결과 16바이트 바이너리를 직접 헥스덤프로 검증: obj1의 `loop` 참조는 그대로 0(자기 앞부분이라 안 밀림), **obj2의 `start` 참조는 원래 로컬 오프셋 0에서 최종 주소 8로 정확히 리베이스됨**(obj1의 8바이트가 앞에 붙었으므로) — 크로스 파일 재배치가 정확히 동작함을 직접 확인. `.data`+`.text` 단일 파일 케이스도 추가로 테스트(`ld.zero gen[0],value`의 임메 필드가 TEXT(8B) 다음에 오는 DATA 섹션의 최종 주소 8로 정확히 패치됨) — 둘 다 통과.
    - **여전히 범위 밖으로 남은 것**: `.extern`/외부 심볼 선언 문법이 없어서 진짜 "다른 파일에서 정의된 심볼을 이 파일에서 참조" 케이스는 애초에 어셈블 단계에서 막힘(오늘 안 건드림) — 그래서 이번 end-to-end 테스트는 "각자 완결된 여러 파일을 합쳐서 재배치"까지만 검증했고, 진짜 크로스파일 심볼 링킹까지는 검증 못 함(구조상 `mapExpr`가 전 파일의 `definedLabel`을 다 뒤지도록 짜여있어서 지원은 되지만). 이름 있는 섹션 파싱 불가 이슈(25번)도 여전히 미해결.

31. (2026-09-24) "assembler에서 fileName으로 label Mangling 추가해줘 (binary기준 이름)" — 30번 테스트에서 확인했듯 서로 다른 파일이 같은 라벨 이름(`loop` 등)을 쓰면 링커의 전역 심볼 테이블에서 충돌하는 문제가 있어서, 어셈블러가 라벨 이름에 파일명을 섞어 넣도록(mangling) 함:
    - **처음엔 `calculateConst`(평가 로직) 안에서 처리하려 했으나, 작업 중 사용자가 "굳이 처리에서 바꾸지 말고 format시 바꾸게 해줘"라고 정정** → `calculateConst`의 `label`/`identifier` case는 원래대로 raw 이름 그대로 쓰도록 되돌리고, 대신 `format::formatData()`(직렬화 시점)에서 `definedIdentifier[].name`과 `usedIdentifier[]`(그리고 그걸 가리키는 `Operation.value` 인덱스) 양쪽에 똑같이 mangling을 적용 — `idenifierValue`/`expressionValue`는 평가 내내 raw 이름으로 유지되고, Format으로 굳어지는 마지막 순간에만 이름이 바뀜.
    - **mangling 형식**: `mangleLabel(fileName, label)` = `fileName.empty() ? label : fileName + "#" + label` (`#`는 식별자에 못 쓰는 문자라 안 겹침 — 이미 매크로 고유화(`identifier_unique`, `#define`류)에서도 `"#"+숫자` 형태로 같은 관례를 쓰고 있었음). `EvalData::fileName`(assemble()이 채워줌)을 `formatData()`가 읽어서 씀.
    - **"binary기준 이름"**: `fileName`은 입력 소스 파일명이 아니라 **출력 바이너리 파일명**(디렉토리/확장자 제거한 base name)을 씀 — `assembler/main.cpp`에서 `outputFile`(argv[2])을 파싱해 `outputBase`를 만들고 `assemble(buff, binary, dump, outputBase)`로 전달. `assemble()`에 `fileName` 매개변수(기본값 `""`, 기존 호출부 호환) 추가.
    - **검증**: 같은 이름(`loop`)의 라벨을 각자 정의하는 파일 2개를 조립 → 바이너리 안에 `mobj1#loop` 형태로 mangled된 이름이 실제로 박혀있음을 확인 → 링크했더니 두 파일의 `loop`가 서로 안 섞이고 각자 자기 파일 안의 올바른 최종 주소로 정확히 리졸브됨(obj1의 loop→0, obj2의 loop→8) — mangling 이전이었다면 링커의 이름 기반 전역 검색이 첫 번째로 찾은 파일의 값으로 둘 다 잘못 리졸브했을 상황.

32. (2026-09-24) "linker의 section Mangling 도 <type>#<name>으로 바꾸고 빌드 된 툴 \digital computer 32\tool에 저장해줘, 기존 dasm은 dasm-old로 새로운 툴은 dasm/dlnk로 작명 (작명 후 table삭제 후 재실행, 꼬임 방지)":
    - **섹션 mangling 형식 통일**: 링커의 섹션 키 mangling을 `"#" + type + "_" + name`에서 어셈블러의 라벨 mangling과 같은 형태인 `type + "#" + name`으로 통일(5곳: `convertData` 3곳, `mergeByName`/`mergeByType` 각 1곳 + 관련 주석). 순수 문자열 포맷 변경이라 의미는 그대로 — 재빌드 후 2파일 링크 회귀 테스트(30/31번과 동일 케이스) 통과 재확인.
    - **툴 배포**: `digital computer 32/tools/bin/`(레포에 이미 있던 옛 어셈블러 배포 위치 — CLAUDE.md에도 `tools/bin/dasm.exe`로 문서화돼 있던 곳)에 있던 기존 `dasm.exe`+`dasm_table.bin`을 `dasm-old.exe`+`dasm-old_table.bin`으로 이름 바꾸고, 새로 빌드한 `assembler.exe`/`linker.exe`(`src/assembler-linker/build/`)를 각각 `dasm.exe`/`dlnk.exe`로 복사.
    - **테이블 꼬임 방지**: 이름 바꾸기 전 있던 `dasm_table.bin`(옛 버전 문법으로 빌드된 캐시)이 새 `dasm.exe`한테 그대로 남아있으면 버전 번호만 맞으면 통째로 재사용돼버려서(과거 25번에서 실제로 겪은 문제) 새 문법과 안 맞는 stale 테이블을 로드할 위험이 있었음 → `dasm_table.bin`/`dlnk_table.bin`을 명시적으로 지운 뒤 `dasm.exe`/`dlnk.exe`를 인자 없이 한 번씩 실행해서 강제로 새 테이블을 생성시킴(`dasm.exe`는 429848바이트짜리 새 `dasm_table.bin` 정상 생성 확인; `dlnk.exe`는 텍스트 문법이 없어 애초에 테이블 캐시 자체가 없는 게 정상).
    - **최종 검증**: 배포된 `digital computer 32/tools/bin/dasm.exe` + `dlnk.exe`로 처음부터 끝까지(2개 파일 조립 → 링크) 다시 돌려서 이전과 동일하게 올바른 결과(리베이스된 주소 `0008`) 나오는 것까지 확인.

## 작업 방식 관련 주의사항

- **빌드/테스트를 절대 자동으로 수행하지 말 것** — 사용자가 "아직 완성본 아니니까 빌드나 테스트까지는 하지 말고"라고 명시함. 명시적 요청이 있을 때만 수행.
- 이 파일(`assembler.hpp`)은 사용자가 대화 밖에서 직접 동시 편집하는 경우가 있었음(과거 Edit 충돌 이력) — 편집 전에는 항상 최신 상태를 다시 Read할 것.
