# Render Engine Architecture Migration Plan

## Purpose

Refactor the existing render engine into a plugin-oriented architecture that is open for extension along two independent axes:

1. New renderable object types can be added without modifying the core renderer, scene system, render frame, or existing renderers.
2. New renderer implementations can be added without modifying the scene system, existing renderers, the application loop, or a central renderer factory.

The target architecture must support:

- renderer-independent scene representation;
- renderer-aware extraction;
- extensible typed render data;
- renderer extensions that contribute render work without modifying renderer classes;
- renderer registration through factories;
- capability-based and exact-type matching;
- optional integration plugins for specific renderable-type/renderer combinations;
- incremental migration from the project’s current architecture.

The coding agent must adapt names, ownership, threading, and memory conventions to the existing codebase rather than introducing parallel abstractions unnecessarily.

---

# 1. Target Runtime Flow

The final frame flow should be:

```text
Scene
  -> immutable/read-only SceneSnapshot
  -> renderer-aware extraction pipeline
  -> RenderFrame with typed data channels
  -> selected Renderer
  -> base render graph
  -> compatible renderer extensions contribute passes
  -> render graph compile and execute
```

Target application loop:

```cpp
auto renderer = engine.rendererRegistry().create("deferred");

while (application.running()) {
    scene.update(deltaTime);

    SceneSnapshot snapshot = scene.createSnapshot();

    RenderFrame frame = engine.renderFrameBuilder().build(
        snapshot,
        renderer->descriptor()
    );

    renderer->draw(frame);
}
```

The exact API may differ, but preserve these dependency rules:

- `Scene` must not depend on `Renderer`.
- `Renderer` must not depend on `Scene`, `SceneNode`, ECS components, or gameplay objects.
- Scene types must not cross the render boundary.
- Renderer implementation types must not cross into the scene module.
- The bridge/extraction layer is the only place allowed to understand both scene-side and render-contract concepts.
- New renderable types must be introduced through registration and composition, not by editing central switches or fixed unions.
- New renderer types must be introduced through registry expansion, not by editing central switches or factories.

---

# 2. Required Core Abstractions

Implement or adapt the following concepts.

## 2.1 Stable type identity

Introduce a stable type ID mechanism for runtime registration and typed channels.

```cpp
using StableTypeId = std::uint64_t;

template<typename T>
StableTypeId typeId();
```

Requirements:

- deterministic within the supported runtime boundary;
- safe for shared library/plugin usage if runtime plugins are supported;
- no dependence on compiler-specific RTTI names unless the project explicitly accepts that limitation;
- collision detection in debug builds;
- human-readable debug names.

Possible implementation strategies:

- explicit compile-time IDs;
- hashed canonical names;
- central type registration;
- existing engine reflection/type system.

Prefer the project’s existing reflection or ID system if available.

---

## 2.2 Renderer descriptor

```cpp
using RendererTypeId = StableTypeId;
using CapabilityId = StableTypeId;

struct RendererDescriptor {
    RendererTypeId type;
    std::string_view name;
    CapabilitySet capabilities;
};
```

Capabilities should describe broad renderer support, for example:

```cpp
namespace RenderCapabilities {
    constexpr CapabilityId Rasterization;
    constexpr CapabilityId RayTracing;
    constexpr CapabilityId Compute;
    constexpr CapabilityId DeferredShading;
    constexpr CapabilityId ForwardShading;
    constexpr CapabilityId ParticipatingMedia;
    constexpr CapabilityId SignedDistanceFields;
}
```

Requirements:

- exact renderer matching must be possible;
- capability matching must be possible;
- descriptors must be immutable after renderer construction;
- capability checks must be cheap;
- debug output must show renderer name and capabilities.

---

## 2.3 Scene snapshot or read view

Create a render-safe, read-only scene interface.

```cpp
class SceneSnapshot {
public:
    template<typename... Components>
    auto view() const;

    template<typename Component>
    const Component* tryGet(EntityId entity) const;
};
```

Requirements:

