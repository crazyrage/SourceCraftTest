# SourceCraft 1:1 Clean-Room Remake Plan

Goal: Recreate SourceCraft behavior on SourceMod 1.12 with modern SourcePawn, preserving gameplay and content while keeping the original sources intact for reference.

Phases
- P0: Scaffold + CI (this PR)
- P1: Core systems (player state, XP/levels, crystals/vespene, energy, cooldowns, menus, config loader, translations)
- P2: Public natives via CreateNative + module wiring; shop items API
- P3: Port priority races (ProtossProbe, Battlecruiser, TerranSCV, ScienceVessel, Chimera)
- P4: Shop items and economy parity
- P5: Optional integrations (jetpack/grapple/sidewinder/etc.) via optional natives
- P6: Cross-game support (DoD/CS/CS:GO) using game-specific event gates
- P7: Parity testing & documentation

Acceptance
- CI builds with SM 1.12.
- Same player-facing behavior for abilities and progression as legacy.
- Reuse of legacy translations/configs whenever possible.