# SourcePawn 1.6 → 1.7+ Syntax Migration Guide

## Overview
This guide provides patterns and sed commands to help migrate SourceCraft from deprecated SourcePawn 1.6 syntax to modern 1.7+ syntax.

**Compatibility:** The new syntax works with:
- ✅ SourceMod 1.7 through **1.12+** (and all future versions)
- ✅ Latest SourcePawn compilers
- ✅ Modern MetaMod: Source (1.11, 1.12+)

**Why migrate?**
- Old syntax (new String:, etc.) generates warnings/errors in SourceMod 1.11+
- New syntax is the official standard since 2014
- Future-proof for upcoming SourceMod releases

## Quick Reference Table

| Old Syntax (1.6) | New Syntax (1.7+) | Example |
|------------------|-------------------|---------|
| `new String:var[]` | `char var[]` | `char buffer[256]` |
| `new Float:var` | `float var` | `float damage = 10.0` |
| `new bool:var` | `bool var` | `bool isActive = false` |
| `new Handle:var` | `Handle var` | `Handle timer = INVALID_HANDLE` |
| `decl String:var[]` | `char var[]` | `char name[64]` |
| `new var` | `int var` | `int count = 0` |
| `public Plugin:myinfo` | `public Plugin myinfo` | Plugin info struct |
| `public Action:Func()` | `public Action Func()` | Function with Action return |
| `public Func(client)` | `public void Func(int client)` | Void function |
| `Handle:param` | `Handle param` | Function parameter |
| `String:param[]` | `char param[]` | Function parameter |
| `bool:param` | `bool param` | Function parameter |
| `Float:param` | `float param` | Function parameter |
| `const String:var[]` | `const char var[]` | Constant string |

## Automated Migration with sed

### Basic Variable Declarations

```bash
# Replace new String: with char
sed -i 's/new String:/char/g' *.sp

# Replace new Float: with float
sed -i 's/new Float:/float/g' *.sp

# Replace new bool: with bool
sed -i 's/new bool:/bool/g' *.sp

# Replace new Handle: with Handle
sed -i 's/new Handle:/Handle/g' *.sp

# Replace decl String: with char
sed -i 's/decl String:/char/g' *.sp

# Replace standalone 'new ' with 'int ' (be careful with this one!)
# This requires manual review as it may catch unwanted patterns
sed -i 's/\bnew \([a-zA-Z_][a-zA-Z0-9_]*\)\b/int \1/g' *.sp
```

### Function Signatures

```bash
# Replace public Plugin: with public Plugin
sed -i 's/public Plugin:/public Plugin/g' *.sp

# Replace public Action: with public Action
sed -i 's/public Action:/public Action/g' *.sp

# Replace public bool: with public bool
sed -i 's/public bool:/public bool/g' *.sp

# Replace public Float: with public float
sed -i 's/public Float:/public float/g' *.sp
```

### Function Parameters

```bash
# Replace Handle: in function parameters
sed -i 's/Handle:/Handle /g' *.sp

# Replace String: in function parameters
sed -i 's/String:/char /g' *.sp

# Replace bool: in function parameters
sed -i 's/bool:/bool /g' *.sp

# Replace Float: in function parameters
sed -i 's/Float:/float /g' *.sp
```

### Add void return type to public functions without return type
```bash
# This regex adds 'void ' after 'public ' if there's no return type
# Requires manual review!
sed -i 's/public \([A-Z][a-zA-Z]*\)(/public void \1(/g' *.sp
```

## Manual Migration Steps

### Step 1: Backup Your Files
```bash
# Create a backup branch
git checkout -b backup-before-migration
git add .
git commit -m "Backup before syntax migration"

# Create migration branch
git checkout -b syntax-migration-1.7
```

### Step 2: Migrate Global Variables

