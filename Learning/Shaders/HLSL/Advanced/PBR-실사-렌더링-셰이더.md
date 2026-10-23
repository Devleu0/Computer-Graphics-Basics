## 실사 렌더링을 위한 PBR 셰이더 가이드 (Metallic-Roughness)
이 문서는 기존의 퐁(Phong) 조명 모델을 넘어, 현대 게임 및 영화에서 표준으로 사용되는 물리 기반 렌더링(Physically Based Rendering, PBR)의 원리를 이해하고 HLSL로 직접 구현하는 방법을 다룹니다.

> **따라 하기**: 코드는 셰이더 부분이며 C++ 호스트가 필요합니다. 환경·행렬 규약은 [실습 환경과 공통 규약](../0.%20실습%20환경과%20공통%20규약.md)을 먼저 읽으세요. 이 문서의 코드는 앞선 HLSL 기초·중급·고급 모듈에서 만든 구조체/상수 버퍼를 이어서 사용하며, 작성자가 직접 컴파일하지는 않았습니다.

### PBR이란 무엇인가?
PBR은 '물리 법칙에 기반'하여 빛과 재질의 상호작용을 시뮬레이션하는 렌더링 기법입니다. 아티스트의 감에 의존하던 기존 방식과 달리, PBR은 몇 가지 직관적인 물리 속성(예: 거칠기, 금속성)만으로 다양한 재질을 일관되고 사실적으로 표현할 수 있습니다.

PBR의 핵심 원칙 3가지:

* 마이크로파셋 이론 (Microfacet Theory): 모든 표면은 미시적으로 봤을 때 완벽히 매끄럽지 않고, 수많은 미세한 평면(microfacet)으로 이루어져 있다고 가정합니다. '거칠기(Roughness)' 값은 이 미세 평면들이 얼마나 정돈되어 있는지를 나타냅니다. 거칠기가 낮으면(매끄러우면) 빛이 한 방향으로 반사(정반사)되고, 높으면(거칠면) 여러 방향으로 흩어집니다(난반사).

* 에너지 보존 법칙 (Energy Conservation): 표면은 입사된 빛보다 더 많은 빛을 반사할 수 없습니다. 빛이 표면에서 난반사되면, 그만큼 정반사될 에너지는 줄어듭니다. PBR은 이 법칙을 철저히 준수하여 과도하게 밝아지는 현상을 막습니다.

* 프레넬 효과 (Fresnel Effect): 표면을 바라보는 시선 각도에 따라 반사율이 달라지는 현상입니다. 거의 모든 재질은 수직으로 바라볼 때보다, 비스듬히(grazing angles) 바라볼 때 더 높은 반사율을 보입니다. 물웅덩이를 바로 아래에서 볼 때와 멀리서 비스듬히 볼 때를 생각하면 쉽습니다.

* Cook-Torrance BRDF: PBR의 심장
PBR을 구현하기 위해 우리는 양방향 반사 분포 함수(BRDF) 라는 수학적 모델을 사용합니다. 그중 가장 널리 쓰이는 것이 Cook-Torrance BRDF입니다. 이 모델은 정반사(Specular) 부분을 세 가지 함수로 나누어 계산합니다.
  * Cook-Torrance Specular BRDF = D * G * F / (4 * (N·L) * (N·V))

    *  D (Normal Distribution Function - 정규 분포 함수): 마이크로파셋들이 얼마나 특정 방향으로 정렬되어 있는지를 계산합니다. Roughness 값에 따라 하이라이트의 모양과 크기를 결정합니다. (GGX 모델 사용)

    *  G (Geometry Function - 기하 함수): 마이크로파셋들 사이에서 발생하는 빛의 차폐(shadowing)와 가림(masking) 현상을 모델링합니다. 울퉁불퉁한 표면에서는 빛이 스스로를 가리는 현상을 표현합니다. (Smith-Schlick GGX 모델 사용)

    *  F (Fresnel Equation - 프레넬 함수): 시선 각도에 따른 표면 반사율을 계산합니다. (Schlick 근사식 사용)

