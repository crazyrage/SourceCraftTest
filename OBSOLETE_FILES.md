# Obsolete Files Documentation

This document catalogs obsolete and deprecated files in the SourceCraft codebase that should be reviewed for removal or archival.

## Summary

- **Total obsolete files found:** 32
- **Directories containing obsolete files:** 3
- **Recommendation:** Review and archive or remove these files

## Obsolete Directories

### 1. `/scripting/obsolete/` (27 files)

Contains old plugins and includes that have been replaced or are no longer used:

#### Old Plugins:
- `Amplifier.sp` / `Amplifier.inc` / `Amplifier.txt` - Old amplifier system
- `TF2_EquipmentManager.sp` - Replaced equipment manager (72KB)
- `armor.sp` - Old armor system
- `beetlemenu.sp` - Old beetle menu implementation
- `buildlimit.sp` / `buildlimit.inc` - Old build limit system
- `fairteambalancer.sp` - Old team balancing plugin
- `fakegift.sp` - Old fake gift implementation
- `flareextinguisher.sp` - Old flare extinguisher plugin
- `redirector.sp` - Old server redirector
- `repairnode.sp` - Old repair node plugin
- `tftools.sp` / `tftools.inc` - Old TF2 tools

#### Old Libraries/Includes:
- `ammo.inc` - Old ammo management
- `authtimer.inc` - Old authentication timer
- `db_ext.inc` - Old database extensions
- `dbfactions.inc` - Old database factions code
- `defcolors.inc` - Old color definitions
- `dump.inc` - Old dump utility
- `log.inc` - Old logging system
- `saytext2.inc` - Old say text system
- `screen.inc` - Old screen/HUD code
- `strtoken.inc` - Old string tokenization
- `util.inc` - Old utility functions

#### Old TF2-Specific:
- `tf2_optional.inc` - Optional TF2 features
- `tf2_ignite.inc` - Old TF2 ignite system
- `tf2_particle.inc` - Old TF2 particle system

### 2. `/scripting/SourceCraft/obsolete/` (5 files)

Contains old race implementations:

- `Protoss.sp` (27 files) - Old Protoss race implementation
- `Zerg.sp` (14 files) - Old Zerg race implementation
- `TerranConfederacy.sp` (7 files) - Old Terran Confederacy race
- `settings.inc` - Old settings include
- `display_flags.inc` - Old display flags

**Note:** These have been replaced by individual race files (e.g., ProtossZealot.sp, ZergHydralisk.sp, etc.)

### 3. `/gamedata/obsolete/` (7 files)

Contains old game data configurations:

- `hooker.games.txt` - Old hooker game configuration
- `hlstatsx.sdktools.txt` - Old HLStatsX SDK tools config
- `nican.offsets.txt` - Old NICAN offsets
- `plugin.hgrsource.txt` - Old HGR source plugin config
- `givenameditem.games.txt` - Old give named item config
- `takedamage.txt` - Old take damage configuration
- `plugin.sourcecraft.txt` - Old SourceCraft plugin config

**Note:** These have likely been superseded by files in the main `/gamedata/` directory.

## Recommendations

### Immediate Actions:

1. **Archive obsolete files** - Move to a separate `archive/` directory or a separate branch
   ```bash
   git checkout -b archive-obsolete-files
   mkdir -p archive/scripting
   mkdir -p archive/gamedata
   mv scripting/obsolete archive/scripting/
   mv scripting/SourceCraft/obsolete archive/scripting/SourceCraft/
   mv gamedata/obsolete archive/gamedata/
   git add archive/
   git commit -m "Archive obsolete files for historical reference"
   ```

2. **Document replacements** - Create a mapping of old files to their replacements:

| Old File | Replacement | Notes |
|----------|-------------|-------|
| `obsolete/Protoss.sp` | Individual race files (ProtossZealot.sp, etc.) | Split into separate races |
| `obsolete/Zerg.sp` | Individual race files (ZergHydralisk.sp, etc.) | Split into separate races |
| `obsolete/TerranConfederacy.sp` | TerranMarine.sp, TerranGhost.sp, etc. | Split into separate races |
| `obsolete/TF2_EquipmentManager.sp` | Built into main SourceCraft.sp | Integrated functionality |
| `obsolete/buildlimit.*` | sc/engine/*.inc | Integrated into core engine |
| `obsolete/db_ext.inc` | sc/engine/db.inc | Replaced with modern DB code |

3. **Verify no dependencies** - Ensure no active code references these files:
   ```bash
   # Search for includes of obsolete files
   grep -r "obsolete/" scripting/ --include="*.sp" --include="*.inc"
   ```

4. **Clean up include paths** - Remove any `#include` statements that reference obsolete files

### Long-term Actions:

1. **Create historical documentation** - Document why these files were replaced
2. **Tag a release** - Before removing, tag the last version that contained these files
3. **Update build scripts** - Remove references to obsolete files from any build/compile scripts
4. **Update README** - Document the modernization effort

## Migration Path

If you need functionality from any obsolete file:

1. **Check the replacement** - Most functionality has been integrated into newer files
2. **Review commit history** - `git log --follow <obsolete-file>` to understand the migration
3. **Consult replacement files** - Look for similar patterns in active codebase

## Verification Commands

```bash
# List all files in obsolete directories
find . -path "*/obsolete/*" -type f

# Check for any active references to obsolete files
grep -r "obsolete/" . --include="*.sp" --include="*.inc" --exclude-dir=obsolete

# Count obsolete files
find . -path "*/obsolete/*" -type f | wc -l
```

## Status

- [ ] Obsolete files reviewed
- [ ] Dependencies verified (none found)
- [ ] Files archived
- [ ] Documentation updated
- [ ] Build scripts updated
- [ ] Final cleanup complete

## Notes

- These files are kept in the repository currently for historical reference
- They may contain useful code patterns or logic that could be referenced
- Before deletion, ensure all functionality has been migrated or is no longer needed
- Consider creating a separate "archive" branch before removing from main development branch

## Date

Documented: 2025-11-03
Status: Pending review and archival