**Before:**
```sourcepawn
new const String:mdlPackage[][] = { "model1.mdl", "model2.mdl" };
new String:g_URL[256] = "http://example.com";
new bool:g_bLoaded = false;
new Float:g_fDamage = 10.0;
new g_iCount = 0;
new Handle:g_hTimer = INVALID_HANDLE;
```

**After:**
```sourcepawn
char mdlPackage[][] = { "model1.mdl", "model2.mdl" };
char g_URL[256] = "http://example.com";
bool g_bLoaded = false;
float g_fDamage = 10.0;
int g_iCount = 0;
Handle g_hTimer = INVALID_HANDLE;
```

### Step 3: Migrate Function Signatures

**Before:**
```sourcepawn
public Plugin:myinfo =
{
    name="My Plugin",
    author="Author",
    description="Description",
    version="1.0",
    url="http://example.com"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    return APLRes_Success;
}

public OnPluginStart()
{
    // code
}

public Action:Command_Test(client, args)
{
    return Plugin_Handled;
}
```

**After:**
```sourcepawn
public Plugin myinfo =
{
    name = "My Plugin",
    author = "Author",
    description = "Description",
    version = "1.0",
    url = "http://example.com"
};

public APLRes AskPluginLoad2(Handle myself, bool late, char error[], int err_max)
{
    return APLRes_Success;
}

public void OnPluginStart()
{
    // code
}

public Action Command_Test(int client, int args)
{
    return Plugin_Handled;
}
```

### Step 4: Migrate Local Variables

**Before:**
```sourcepawn
public void SomeFunction(int client)
{
    new String:name[64];
    new String:buffer[256];
    decl String:temp[128];
    new Handle:timer;
    new Float:pos[3];
    new bool:found = false;
    new i;

    for (new x = 0; x < 10; x++)
    {
        // code
    }
}
```

**After:**
```sourcepawn
public void SomeFunction(int client)
{
    char name[64];
    char buffer[256];
    char temp[128];
    Handle timer;
    float pos[3];
    bool found = false;
    int i;

    for (int x = 0; x < 10; x++)
    {
        // code
    }
}
```

## Common Pitfalls

### 1. Plugin Info Struct
**Wrong:**
```sourcepawn
public Plugin:myinfo=  // Missing space after Plugin
{
    name="Test",       // Old style, no spaces
    ...
};
```

**Correct:**
```sourcepawn
public Plugin myinfo =  // Space before myinfo, space before =
{
    name = "Test",      // Spaces around =
    ...
};
```

### 2. Don't Touch Already-Correct Syntax
Some files may already be partially migrated. Watch for:
- `Handle variable = INVALID_HANDLE;` (already correct!)
- `int count;` (already correct!)
- Function signatures that already have types

### 3. Array Syntax
Both old and new syntax support arrays the same way:
```sourcepawn
// Both work the same:
char buffer[256];
char names[10][64];
float vectors[MAXPLAYERS][3];
```

### 4. Type Tags in Casts
Old syntax used type tags in casts, new syntax doesn't always need them:
```sourcepawn
// Old
new Float:value = Float:GetEntProp(entity, Prop_Data, "m_flValue");

// New
float value = view_as<float>(GetEntProp(entity, Prop_Data, "m_flValue"));
// or often just:
float value = GetEntPropFloat(entity, Prop_Data, "m_flValue");
```

## Testing After Migration

After migrating files, always:

1. **Compile the plugin:**
   ```bash
   spcomp scripting/SourceCraft/SourceCraft.sp
   ```

2. **Check for warnings:**
   - Modern compilers may still warn about deprecated syntax
   - Fix any compilation errors

3. **Test in-game:**
   - Load the plugin
   - Test basic functionality
   - Check for runtime errors in logs

4. **Verify consistency:**
   - Ensure all files in a module use consistent syntax
   - Update any related include files

## Migration Priority

Suggested order for migrating SourceCraft:

1. ✅ **Core files:** (Done)
   - SourceCraft.sp

2. **Critical systems:**
   - ShopItems.sp
   - Race management includes

