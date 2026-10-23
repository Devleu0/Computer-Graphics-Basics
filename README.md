# 실시간 그래픽스와 셰이더 학습 자료

이 저장소는 Unity URP에서 NPR(툰) 셰이더를 직접 만들고 실행해 보는 것을 대표 실습으로 삼는 한국어 그래픽스 학습 저장소입니다. 셰이더 기초와 HLSL/DirectX 자료가 핵심 실습을 뒷받침하며, DirectX 11과 Win32는 별도의 학습 경로로 제공합니다.

## 시작하기

### 대표 결과물: NPR 데모 실행

1. Unity 2022.3 LTS로 [`Unity shader/ShaderTestProject`](./Unity%20shader/ShaderTestProject/)를 엽니다.
2. 프로젝트가 임포트를 마치면 `Assets/Scenes/NPRDemo.unity`를 열고 Play를 누릅니다.
3. `NPR Toon` 머티리얼이 적용된 구와 캡슐에서 단계형 툰 명암, 외곽선, 메인 라이트 그림자를 확인합니다.
4. 따뜻한 램프와 차가운 램프 전환은 Hierarchy의 `NPR Ramp Blend Controller`가 참조하는 Directional Light의 색을 바꾸어 확인합니다.

데모 생성 및 수정 안내는 [NPR 실습 README](./Unity%20shader/NPR/README.md), 현재 우선순위와 검증 상태는 [NPR 중심 개선 계획](./docs/LEARNING_MATERIALS_PLAN.md)을 참고하세요. 실제 Unity 검증을 마치기 전에는 에디터 실행 결과가 보장된다고 가정하지 마세요.

### 다른 학습 경로

대표 실습 외의 경로는 목표에 맞춰 선택할 수 있습니다.

1. 관심 경로를 고릅니다. 각 경로는 정해진 주차나 학습 기간이 없는 자기 주도형 모듈입니다.
2. 경로 문서에서 선수 지식, 환경, 목표를 확인합니다.
3. 개념을 읽은 뒤 코드를 직접 실행하거나 프로젝트에 적용하고, 문서의 연습 과제와 성공 기준으로 이해도를 점검합니다.
4. 셰이더 코드의 호스트 측 설정이 필요하면 [실습 환경과 공통 규약](./Shader%20Learning/0.%20실습%20환경과%20공통%20규약.md)도 확인합니다.

문서의 검증 범위와 알려진 제한은 [저장소 검토 결과](./docs/REVIEW.md)를 참고하세요. 코드가 실제 컴파일·실행 환경에서 확인되었는지 각 문서의 안내를 먼저 읽어 주세요.

## 어떤 경로를 선택할까요?

| 목표 | 추천 순서 | 선수 지식 및 결과 |
|---|---|---|
| NPR/툰 셰이더 결과물 만들기 | [NPR 데모 실행](./Unity%20shader/NPR/README.md) → [Unity 셰이더 학습 경로](./Unity%20shader/학습-경로.md) | Unity 2022.3 / URP 14. 램프 조명, 그림자, 외곽선이 통합된 결과를 실행하고 수정 |
| Unity에서 셰이더를 처음 만들기 | [Unity 셰이더 학습 경로](./Unity%20shader/학습-경로.md) → [ShaderLab 기초](./Unity%20shader/셰이더-기초-ShaderLab-HLSL-좌표-텍스처.md) → [조명·블렌딩·정점 애니메이션](./Unity%20shader/조명-블렌딩-정점-애니메이션.md) → [노멀 매핑·프레넬·디졸브](./Unity%20shader/노멀-매핑-프레넬-디졸브.md) → [후처리·컴퓨트·최적화](./Unity%20shader/후처리-컴퓨트-최적화-프로젝트.md) | Unity 프로젝트 사용 경험 권장. 머티리얼과 화면 효과를 직접 제작 |
| HLSL과 GPU 파이프라인 이해하기 | [HLSL/DirectX 학습 경로](./Shader%20Learning/Basics/학습-경로.md) → [실습 환경과 공통 규약](./Shader%20Learning/0.%20실습%20환경과%20공통%20규약.md) → [기초](./Shader%20Learning/Basics/기초-파이프라인-조명-텍스처.md) → [중급](./Shader%20Learning/Basics/중급-재질-노멀-그림자-반사.md) → [고급](./Shader%20Learning/Basics/고급-후처리-테셀레이션-컴퓨트.md) | C++ 기초와 벡터·행렬 이해 권장. 셰이더와 호스트 프로그램의 연결을 학습 |
| DirectX 11 렌더링 기반 다지기 | [DirectX 11 학습 커리큘럼](./Directx11/DirectX%2011%20학습%20커리큘럼.md) → [기초](./Directx11/DirectX%2011%201단계.md) → [중급](./Directx11/DirectX%2011%202단계%20중급.md) → [고급](./Directx11/DirectX%2011%203단계%20고급.md) | C++ 및 Win32 기본 권장. 초기화부터 렌더링 기능까지 단계별 구현 |
| Windows 데스크톱 API 익히기 | [Win32 학습 경로](./Win32/0.%20Win32%20학습%20커리큘럼.md) → [예제 실행 가이드](./Win32/Win%20API%20예제%20테스트%20환경%20활용%20가이드.md) | C/C++ 기초 권장. 윈도우, 입력, GDI, 리소스, 시스템 API 학습 |