- renderer code must never receive this type;
- extractors may read it;
- it must provide a consistent frame view;
- it should support future multithreaded extraction;
- avoid exposing mutable scene state;
- if the current project already uses ECS snapshots, read locks, immutable registries, or frame copies, adapt those.

Initial implementation may wrap the current scene representation if full snapshotting is too invasive, but its public contract must remain read-only.

---

## 2.4 Extensible render data store

Replace fixed render-world fields with typed channels.

```cpp
class IRenderDataChannel {
public:
    virtual ~IRenderDataChannel() = default;
    virtual StableTypeId elementType() const = 0;
};

template<typename T>
class RenderDataChannel final : public IRenderDataChannel {
public:
    void append(T value);
    std::span<const T> items() const;
};

class RenderDataStore {
public:
    template<typename T>
    RenderDataChannel<T>& getOrCreate();

    template<typename T>
    void append(T value);

    template<typename T>
    std::span<const T> read() const;

    template<typename T>
    bool contains() const;
};
```

Requirements:

- adding a new data type must not require changing `RenderDataStore`;
- channels are keyed by stable type ID;
- frame data is read-only once extraction ends;
- channel lifetime is tied to the frame;
- debug mode validates type IDs and channel types;
- support efficient bulk append;
- allow allocators or frame arenas used by the existing engine;
- no per-element virtual dispatch;
- no `std::any` in hot loops unless performance is explicitly acceptable.

Potential future optimization:

- pre-register channel factories;
- use frame arenas;
- use structure-of-arrays channels;
- cache channel lookup indices;
- support persistent channels and change sets.

Do not optimize prematurely during the first migration phase.

---

## 2.5 Render frame

```cpp
struct RenderFrame {
    RenderDataStore data;
    std::vector<RenderView> views;
    FrameMetadata metadata;
};
```

`RenderFrame` should contain renderer-facing frame input only.

It must not contain:

- `Scene*`;
- `SceneNode*`;
- entity component registry references;
- gameplay objects;
- backend-specific GPU command objects;
- concrete renderer pointers.

---

## 2.6 Render extractor interface

```cpp
class IRenderExtractor {
public:
    virtual ~IRenderExtractor() = default;

    virtual bool supports(
        const RendererDescriptor& renderer
    ) const = 0;

    virtual void extract(
        const SceneSnapshot& scene,
        RenderExtractionContext& context,
        RenderDataStore& output
    ) const = 0;
};
```

`RenderExtractionContext` may expose stable services:

```cpp
class RenderExtractionContext {
public:
    const RendererDescriptor& renderer() const;
    RenderResourceResolver& resources();
    FrameAllocator& allocator();
    Diagnostics& diagnostics();
};
```

Requirements:

- extractor registration must be open-ended;
- extractor execution order must be deterministic;
- optional extractor dependencies or phases must be supported;
- extraction errors must be diagnosable;
- extractors should be stateless by default;
- stateful extractors must have explicit lifetime and thread-safety policy.

---

## 2.7 Extraction registry

```cpp
class ExtractionRegistry {
public:
    RegistrationToken registerExtractor(
        std::unique_ptr<IRenderExtractor> extractor,
        ExtractorRegistrationInfo info
    );

    void extractFor(
        const RendererDescriptor& renderer,
        const SceneSnapshot& scene,
        RenderExtractionContext& context,
        RenderDataStore& output
    ) const;
};
```

`ExtractorRegistrationInfo` should support:

- debug name;
- phase;
- priority;
- optional dependencies;
- optional plugin owner;
- enabled/disabled state.

Suggested phases:

```cpp
enum class ExtractionPhase {
    Views,
    Geometry,
    Lighting,
    Volumes,
    Effects,
    Debug
};
```

Do not rely only on registration order.

---

## 2.8 Renderer base contract