### HLSL 구현: 단계별 접근
1. 입력 텍스처와 재질 속성
PBR 셰이더는 기존과 다른 텍스처들을 입력으로 받습니다.

* Albedo: 재질의 기본 색상입니다. (금속의 경우 반사광의 색, 비금속의 경우 표면의 색)

* Normal: 노멀 맵. 표면의 요철을 표현합니다.

* Metallic: 금속성(0.0: 비금속, 1.0: 금속). 재질이 금속인지 아닌지를 결정하는 핵심 값입니다.

* Roughness: 거칠기(0.0: 매끄러움, 1.0: 거침). 표면의 매끄러운 정도를 나타냅니다.

* AO (Ambient Occlusion): 주변광 차폐. 틈새나 구석진 곳이 어두워지는 효과를 더해 깊이감을 줍니다.

2. 핵심 함수 구현
Cook-Torrance BRDF를 구성하는 각 함수를 HLSL로 구현합니다.
```hlsl
// 상수 정의
static const float PI = 3.14159265359;

// D: Trowbridge-Reitz GGX (정규 분포 함수)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / denom;
}

// G: Smith's Method with Schlick-GGX (기하 함수)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float denom = NdotV * (1.0 - k) + k;
    
    return NdotV / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// F: Fresnel-Schlick Approximation (프레넬 함수)
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}
```
3. 메인 픽셀 셰이더
이제 모든 조각을 하나로 합쳐 최종 색상을 계산합니다. PBR은 직접광(Directional/Point light)과 간접광(Image-Based Light)을 모두 고려해야 완벽한 결과가 나옵니다. 여기서는 직접광 계산에 집중합니다.
이 코드가 컴파일되려면 아래 선언이 먼저 있어야 합니다 (HLSL 기초·중급 모듈과 동일).
```hlsl
cbuffer CBufferPerFrame : register(b1) { float4 LightDir; float4 LightColor; float4 CameraPos; } // LightDir: 빛이 진행하는 방향
struct PS_INPUT { float4 Pos : SV_POSITION; float3 Normal : NORMAL; float3 WorldPos : TEXCOORD0; float2 Tex : TEXCOORD1; };
SamplerState ObjSampler : register(s0);
```
```hlsl
// 입력 텍스처들 (Albedo 는 sRGB 포맷, 나머지는 UNORM 으로 SRV 를 만든다)
Texture2D AlbedoMap    : register(t0);
Texture2D NormalMap    : register(t1);
Texture2D MetallicMap  : register(t2);
Texture2D RoughnessMap : register(t3);

float4 PS(PS_INPUT input) : SV_Target
{
    // 1. 텍스처에서 재질 속성 샘플링
    float3 albedo = AlbedoMap.Sample(ObjSampler, input.Tex).rgb;
    float metallic = MetallicMap.Sample(ObjSampler, input.Tex).r;
    float roughness = RoughnessMap.Sample(ObjSampler, input.Tex).r;
    float3 N = normalize(input.Normal); // 노멀 맵을 사용한다면 여기서 계산

    // 2. 기본 벡터 계산
    float3 V = normalize(CameraPos.xyz - input.WorldPos);
    float3 L = normalize(-LightDir.xyz);
    float3 H = normalize(V + L);

    // 3. 재질에 따른 F0(기본 반사율) 계산
    float3 F0 = float3(0.04, 0.04, 0.04); // 대부분의 비금속 F0
    F0 = lerp(F0, albedo, metallic); // 금속일 경우 F0는 albedo 색상

    // 4. Cook-Torrance BRDF 계산
    float NDF = DistributionGGX(N, H, roughness);   
    float G = GeometrySmith(N, V, L, roughness);      
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

    // 정반사(Specular) BRDF 계산
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * saturate(dot(N, V)) * saturate(dot(N, L)) + 0.0001; // 0으로 나누는 것 방지
    float3 specular = numerator / denominator;

    // 5. 에너지 보존에 따른 난반사(Diffuse)/정반사 비율 계산
    float3 kS = F; // 정반사 비율은 프레넬 값
    float3 kD = float3(1.0, 1.0, 1.0) - kS; // 난반사 비율
    kD *= (1.0 - metallic); // 금속은 난반사가 없으므로 0으로 만듦

    // 6. 최종 색상 계산
    float NdotL = saturate(dot(N, L));
    // (난반사 + 정반사) * 광원 색 * 빛의 세기
    float3 Lo = (kD * albedo / PI + specular) * LightColor.rgb * NdotL;

    // 톤 매핑을 생략하고 선형 색을 그대로 출력한다.
    // 렌더 타겟이 *_SRGB 포맷이면 출력 시 감마 보정이 자동 적용된다. UNORM 이면 아래 줄을 켜서 직접 보정해야 한다.
    float3 color = Lo;
    // color = pow(color, 1.0 / 2.2);
    
    return float4(color, 1.0);
}
```
> **주의**: `LightColor` 의 밝기가 1 정도이면 결과가 어둡게 보입니다. PBR 은 `Lo = BRDF × 빛의 세기 × cosθ` 이므로 방향광은 `LightColor` 를 3~5 정도로 두고 시작해 보세요. 환경광(ambient)과 간접광이 없어 그림자 쪽이 완전히 검게 나오는 것은 정상입니다.

