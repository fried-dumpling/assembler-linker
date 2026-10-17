# TODO — assembler-linker (2026-10-11 기준)

## 완료됨 (커밋 `5cd5a7e` 이후 작업, 2026-10-11 커밋에 포함)

### 어셈블러 (dasm)
- `.bss` 크기를 오브젝트에 기록 (`Section::size`). 여러 파일의 bss가 같은 주소에 겹치던 버그가 해결됨.
  - ⚠️ 오브젝트 포맷이 바뀌었으므로 예전 `.o`는 다시 어셈블해야 함.
- `global` / `extern` 파싱과 의미 구현.
  - global 이름은 접두사 없이 내보내고, 나머지 이름은 `파일명#이름`으로 mangle함.
  - extern 이름은 항상 재배치 대상.
  - 오류 추가: `GLOBAL_UNDEFINED`, `EXTERN_DEFINED`.
- BUGS.md의 3건 수정.
  - 인자 1개짜리 매크로 동작 수정, 인자 개수 오류와 괄호 생략 오류 추가.
  - `;`만 있는 줄도 주석으로 처리 (테이블 version 407).
  - 실패하면 출력 파일을 만들지 않음.
- 그 전에 한 작업:
  - eval 오류에 줄/토큰/오류 이름 표시, `srcToken`을 값으로 복사.
  - `DataValue::length` 수정.
  - `equ`와 `$` 평가 연결.
  - `#ifdef` / `#ifndef` 추가, 전처리 오류 보고.
  - UTF-8 주석에서 생기던 UB 수정.

### 링커 (dlnk)
- `Format::loadRaw`가 오브젝트를 검증함. 깨진 오브젝트는 segfault 대신 `invalid object file` 오류.
- `undefined symbol` / `duplicate symbol` 오류 보고.
- bss를 크기 기준으로 배치.

### 문서 / 배포
- 문서:
  - `SYNTAX.md` 작성.
  - `digital computer 32/ISA.md` 작성.
  - `CLAUDE.md` 56~58번 항목 추가.
- 배포:
  - `computer 32 (new)/bin`과 `digital computer 32/tools`의 dasm/dlnk 교체, table.bin 재생성.
  - tools에 소스, SYNTAX.md, BUGS.md 반영.
- 회귀 테스트:
  - `test/program` 29개 중 25개 성공, 배포본과 바이트 단위로 동일.
  - BUGS 재현 3건 해결 확인.
  - float 예제 어셈블+링크 성공.

## 남은 일

### 커밋
- [x] 이 저장소: 2026-10-11 커밋 (저장소 루트가 `src/`로 바뀐 뒤 첫 커밋).
- [ ] `digital computer 32` 저장소: ISA.md와 tools가 커밋되지 않음 (2026-10-11 tools 재배포분 포함). 다른 세션(floatingpoint)의 변경도 섞여 있으니 범위를 나눠서 커밋할 것.
- [ ] 추후 이 저장소(`src/`)를 `digital computer 32` 저장소로 git 통합.
  - 지금은 소스를 `digital computer 32/tools` 미러에 수동으로 복사하고 있음. 통합하면 복사가 필요 없음.
  - 이 저장소의 커밋 히스토리를 보존하는 방식(subtree 등)으로 합칠 것.

### 목표
- [x] linker: bss padding 구현. (`-padbss` 플래그, `linker::bssPadding`)
- [x] assembler: 문자열 추가
- [x] assembler: 리스트 추가
- [x] assembler / linker: `-o <출력 파일>` 플래그 추가 (현재는 위치 인자로 출력 파일을 받음)
- [x] 터미널 입출력 통일 (`CLI.md`): `iotools::Terminal`/`errPosGenerator`, dasm·dlnk·hexdump 적용, GCC 형식 위치 오류, 종료 코드 0/1/2.
- [ ] `-align [byte]` 플래그로 word 정렬 (값 생략 시 4바이트).
  - linker: 섹션 조각과 text/data/bss 그룹의 시작 주소를 맞추고, 그 사이는 0으로 채움.
  - assembler: 크기가 4의 배수가 아닌 allocate 뒤의 데이터/라벨을 맞춤 (아래 "데이터 배치 한계" 참고).
  - 값은 2의 거듭제곱만 허용. 값을 생략할 수 있는 플래그라 인자 파싱 규칙을 정해야 함 (터미널 입출력 통일안과 같이).

### 알려진 미해결 문제
- [x] 파서(LALR) 실패: branch, factorial_norec, prime_old. 사용자 판단으로 보류 중.
- [x] 소스 쪽 오류 (어셈블러 문제 아님):
  - bcdtodec: 소스에 없는 라벨 참조.
  - texteditor: `#define LSET 2`는 `#macro`로 써야 함.
- [ ] 데이터 배치 한계:
  - 4바이트보다 큰 allocate에 재배치 값이 들어가면 첫 워드만 패치됨.
  - 크기가 4의 배수가 아니면 뒤 데이터가 정렬되지 않음.
- [ ] 중첩 `#include`의 경로 기준이 항상 최초 입력 파일의 디렉터리 (`PreprocData::fileName`이 갱신되지 않음).
- [ ] `gen[n]` 인덱스 범위 검사 없음 (25 이상이면 특수 레지스터 id와 겹침).
- [ ] global/extern 오류 메시지의 토큰이 이름이 아니라 키워드로 표시됨.
- [x] ISA 시트(xlsx)와 다른 부분: `pctl`/`getabs`/`cache`(flush·inval)/`atom`/`intervec` 시트에 맞춤 (2026-10-17).
- [ ] ISA 남은 확인 (ISA.md 6절): md 비트 방향(flush=1 가정), `rol`/`ror` 방향, `jp`/`jpi`/`break` 이름.
- [ ] 문자열 리터럴(`"..."`)은 렉서만 인식하고 문법에 연결되지 않음.
- [ ] `digital computer 32` 쪽 CLAUDE.md의 도구 설명이 예전 상태.