```cpp
class Renderer {
public:
    virtual ~Renderer() = default;

    virtual const RendererDescriptor& descriptor() const = 0;

    void installExtension(
        std::unique_ptr<IRendererExtension> extension
    );

    void draw(const RenderFrame& frame);

protected:
    virtual void buildBaseGraph(
        const RenderFrame& frame,
        RenderGraphBuilder& graph,
        RenderExtensionContext& context
    ) = 0;
};
```

`draw` should:

1. create/reset the graph;
2. allow the renderer to build its base graph;
3. execute compatible installed extensions;
4. validate graph resources;
5. compile the graph;
6. execute the graph.

If the current project has no render graph, introduce an intermediate extension API first and migrate to a render graph later. However, do not expose fixed renderer internals directly to plugins.

---

## 2.9 Renderer extension interface

```cpp
class IRendererExtension {
public:
    virtual ~IRendererExtension() = default;

    virtual bool supports(
        const RendererDescriptor& renderer
    ) const = 0;

    virtual void contribute(
        const RenderFrame& frame,
        RenderGraphBuilder& graph,
        RenderExtensionContext& context
    ) = 0;
};
```

Requirements:

- extensions must not require edits to concrete renderer classes;
- extensions may own renderer-instance-specific resources;
- each renderer instance receives separate extension instances;
- extension execution order must be deterministic;
- extension dependencies must be expressible;
- extensions must have restricted access through `RenderExtensionContext`;
- renderer private implementation details must remain inaccessible.

---

## 2.10 Renderer extension context

```cpp
class RenderExtensionContext {
public:
    const RendererDescriptor& renderer() const;

    RenderResourceRegistry& resources();
    PipelineRegistry& pipelines();
    ShaderRegistry& shaders();
    Device& device();

    std::optional<RenderResourceHandle>
    findResource(RenderResourceSemantic semantic);
};
```

Expose only stable services.

Do not pass the concrete renderer object to extensions unless the match is exact and the project explicitly accepts that coupling.

---

## 2.11 Renderer matching

```cpp
class RendererMatch {
public:
    static RendererMatch exact(RendererTypeId type);

    static RendererMatch requires(
        CapabilitySet capabilities
    );

    static RendererMatch predicate(
        std::function<bool(const RendererDescriptor&)> fn
    );

    bool matches(const RendererDescriptor& renderer) const;
};
```

Prefer declarative matching over arbitrary predicates where possible.

---

## 2.12 Renderer registry

```cpp
class RendererRegistry {
public:
    using Factory =
        std::function<std::unique_ptr<Renderer>()>;

    RegistrationToken registerRenderer(
        std::string key,
        RendererDescriptor descriptor,
        Factory factory
    );

    std::unique_ptr<Renderer> create(
        std::string_view key
    ) const;
};
```

Requirements:

- no central renderer `switch`;
- duplicate registration must fail clearly;
- renderer keys must be enumerable;
- renderer descriptors should be queryable before creation;
- creation should automatically install compatible renderer extensions.

---

## 2.13 Renderer extension registry

```cpp
class RendererExtensionRegistry {
public:
    using Factory =
        std::function<std::unique_ptr<IRendererExtension>()>;

    RegistrationToken registerFactory(
        RendererMatch match,
        Factory factory,
        RendererExtensionRegistrationInfo info
    );

    void installCompatibleExtensions(
        Renderer& renderer
    ) const;
};
```

Requirements:

- registry owns factories, not shared extension instances;
- each renderer receives its own instances;
- installation order is deterministic;
- unsupported required dependencies produce explicit diagnostics.

---

## 2.14 Plugin registration contract

```cpp
class IEnginePlugin {
public:
    virtual ~IEnginePlugin() = default;

    virtual void install(
        EngineExtensionRegistry& registry
    ) = 0;
};
```

```cpp
class EngineExtensionRegistry {
public:
    RendererRegistry renderers;
    ExtractionRegistry extractors;
    RendererExtensionRegistry rendererExtensions;
};
```

Use the existing project plugin/module system if present.

The new architecture should integrate into existing startup and dependency-injection infrastructure rather than creating a second module loader.

---

# 3. Migration Strategy