## 자료 찾아보기

### Unity 셰이더

- [Unity 셰이더 학습 경로](./Unity%20shader/학습-경로.md)
- [ShaderLab 기초와 좌표·텍스처](./Unity%20shader/셰이더-기초-ShaderLab-HLSL-좌표-텍스처.md)
- [조명·블렌딩·정점 애니메이션](./Unity%20shader/조명-블렌딩-정점-애니메이션.md)
- [노멀 매핑·프레넬·디졸브](./Unity%20shader/노멀-매핑-프레넬-디졸브.md)
- [후처리·컴퓨트·최적화·프로젝트](./Unity%20shader/후처리-컴퓨트-최적화-프로젝트.md)
- [고급 셰이더 학습 로드맵](./Unity%20shader/고급%20셰이더%20기술%20심화%20학습%20로드맵.md)
- [NPR 셰이더 실습 프로젝트](./Unity%20shader/NPR/README.md)

### HLSL / DirectX 셰이더

- [학습 경로](./Shader%20Learning/Basics/학습-경로.md)
- [실습 환경과 공통 규약](./Shader%20Learning/0.%20실습%20환경과%20공통%20규약.md)
- [기초: 파이프라인·조명·텍스처](./Shader%20Learning/Basics/기초-파이프라인-조명-텍스처.md)
- [중급: 재질·노멀·그림자·반사](./Shader%20Learning/Basics/중급-재질-노멀-그림자-반사.md)
- [고급: 후처리·테셀레이션·컴퓨트](./Shader%20Learning/Basics/고급-후처리-테셀레이션-컴퓨트.md)
- [PBR 셰이더](./Shader%20Learning/Advanced/PBR-실사-렌더링-셰이더.md)
- [Deferred Shading](./Shader%20Learning/Advanced/Deferred-Shading-다중-광원.md)
- [테셀레이션 지형](./Shader%20Learning/Advanced/테셀레이션-동적-지형-렌더링.md)

### 그래픽스 기초와 API 참고

- [컴퓨터 그래픽스 학습 로드맵](./컴퓨터%20그래픽스%20학습%20로드맵.md)
- [Unity 셰이더 경험자를 위한 실제 API 개요](./UnityVSAPI.md)
- [DirectX 11 학습 경로](./Directx11/DirectX%2011%20학습%20커리큘럼.md)
- [Win32 학습 경로](./Win32/0.%20Win32%20학습%20커리큘럼.md)

## 학습 자료를 활용하는 방법

- **주차나 기간에 얽매이지 마세요.** 모듈은 개념 의존 관계를 안내하는 순서이며, 필요한 시간을 정하거나 진도를 재촉하지 않습니다.
- **먼저 목표를 확인하세요.** 각 경로에서 다루는 주제와 필요한 배경 지식을 확인한 다음, 모르는 선행 개념은 그래픽스 로드맵이나 공통 규약 문서로 돌아가 보충하세요.
- **코드를 직접 바꿔 보세요.** 색, 빛 방향, 표면 거칠기, 텍스처 좌표 등 입력을 바꾸고 결과가 달라지는 이유를 기록하면 단순 복사보다 학습 효과가 큽니다.
- **학습 결과를 남기세요.** 각 모듈을 마칠 때 직접 만든 결과물, 바꿔 본 값, 해결한 문제를 짧게 기록하고, 다음 모듈의 선수 지식으로 활용하세요.
- **완성형과 발췌 코드를 구분하세요.** 발췌 코드는 호스트 코드, 상수 버퍼 또는 앞선 셰이더 정의가 필요할 수 있습니다. 문서의 실행 조건을 먼저 확인하세요.
- **검증 상태를 확인하세요.** 모든 예제가 모든 환경에서 실행된다고 가정하지 않습니다. 버전 차이와 미검증 항목은 문서 및 [검토 기록](./docs/REVIEW.md)에 표시합니다.

## 참고 도구

- 개발: Visual Studio, Unity
- 그래픽스 디버깅: PIX for Windows, RenderDoc, Unity Frame Debugger
- 이론 및 예제: [Real-Time Rendering](https://www.realtimerendering.com/), [Learn OpenGL](https://learnopengl.com/), [Catlike Coding](https://catlikecoding.com/), [Microsoft DirectX Graphics Samples](https://github.com/microsoft/DirectX-Graphics-Samples)
