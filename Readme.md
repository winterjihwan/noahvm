<div align="center"

### Noah VM

</div>

## 📝 소개

Noah VM은 가상머신이 어떻게 동작하는지 깊이 이해해보고 싶어서 직접 설계하고 구현한 프로젝트. 컴파일러, 레지스터 기반 가상머신, 렉서, 파서, IR 생성 등 저수준 시스템 프로그래밍 요소를 포함하고 있으며, C 언어로 개발했다. 토이 언어를 컴파일하고 실행하는 환경을 만들면서 실제 언어가 어떻게 동작하는지 바닥부터 경험하고 이해하는 데 집중했다.

<br />

## 🌊 흐름

1. **소스 코드 입력**  
   `main.c` → 파일에서 코드 로드

2. **Lexing**  
   `lexer.c` → 토큰화 수행

3. **Parsing & 컴파일**  
   `compiler.c` → 토큰을 IR(중간 표현)으로 변환

4. **정적 분석 (옵션)**  
   `analyzer.c` → Dead Store Elimination(DSE) 실행

5. **프로그램 로딩**  
   `vm.c` → IR을 VM 메모리에 로딩

6. **실행**  
   `vm.c` → 가상머신 스택/레지스터 기반으로 IR 실행

7. **후처리**  
   `vm_stack_dump()` → 스택 출력
   `vm_destruct()` → 리소스 해제

<br />

## ⚙ 기술

<img src="./images/c.png" alt="C Language" width="50"/>  
<img src="./images/llvm.png" alt="LLDB" width="50"/>

<br />

## 🐛 트러블슈팅

- ABA 문제

<br />

## 📚 참고자료

**Engineering a Compiler** (3rd ed.) by Keith D. Cooper & Linda Torczon

[bm - tsoding](https://github.com/tsoding/bm)

<br />

## 📦 사용법

```bash
make main
./main ./examples/scope
# 또는 ./examples/ 안의 다른 예제 사용 가능
```

## 💡 느낀점

프로그래밍 언어가 어떻게 동작하는지 큰 그림을 이해하는 게 정말 재미있었다. 직접 컴파일러랑 가상머신을 설계하고 만들어보면서 언어 내부 구조나 메모리 동작 방식을 제대로 체감할 수 있었다. 이 과정을 거치면서 하스켈 같은 함수형 언어나 러스트의 오너십 모델이 실제로는 어떻게 구현됐을지 더 궁금해졌다.

앞으로는 내 컴퓨터 ISA인 x86을 기반으로 저수준 컴파일러를 직접 만들어보고 싶다. 레지스터 할당, SSA 같은 최적화 기법도 직접 구현해보고, 궁극적으로는 병렬 컴파일러에 대해서도 알아보고 싶다.