Do not rewrite the renderer in one step.

Use a strangler-style migration that preserves a working build after each phase.

---

## Phase 0: Architecture inventory

Before modifying code, inspect and document:

- current `Scene` ownership;
- scene graph or ECS model;
- current renderable base classes;
- current renderer hierarchy;
- how objects submit render commands;
- current frame loop;
- render backend ownership;
- resource cache ownership;
- material and mesh handles;
- visibility and culling;
- camera and view handling;
- threading model;
- plugin/module system;
- current render passes;
- current factory or switch statements;
- test coverage.

Produce a mapping table:

```text
Current Type/System              Target Role
-------------------------------------------------------------
Existing Scene                   Scene
Existing read lock/snapshot      SceneSnapshot
Current render queue             Initial RenderDataStore adapter
Current renderer base            Renderer
Current render pass API          RenderGraphBuilder or compatibility layer
Current module registry          EngineExtensionRegistry
```

Identify coupling hotspots:

- renderer includes scene headers;
- scene objects include renderer headers;
- `dynamic_cast` on scene/renderable types;
- central render-object enums;
- renderer switches by object type;
- object switches by renderer type;
- fixed `RenderWorld` fields;
- central renderer factory switches.

No behavioral changes in this phase.

---

## Phase 1: Add render contracts module

Create a module that contains only renderer-facing contracts:

- stable type IDs;
- renderer descriptors;
- capabilities;
- render views;
- render frame;
- typed data store;
- extraction interfaces;
- renderer extension interfaces;
- registration metadata.

Rules:

- no scene implementation headers;
- no concrete renderer headers;
- no backend-specific API types except opaque handles;
- build this module independently if the build system allows it.

Add unit tests for:

- type ID uniqueness;
- typed channel creation;
- typed channel retrieval;
- missing channel behavior;
- duplicate renderer registration;
- renderer match behavior.

---

## Phase 2: Wrap the existing scene with `SceneSnapshot`

Introduce a read-only adapter around the existing scene.

Initial version may hold:

- a const reference protected by a frame lock;
- a copied list of render-relevant entities;
- an ECS read-only registry;
- an immutable snapshot.

Do not let renderer code access it.

Refactor only the extraction-facing reads first.

Acceptance criteria:

- existing scene behavior unchanged;
- snapshot cannot mutate scene state;
- snapshot lifetime is explicit;
- invalid lifetime use is detectable in debug builds.

---

## Phase 3: Introduce `RenderDataStore`

Add typed channels without removing the current render queue.

Create adapters such as:

```cpp
struct LegacyMeshDrawData {
    // data required by the current renderer
};
```

Implement a first extractor:

```cpp
class LegacyMeshExtractor final : public IRenderExtractor;
```

It should fill a typed channel that the existing renderer can consume.

Temporarily allow:

```text
SceneSnapshot
  -> new extractor
  -> typed RenderDataStore
  -> compatibility adapter
  -> existing render queue
  -> existing renderer
```

This proves the boundary before refactoring the renderer.

Acceptance criteria:

- at least one existing renderable category uses extraction;
- the renderer no longer traverses scene objects for that category;
- visual output remains unchanged;
- frame capture comparison or image regression passes.

---

## Phase 4: Introduce extraction registry

Replace direct extractor invocation with registration.

Register all current extractors through startup/module installation.

Add deterministic phases and priorities.

Initially migrate categories in this order:

1. camera/view data;
2. static meshes;
3. lights;
4. skinned meshes;
5. particles;
6. decals;
7. volumes;
8. debug rendering;
9. renderer-specific effects.

For every migrated category:

- remove renderer traversal of the corresponding scene type;
- remove scene-to-renderer callbacks;
- add extracted render data;
- preserve current behavior.

Do not create one universal `RenderObject` structure.

Use separate typed data structures.

---

## Phase 5: Add renderer descriptors and registry

Give the existing renderer a descriptor.

Example:

```cpp
RendererDescriptor {
    .type = typeId<ExistingDeferredRenderer>(),
    .name = "Existing Deferred Renderer",
    .capabilities = {
        Rasterization,
        Compute,
        DeferredShading
    }
};
```

Replace central renderer construction with `RendererRegistry`.

Keep existing configuration compatibility:

```cpp
config.renderer = "deferred";
```

The registry may initially contain only one renderer.

Acceptance criteria:

- renderer creation no longer uses a central type switch;
- renderer enumeration works;
- unknown renderer keys produce clear errors;
- no behavior change.

---

## Phase 6: Add renderer extensions

Select one existing optional feature that currently modifies or subclasses the renderer, such as:

- fog;
- decals;
- SSAO;
- debug overlay;
- bloom;
- volumetric lighting;
- editor selection outline.

Convert it into `IRendererExtension`.

The extension must:

- match the current renderer by capability or exact type;
- read its typed channel from `RenderFrame`;
- contribute its render work;
- own its own pipeline/resources;
- be installed automatically through the registry.

If no render graph exists, create a compatibility interface:

```cpp
class RenderPassContributionBuilder {
public:
    void addBefore(...);
    void addAfter(...);
    void addPass(...);
};
```

This is transitional. The long-term target should be resource-dependency-based graph scheduling.

Acceptance criteria:

- feature code is removed from the concrete renderer;
- enabling/disabling the plugin changes feature availability;
- renderer source files do not require edits when the feature plugin is installed;
- output matches the previous implementation.

---

## Phase 7: Introduce render graph contribution

If a render graph already exists, expose a restricted builder to extensions.

If not, implement a minimal graph with:

- named or semantic resources;
- pass read/write declarations;
- topological ordering;
- cycle detection;
- transient resource creation;
- deterministic execution;
- debug visualization;
- validation for missing resources.

Do not attempt a full production graph rewrite at once.

Start by wrapping existing pass order in graph nodes.

Migrate one pass chain at a time.

Required semantic resources may include:

```cpp
enum class RenderResourceSemantic {
    SceneDepth,
    SceneNormals,
    MotionVectors,
    HdrColor,
    LdrColor,
    GBufferAlbedo,
    GBufferNormals,
    GBufferMaterial,
    AccelerationStructure
};
```

Extensions should query semantics instead of concrete renderer members.

Acceptance criteria:

- an extension can add a pass based on resource dependencies;
- no hardcoded `beforeFog` or `afterLighting` hook is required for the selected migrated feature;
- missing required resources produce a readable diagnostic.

---

## Phase 8: Prove new renderable-object extensibility

Implement a test plugin for a unique volumetric object.

Suggested test object:

```cpp
TemporalDistortionVolumeComponent
```

Add only:

- scene component;
- extracted data type;
- extractor;
- renderer extension;
- plugin registration;
- shaders/resources;
- tests.

Do not modify:

- core `Scene`;
- `RenderFrame`;
- `RenderDataStore`;
- existing renderer class;
- renderer registry implementation;
- application loop.

Use this plugin as the architecture acceptance test.

Required tests:

- plugin disabled: object is unsupported or ignored with a diagnostic;
- plugin enabled: extractor channel appears;
- compatible renderer receives the extension;
- incompatible renderer reports unsupported status;
- rendering pass is inserted in the graph;
- plugin unload is safe if runtime unload is supported.

---

## Phase 9: Add a second renderer

Implement a minimal second renderer to validate renderer extensibility.

Prefer a small renderer before implementing a full path tracer:

- depth-only renderer;
- normal-visualization renderer;
- wireframe renderer;
- headless visibility renderer.

It must be added through:

- renderer implementation;
- descriptor;
- factory registration;
- renderer-specific extractors only where needed;
- compatible extensions.

Do not modify:

- existing renderer;
- scene;
- main loop;
- central factory;
- existing object types.

Once this works, implement the path-traced renderer.

---

## Phase 10: Add path-traced renderer

Implement:

```cpp
class PathTracingRenderer final : public Renderer;
```