### 다음 단계: 간접광 (Image-Based Lighting)
위 코드는 직접광(하나의 광원)만을 계산합니다. PBR의 진정한 힘은 간접광, 즉 주변 환경으로부터 반사되는 빛을 계산할 때 나타납니다. 이를 위해서는 다음과 같은 추가적인 기술이 필요합니다.

* Irradiance Map: 주변 환경의 난반사광을 미리 계산해 둔 큐브맵입니다.

* Prefiltered Environment Map: 다양한 거칠기 수준에 따른 정반사광을 미리 계산하여 큐브맵의 밉맵(Mipmap) 레벨에 저장한 것입니다.

* BRDF Lookup Table (LUT): 프레넬 계산의 일부를 미리 2D 텍스처에 구워놓은 것입니다.

#### 직접 구현하는 순서

1. **기준 장면을 기록합니다.** 현재 셰이더에서 직접광만 켠 상태로 금속과 비금속 구를 렌더링하고 결과를 저장합니다. 광원 색·세기와 재질 값을 고정해야 이후 비교가 가능합니다.
2. **환경맵을 준비합니다.** 선형 색 공간의 환경 큐브맵을 로드하고, 표면에서 반사되는 방향을 계산해 배경 환경이 조회되는지 먼저 확인합니다.
3. **난반사 간접광을 추가합니다.** 환경맵을 코사인 가중으로 미리 적분한 Irradiance Map을 생성하고, 표면 법선 방향으로 샘플링해 난반사 항에 더합니다. 금속도 난반사광을 받는다고 가정하지 않도록 기존의 `kD` 비율을 적용합니다.
4. **정반사 간접광을 추가합니다.** 거칠기별로 미리 흐리게 만든 Prefiltered Environment Map을 반사 방향으로 샘플링합니다. 거칠기에 따라 큐브맵의 밉 레벨이 달라지는지 확인합니다.
5. **BRDF LUT로 합성합니다.** `N·V`와 roughness를 LUT 좌표로 사용하고, LUT 결과와 프리필터 환경광 및 프레넬 항을 결합합니다. 직접광만 사용한 결과와 비교해 그림자가 완전히 검게 뭉개지지 않고 금속 표면에 주변 환경이 반사되는지 확인합니다.

**완료 기준**: 금속 구에 주변 환경의 색과 형태가 반사되고, roughness를 높이면 반사가 넓고 흐려지며, 비금속 구에는 난반사 간접광이 남습니다. 직접광 항을 끄더라도 환경광이 있는 곳과 없는 곳의 차이가 설명 가능해야 합니다.

> **자료 상태**: 이 저장소에는 Irradiance Map·Prefiltered Environment Map·BRDF LUT를 생성하고 바인딩하는 독립적인 IBL 실습 문서가 아직 없습니다. 위 순서는 구현 계획이며, 완성형 코드 예제로 검증된 튜토리얼을 의미하지 않습니다. 해당 문서는 개선 계획의 후속 작업으로 추적합니다.
