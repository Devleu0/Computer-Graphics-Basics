# Unity URP NPR 셰이더 실습

이 저장소의 목표는 **Unity URP에서 NPR(비실사 렌더링) 셰이더를 이해하고, 실행하고, 직접 확장하는 것**입니다. 대표 결과물을 먼저 실행한 뒤 필요한 셰이더 개념을 학습하고, NPR 효과를 재현·변형하는 단일 권장 경로를 제공합니다. 다른 API 학습 자료는 선택 참고용이며, Windows API 자료는 독립 보관합니다.

## 권장 학습 경로

### 1. NPR 데모 실행하기

Unity 2022.3 LTS로 [`Projects/NPR Unity/ShaderTestProject`](./Projects/NPR%20Unity/ShaderTestProject/)를 열고 `Assets/Scenes/NPRDemo.unity`를 재생합니다. 툰 명암, 외곽선, 메인 라이트 그림자를 관찰하고, Directional Light 색을 바꾸어 램프 혼합을 확인합니다.

실행 방법과 효과별 실습은 [NPR 실습 안내](./Learning/Shaders/Unity%20URP/NPR/README.md)에 있습니다. 프로젝트는 Unity 2022.3.14f1 / URP 14.0.9를 기준으로 합니다. 이전 빌드는 Unity 2022.3.62f3 / URP 14.0.12에서 확인했으며, 고정 버전 조합의 화면 렌더링은 아직 확인되지 않았습니다.

### 2. 셰이더의 필수 개념 익히기

[Unity 셰이더 기초](./Learning/Shaders/Unity%20URP/셰이더-기초-ShaderLab-HLSL-좌표-텍스처.md)에서 ShaderLab과 URP 셰이더 구조, 정점·프래그먼트 흐름, 좌표 공간, UV와 텍스처 샘플링을 학습합니다. 이후 [조명·블렌딩·정점 애니메이션](./Learning/Shaders/Unity%20URP/조명-블렌딩-정점-애니메이션.md)에서 조명 방향과 법선 등 NPR 조명에 필요한 항목을 다룹니다. 각 문서의 모든 주제를 순서대로 완주할 필요는 없습니다.

### 3. NPR 효과 재현하고 원리 설명하기

[NPR 실습 안내](./Learning/Shaders/Unity%20URP/NPR/README.md)의 순서에 따라 램프 명암, 역면 외곽선, 림·스페큘러, 다중 램프 혼합을 하나씩 바꿔 봅니다. 실습별 입력과 기대 결과를 비교하고, 각 효과의 셰이더 코드와 한계를 설명할 수 있는지 확인합니다.

### 4. 결과 확장하고 검증하기

기존 데모를 바탕으로 색상 단계, 외곽선, 림 효과 중 하나를 확장하거나 조합합니다. 변경 전후 이미지를 남기고, Unity Console 오류와 그림자·머티리얼 동작을 확인합니다. 새 주제를 추가하기보다 기존 효과의 재현성과 학습 결과를 먼저 검증합니다.

## 보조 학습 자료

아래 자료는 핵심 경로의 필수 단계가 아닙니다. 필요한 개념이 있을 때 참고하고, NPR 실습과 직접 연결되지 않는 확장 주제는 완료 목표에 포함하지 않습니다.

| 자료 | 활용 범위 |
|---|---|
| [컴퓨터 그래픽스 학습 로드맵](./Learning/Computer%20Graphics/컴퓨터%20그래픽스%20학습%20로드맵.md) | 벡터·행렬·렌더링 이론을 보충할 때 참고. 전체 단계나 API를 모두 이수할 필요 없음 |
| [HLSL 실습 환경과 공통 규약](./Learning/Shaders/HLSL/0.%20실습%20환경과%20공통%20규약.md) 및 [HLSL 경로](./Learning/Shaders/HLSL/학습-경로.md) | GPU 파이프라인, 리소스 바인딩, 네이티브 셰이더 호스트 코드를 더 깊이 볼 때 참고. Unity URP 셰이더 실습의 선수 과정은 아님 |
| [Direct3D 11 실습 경로](./Learning/Graphics%20APIs/Direct3D%2011/DirectX%2011%20실습%20경로.md) | Unity가 숨기는 그래픽 API와 C++ 호스트 역할을 비교하고 싶을 때 선택 |
| [Unity 셰이더 경험자를 위한 그래픽 API 개요](./Learning/Graphics%20APIs/Unity%20셰이더%20경험자를%20위한%20그래픽%20API%20개요.md) | Unity와 네이티브 API의 파이프라인·바인딩 차이를 빠르게 비교 |
| [Unity 셰이더 확장 주제](./Learning/Shaders/Unity%20URP/학습-경로.md) | 노멀 매핑, 디졸브, 후처리, 컴퓨트 등 NPR 데모 밖의 기법을 개별 목적에 따라 학습 |

## 독립 보관 자료

[Windows API 학습 자료와 예제](Archive/Windows%20API/Lessons/0.%20Win32%20학습%20커리큘럼.md)는 NPR 셰이더 학습과 별개의 Windows 데스크톱 프로그래밍 경로입니다. 기존 자료를 보존하되 이 저장소의 핵심 학습 경로와는 분리했습니다.

## 분류 기준

```text
Learning/
  Shaders/Unity URP/       NPR 핵심 경로와 Unity 셰이더 선택 주제
  Shaders/HLSL/            네이티브 HLSL 선택 참고 자료
  Graphics APIs/           Direct3D 11 및 API 비교 참고 자료
  Computer Graphics/       수학·그래픽스 이론 보충 자료
Projects/
  NPR Unity/               실행 가능한 대표 데모
Archive/
  Windows API/             NPR과 무관한 독립 자료를 삭제 없이 보관
    README.md               보관 자료 진입점
docs/                      검토·개선 기록
```

새 내용은 NPR 학습 목표, 실행 가능한 결과, 관찰 가능한 완료 기준을 모두 제시할 수 있을 때만 핵심 경로에 추가합니다. 검증 범위와 알려진 제한은 [저장소 검토 결과](./docs/REVIEW.md), 작업 우선순위와 진행 기록은 [개선 계획](./docs/LEARNING_MATERIALS_PLAN.md)을 참고하세요.

## 참고 도구

- 개발: Unity 2022.3 LTS, Visual Studio
- 그래픽스 디버깅: Unity Frame Debugger, RenderDoc, PIX for Windows
- 이론 및 예제: [Real-Time Rendering](https://www.realtimerendering.com/), [Catlike Coding](https://catlikecoding.com/), [Microsoft DirectX Graphics Samples](https://github.com/microsoft/DirectX-Graphics-Samples)