Suggested capabilities:

```cpp
RayTracing
Compute
ParticipatingMedia
```

Add path-tracer-specific systems as plugins or renderer-owned services:

- geometry preparation;
- acceleration structure building;
- path-traced material conversion;
- light sampling data;
- accumulation buffers;
- denoising;
- tone mapping.

Reuse generic channels where appropriate:

- `MeshInstance`;
- `CameraData`;
- `AnalyticLightData`.

Add specialized channels only where necessary:

- `PathTracingMaterialData`;
- `ProceduralPrimitiveData`;
- `ParticipatingMediumData`.

Do not force path-tracer requirements into raster-renderer structures.

---

# 4. Integration Plugin Pattern

Use integration plugins to avoid direct dependencies between feature plugins and renderer plugins.

Example modules:

```text
TemporalDistortion.Core
TemporalDistortion.DeferredIntegration
TemporalDistortion.PathTracingIntegration

PathTracingRenderer.Core
DeferredRenderer.Core
```

Dependency direction:

```text
TemporalDistortion.DeferredIntegration
    -> TemporalDistortion.Core
    -> DeferredRenderer contracts or capability contracts
```

Prefer capability-based integration when no concrete renderer API is needed.

Use exact renderer integration only when the implementation requires renderer-specific services.

Rules:

- feature core plugin must not depend on every renderer;
- renderer core plugin must not depend on every feature;
- feature-renderer support belongs in an integration module;
- integration registration must be optional.

---

# 5. Compatibility and Support Reporting

Introduce explicit support reporting.

```cpp
enum class RenderSupportStatus {
    Supported,
    SupportedWithFallback,
    Unsupported
};

struct RenderSupportReport {
    RenderSupportStatus status;
    std::string reason;
};
```

The engine should be able to report:

- which extracted data types exist in a frame;
- which renderer extensions consume them;
- which object types are unsupported by the selected renderer;
- missing capabilities;
- missing graph resources;
- missing integration plugins.

Add a development diagnostics view or log output.

Do not silently ignore unsupported renderables by default.

---

# 6. Ordering and Dependency Metadata

Both extractors and renderer extensions need deterministic ordering.

Suggested registration metadata:

```cpp
struct ExtensionOrder {
    std::string phase;
    int priority;
    std::vector<std::string> after;
    std::vector<std::string> before;
};
```

Requirements:

- detect cycles;
- stable ordering across runs;
- diagnostics list resolved order;
- plugin load order must not accidentally determine behavior;
- graph dependencies should determine render-pass order wherever possible.

Use phase/priority only for orchestration not expressible through resource dependencies.

---

# 7. Ownership and Lifetime Rules

Define and document ownership explicitly.

Recommended rules:

- registries own factories and registration metadata;
- renderer instances own renderer extension instances;
- frame builder owns temporary extraction context;
- `RenderFrame` owns frame-local data channels;
- scene owns scene components;
- renderer resource registry owns GPU resources;
- extracted data uses stable asset/resource handles, not raw scene pointers;
- extensions may retain GPU resources but not frame-local references;
- no extension may retain references into a previous `RenderFrame`;
- plugin unload must invalidate factories before unloading code.

Add assertions for frame-lifetime misuse where feasible.

---

# 8. Threading Plan

Initial safe implementation:

```text
Scene update
  -> snapshot creation
  -> extraction
  -> rendering
```

Later parallelization:

```text
Scene update
  -> immutable snapshot
  -> parallel extractor jobs
  -> channel merge/finalization
  -> render thread
```

Prepare for parallel extraction by:

- avoiding global mutable extractor state;
- using per-extractor temporary buffers;
- merging channels deterministically;
- making resource resolution thread-safe or deferred;
- separating registration from frame execution.

Do not parallelize until the sequential architecture is stable and tested.

---

# 9. Performance Constraints

Track these risks during implementation:

