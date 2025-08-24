# SourceCraft 2 (SM 1.12 remake)

This folder contains a clean, modern scaffold for a SourceMod 1.12-compatible remake of SourceCraft. The legacy code in the repository is left untouched for reference.

What's here now
- A minimal core plugin (SourceCraft2.sp) using newdecls.
- A public API include (core/api.inc) for race/shop modules.
- Internal core implementation (core/core.inc) with stubs that will grow over time.
- An example race plugin (races/example_protoss_probe.sp) that demonstrates the registration workflow.

Build
- GitHub Actions compiles only this folder using a pinned SM 1.12 toolchain.
- Local: download SourceMod 1.12, then run:

```bash
sm/scripting/spcomp -i sm/scripting/include -i scripting/include -i scripting scripting/sourcecraft2/SourceCraft2.sp
sm/scripting/spcomp -i sm/scripting/include -i scripting/include -i scripting scripting/sourcecraft2/races/example_protoss_probe.sp
```

Next steps
- Implement CreateNative exposure for API so modules link at runtime.
- Port menu/cooldown/xp/energy/economy systems from legacy behavior.
- Add translation search-order parity (sc.*.phrases.txt, fallback *.phrases.txt, w3s.race.*.phrases.txt).
- Add optional integrations (jetpack/grapple/etc.) gated by optional natives.