3. **Race files by faction:**
   - Terran races (11 files)
   - Protoss races (13 files)
   - Zerg races (14 files)
   - War3Source races (20+ files)

4. **Library files:**
   - Include files in sc/
   - Utility libraries in lib/

5. **Optional plugins:**
   - Standalone plugins
   - Test files

## Batch Migration Script

Create a script `migrate_syntax.sh`:

```bash
#!/bin/bash

# Batch migrate all .sp files in a directory
# Usage: ./migrate_syntax.sh scripting/SourceCraft/

DIR=${1:-.}

echo "Migrating files in: $DIR"

find "$DIR" -name "*.sp" -type f | while read file; do
    echo "Processing: $file"

    # Backup
    cp "$file" "$file.bak"

    # Apply migrations
    sed -i 's/new const String:/char /g' "$file"
    sed -i 's/new String:/char /g' "$file"
    sed -i 's/new Float:/float /g' "$file"
    sed -i 's/new bool:/bool /g' "$file"
    sed -i 's/new Handle:/Handle /g' "$file"
    sed -i 's/decl String:/char /g' "$file"

    sed -i 's/public Plugin:/public Plugin /g' "$file"
    sed -i 's/public Action:/public Action /g' "$file"
    sed -i 's/public bool:/public bool /g' "$file"

    # Parameters
    sed -i 's/\bHandle:\([a-zA-Z_][a-zA-Z0-9_]*\)/Handle \1/g' "$file"
    sed -i 's/\bString:\([a-zA-Z_][a-zA-Z0-9_]*\[\]/char \1/g' "$file"
    sed -i 's/\bbool:\([a-zA-Z_][a-zA-Z0-9_]*\)/bool \1/g' "$file"
    sed -i 's/\bFloat:\([a-zA-Z_][a-zA-Z0-9_]*\)/float \1/g' "$file"

    echo "  Done: $file"
done

echo "Migration complete!"
echo "Review changes and run: git diff"
```

Make it executable:
```bash
chmod +x migrate_syntax.sh
```

## Verification Commands

```bash
# Find remaining old syntax patterns
grep -r "new String:" scripting/
grep -r "new Float:" scripting/
grep -r "new bool:" scripting/
grep -r "new Handle:" scripting/
grep -r "decl String:" scripting/
grep -r "public Plugin:" scripting/
grep -r "public Action:" scripting/

# Count occurrences
grep -r "new String:" scripting/ | wc -l
grep -r "new Float:" scripting/ | wc -l
```

## Files Already Migrated

- ✅ scripting/SourceCraft/SourceCraft.sp
- ✅ scripting/SourceCraft/ZergQueen.sp (partial)

## Notes

- The new syntax is backward compatible with SourceMod 1.7+
- Old syntax may fail on SourceMod 1.11+ compilers
- Always test after migration
- Keep backups of original files
- Migrate in batches and commit frequently

## References

- [SourcePawn Transitional Syntax Documentation](https://wiki.alliedmods.net/SourcePawn_Transitional_Syntax)
- [SourceMod 1.7+ API Changes](https://wiki.alliedmods.net/SourceMod_1.7_API_Changes)
- [SourceMod Downloads](https://www.sourcemod.net/downloads.php) - Get SourceMod 1.12+ (latest)
- [MetaMod: Source Downloads](https://www.sourcemm.net/downloads.php) - Get MetaMod 1.12+ (latest)

## Version Compatibility

| SourceMod Version | Old Syntax Support | New Syntax Support | Status |
|-------------------|-------------------|-------------------|--------|
| 1.6 and earlier | ✅ Yes | ❌ No | Legacy |
| 1.7 - 1.10 | ⚠️ Deprecated | ✅ Yes | Transitional |
| 1.11 - 1.12+ | ❌ Warnings/Errors | ✅ Yes | **Current** |

**Bottom line:** Migrating to new syntax ensures compatibility with SourceMod 1.12 and all future releases.