- repeated type-ID hash lookups;
- many tiny allocations;
- virtual calls per renderable;
- copying large render data;
- rebuilding unchanged renderer resources;
- expensive scene snapshot creation;
- extension graph rebuild cost;
- capability predicate overhead.

Required design choices:

- virtual dispatch occurs per extractor/extension, not per renderable element;
- render data channels store contiguous data;
- frame allocators are supported;
- bulk extraction is preferred;
- channel lookup results may be cached inside extensions;
- GPU resource creation is cached across frames;
- scene and render handles are stable;
- future change-set extraction remains possible.

Add profiling markers around:

- snapshot creation;
- each extractor;
- channel finalization;
- each renderer extension;
- graph compilation;
- graph execution.

---

# 10. Testing Plan

## Unit tests

Add tests for:

- stable type ID registration;
- channel append/read;
- missing channel;
- duplicate channel type safety;
- renderer exact matching;
- capability matching;
- extractor ordering;
- extension ordering;
- dependency cycle detection;
- renderer registration;
- automatic extension installation;
- unsupported capability reporting.

## Integration tests

Add tests for:

- existing deferred renderer with migrated mesh extraction;
- existing renderer with optional extension disabled;
- existing renderer with optional extension enabled;
- unique volumetric object plugin;
- second renderer using generic extractors;
- renderer-specific extractor selection;
- unsupported renderable diagnostics;
- integration plugin discovery.

## Regression tests

Use available project infrastructure:

- image comparison;
- frame capture comparison;
- render command snapshot;
- GPU validation;
- scene loading tests;
- performance baselines.

For each migration step, compare against the pre-refactor renderer.

## Architecture tests

Add build or dependency tests where possible:

- renderer module cannot include scene headers;
- scene module cannot include renderer headers;
- render contracts cannot depend on concrete scene or renderer modules;
- feature core modules cannot depend on all renderers;
- no central object-type switch grows when test plugins are added;
- no central renderer switch grows when test renderers are added.

---

# 11. Acceptance Criteria

The migration is complete when all statements below are true.

## New renderable type

A new renderable type can be added using only:

- a scene component or scene-side representation;
- a render data type;
- an extractor;
- one or more renderer extensions;
- optional integration plugins;
- registration code;
- shaders/assets/tests.

No modification is required to:

- core scene class;
- renderer base class;
- render frame structure;
- typed data store;
- existing renderer classes;
- application main loop;
- central renderer factory;
- central render-object enum or union.

## New renderer

A new renderer can be added using only:

- renderer implementation;
- descriptor;
- factory registration;
- renderer-specific extractors where needed;
- renderer-specific extensions/integration plugins;
- tests and resources.

No modification is required to:

- scene system;
- existing renderers;
- main loop;
- central renderer switch;
- existing renderable object implementations.

## Existing renderer extension

A new effect or object integration can contribute passes without editing the renderer source.

## Dependency isolation

- renderer code does not include scene implementation headers;
- scene code does not include renderer implementation headers;
- scene objects do not call renderer methods;
- renderer does not traverse scene nodes or ECS components.

## Diagnostics

Unsupported combinations are explicit and inspectable.

---

# 12. Coding Agent Execution Rules

The coding agent should follow these rules throughout implementation:

1. Inspect before replacing. Reuse existing registries, reflection, allocators, render graph, ECS views, and plugin systems where suitable.
2. Keep the project compiling after every logical commit.
3. Prefer adapters over broad rewrites.
4. Do not introduce a second competing scene, renderer, or plugin architecture.
5. Migrate one renderable category at a time.
6. Preserve visual behavior before adding new capabilities.
7. Add tests with each abstraction.
8. Avoid renderer/object type switches in new code.
9. Do not use `dynamic_cast` as the primary extension mechanism.
10. Do not create a universal base `IRenderable::render(Renderer&)`.
11. Do not add new fixed fields to `RenderFrame` for plugin-defined object types.
12. Do not expose concrete renderer internals to general extensions.
13. Use integration plugins when feature and renderer modules should remain independent.
14. Record temporary compatibility layers and remove them after migration.
15. Document all new ownership, threading, and lifetime rules.

