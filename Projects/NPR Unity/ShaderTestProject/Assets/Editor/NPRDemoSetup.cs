using System.IO;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace ShaderStudy.NPR
{
    public static class NPRDemoSetup
    {
        const string ScenePath = "Assets/Scenes/NPRDemo.unity";
        const string MaterialPath = "Assets/Materials/NPRToon.mat";
        const string FloorMaterialPath = "Assets/Materials/NPRDemoFloor.mat";
        const string WarmRampPath = "Assets/Textures/NPR/Ramp_Warm.png";
        const string CoolRampPath = "Assets/Textures/NPR/Ramp_Cool.png";

        [MenuItem("Tools/NPR/Create Demo Scene")]
        public static void CreateDemoScene()
        {
            if (!Application.isBatchMode && !EditorSceneManager.SaveCurrentModifiedScenesIfUserWantsTo())
                return;
            if (!Application.isBatchMode && DemoAssetsExist() &&
                !EditorUtility.DisplayDialog(
                    "Rebuild NPR Demo",
                    "The generated NPR demo scene, materials, and ramp textures will be replaced.",
                    "Rebuild",
                    "Cancel"))
                return;

            EnsureFolder("Assets/Textures");
            EnsureFolder("Assets/Textures/NPR");
            CreateRampTexture(WarmRampPath, new[]
            {
                new Color32(110, 80, 90, 255),
                new Color32(210, 170, 150, 255),
                new Color32(255, 240, 220, 255)
            });
            CreateRampTexture(CoolRampPath, new[]
            {
                new Color32(60, 70, 120, 255),
                new Color32(130, 150, 200, 255),
                new Color32(225, 235, 255, 255)
            });

            AssetDatabase.Refresh();
            ConfigureRampImporter(WarmRampPath);
            ConfigureRampImporter(CoolRampPath);

            Shader toonShader = Shader.Find("Tutorial/NPR/Toon");
            Shader litShader = Shader.Find("Universal Render Pipeline/Lit");
            if (toonShader == null || litShader == null)
                throw new System.InvalidOperationException("NPR or URP Lit shader could not be found.");

            Material toonMaterial = LoadOrCreateMaterial(MaterialPath, toonShader);
            toonMaterial.SetTexture("_WarmRamp", AssetDatabase.LoadAssetAtPath<Texture2D>(WarmRampPath));
            toonMaterial.SetTexture("_CoolRamp", AssetDatabase.LoadAssetAtPath<Texture2D>(CoolRampPath));
            toonMaterial.SetColor("_Color", new Color(1f, 0.92f, 0.84f, 1f));
            toonMaterial.SetColor("_OutlineColor", new Color(0.035f, 0.025f, 0.04f, 1f));
            toonMaterial.SetFloat("_OutlineThickness", 0.012f);
            toonMaterial.SetVector("_OutlineDistanceFade", new Vector4(15f, 45f, 0f, 0f));
            toonMaterial.SetColor("_RimColor", new Color(0.5f, 0.72f, 1f, 1f));
            toonMaterial.SetFloat("_RimWidth", 0.72f);
            toonMaterial.SetFloat("_RimSharpness", 2.5f);
            toonMaterial.SetFloat("_Glossiness", 0.45f);
            toonMaterial.SetFloat("_SpecularThreshold", 0.88f);

            Material floorMaterial = LoadOrCreateMaterial(FloorMaterialPath, litShader);
            floorMaterial.SetColor("_BaseColor", new Color(0.16f, 0.19f, 0.25f, 1f));
            floorMaterial.SetFloat("_Smoothness", 0.25f);

            CreateScene(toonMaterial, floorMaterial);
            AssetDatabase.SaveAssets();
            AssetDatabase.Refresh();
            Debug.Log("NPR demo scene created at " + ScenePath);
        }

        static bool DemoAssetsExist()
        {
            return File.Exists(ScenePath) || File.Exists(MaterialPath) ||
                   File.Exists(FloorMaterialPath) || File.Exists(WarmRampPath) ||
                   File.Exists(CoolRampPath);
        }

        static void CreateRampTexture(string path, Color32[] colors)
        {
            var texture = new Texture2D(256, 16, TextureFormat.RGB24, false);
            for (int x = 0; x < texture.width; x++)
            {
                Color32 color = colors[x * colors.Length / texture.width];
                for (int y = 0; y < texture.height; y++)
                    texture.SetPixel(x, y, color);
            }
            texture.Apply();
            File.WriteAllBytes(path, texture.EncodeToPNG());
            Object.DestroyImmediate(texture);
        }

        static void ConfigureRampImporter(string path)
        {
            var importer = (TextureImporter)AssetImporter.GetAtPath(path);
            importer.sRGBTexture = true;
            importer.filterMode = FilterMode.Point;
            importer.wrapMode = TextureWrapMode.Clamp;
            importer.mipmapEnabled = false;
            importer.SaveAndReimport();
        }

        static Material LoadOrCreateMaterial(string path, Shader shader)
        {
            Material material = AssetDatabase.LoadAssetAtPath<Material>(path);
            if (material == null)
            {
                material = new Material(shader);
                AssetDatabase.CreateAsset(material, path);
            }
            else
            {
                material.shader = shader;
            }
            return material;
        }

        static void CreateScene(Material toonMaterial, Material floorMaterial)
        {
            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            RenderSettings.ambientMode = UnityEngine.Rendering.AmbientMode.Flat;
            RenderSettings.ambientLight = new Color(0.22f, 0.22f, 0.25f);

            GameObject sphere = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            sphere.name = "Toon Sphere";
            sphere.transform.position = new Vector3(-0.9f, 1f, 0f);
            sphere.GetComponent<Renderer>().sharedMaterial = toonMaterial;

            GameObject capsule = GameObject.CreatePrimitive(PrimitiveType.Capsule);
            capsule.name = "Toon Capsule";
            capsule.transform.position = new Vector3(1f, 0.9f, 0f);
            capsule.transform.localScale = new Vector3(0.8f, 0.9f, 0.8f);
            capsule.GetComponent<Renderer>().sharedMaterial = toonMaterial;

            GameObject ground = GameObject.CreatePrimitive(PrimitiveType.Plane);
            ground.name = "Ground";
            ground.transform.position = new Vector3(0f, -0.02f, 0f);
            ground.transform.localScale = new Vector3(1.2f, 1f, 1.2f);
            ground.GetComponent<Renderer>().sharedMaterial = floorMaterial;

            var cameraObject = new GameObject("Main Camera");
            cameraObject.tag = "MainCamera";
            var camera = cameraObject.AddComponent<Camera>();
            cameraObject.AddComponent<AudioListener>();
            cameraObject.transform.position = new Vector3(0f, 2.1f, -6.5f);
            cameraObject.transform.LookAt(new Vector3(0f, 0.9f, 0f));
            camera.fieldOfView = 38f;
            camera.clearFlags = CameraClearFlags.SolidColor;
            camera.backgroundColor = new Color(0.11f, 0.13f, 0.18f, 1f);

            var lightObject = new GameObject("Directional Light");
            var light = lightObject.AddComponent<Light>();
            light.type = LightType.Directional;
            light.color = new Color(1f, 0.84f, 0.68f);
            light.intensity = 1.2f;
            light.shadows = LightShadows.Soft;
            lightObject.transform.rotation = Quaternion.Euler(45f, -30f, 0f);

            var controllerObject = new GameObject("NPR Ramp Blend Controller");
            var controller = controllerObject.AddComponent<NPRRampBlendController>();
            var serializedController = new SerializedObject(controller);
            serializedController.FindProperty("mainLight").objectReferenceValue = light;
            serializedController.ApplyModifiedPropertiesWithoutUndo();

            EditorSceneManager.SaveScene(scene, ScenePath);
            EditorBuildSettingsScene[] currentScenes = EditorBuildSettings.scenes;
            bool sceneInBuild = false;
            foreach (EditorBuildSettingsScene buildScene in currentScenes)
                sceneInBuild |= buildScene.path == ScenePath;
            if (!sceneInBuild)
            {
                var updatedScenes = new EditorBuildSettingsScene[currentScenes.Length + 1];
                currentScenes.CopyTo(updatedScenes, 0);
                updatedScenes[currentScenes.Length] = new EditorBuildSettingsScene(ScenePath, true);
                EditorBuildSettings.scenes = updatedScenes;
            }
        }

        static void EnsureFolder(string path)
        {
            if (AssetDatabase.IsValidFolder(path))
                return;

            string parent = Path.GetDirectoryName(path).Replace('\\', '/');
            string folder = Path.GetFileName(path);
            AssetDatabase.CreateFolder(parent, folder);
        }
    }
}