---

# 13. Recommended Commit Sequence

Use small, reviewable commits.

```text
1. Add architecture inventory document.
2. Add stable type ID and capability primitives.
3. Add typed RenderDataStore with tests.
4. Add RendererDescriptor and RendererMatch.
5. Add SceneSnapshot adapter.
6. Add extraction interfaces and registry.
7. Migrate camera/view extraction.
8. Migrate static mesh extraction.
9. Adapt existing renderer to consume RenderFrame.
10. Add RendererRegistry and register existing renderer.
11. Add renderer extension interfaces and registry.
12. Convert one optional feature into an extension.
13. Add render graph compatibility layer or restricted graph builder.
14. Add unique volumetric test plugin.
15. Add diagnostics for unsupported combinations.
16. Add minimal second renderer.
17. Add architecture dependency tests.
18. Remove obsolete scene-to-renderer paths.
19. Implement path-traced renderer plugin.
20. Remove remaining compatibility adapters.
```

Each commit should include:

- implementation;
- tests;
- migration notes;
- no unrelated formatting changes.

---

# 14. Deliverables

The coding agent should produce:

- architecture inventory;
- dependency diagram;
- render contracts module;
- typed render data store;
- scene snapshot/read view;
- extraction registry;
- renderer descriptor and capability system;
- renderer registry;
- renderer extension registry;
- render graph contribution API;
- migrated existing renderer;
- one migrated existing renderable category;
- one unique volumetric object plugin;
- one minimal second renderer;
- path-traced renderer implementation or staged skeleton;
- diagnostics tooling;
- unit and integration tests;
- migration documentation;
- removal plan for legacy paths.

---

# 15. Final Reference Example

A feature plugin:

```cpp
class TemporalDistortionPlugin final : public IEnginePlugin {
public:
    void install(
        EngineExtensionRegistry& registry
    ) override {
        registry.extractors.registerExtractor(
            std::make_unique<
                TemporalDistortionExtractor
            >(),
            {
                .name = "TemporalDistortionExtractor",
                .phase = ExtractionPhase::Volumes
            }
        );

        registry.rendererExtensions.registerFactory(
            RendererMatch::requires({
                RenderCapabilities::DeferredShading,
                RenderCapabilities::Compute
            }),
            [] {
                return std::make_unique<
                    DeferredTemporalDistortionExtension
                >();
            },
            {
                .name =
                    "DeferredTemporalDistortionExtension"
            }
        );

        registry.rendererExtensions.registerFactory(
            RendererMatch::requires({
                RenderCapabilities::RayTracing
            }),
            [] {
                return std::make_unique<
                    PathTracedTemporalDistortionExtension
                >();
            },
            {
                .name =
                    "PathTracedTemporalDistortionExtension"
            }
        );
    }
};
```

A renderer plugin:

```cpp
class PathTracingRendererPlugin final
    : public IEnginePlugin {
public:
    void install(
        EngineExtensionRegistry& registry
    ) override {
        registry.renderers.registerRenderer(
            "path-tracing",
            PathTracingRenderer::staticDescriptor(),
            [] {
                return std::make_unique<
                    PathTracingRenderer
                >();
            }
        );

        registry.extractors.registerExtractor(
            std::make_unique<
                PathTracingMaterialExtractor
            >(),
            {
                .name = "PathTracingMaterialExtractor",
                .phase = ExtractionPhase::Geometry
            }
        );
    }
};
```

Stable application code:

```cpp
auto renderer =
    engine.rendererRegistry().create(
        configuration.rendererName
    );

while (application.running()) {
    scene.update(deltaTime);

    const SceneSnapshot snapshot =
        scene.createSnapshot();

    RenderFrame frame =
        engine.renderFrameBuilder().build(
            snapshot,
            renderer->descriptor()
        );

    renderer->draw(frame);
}
```

The architecture is successful when adding another object plugin or renderer plugin does not require changes to this loop or to existing implementations.
