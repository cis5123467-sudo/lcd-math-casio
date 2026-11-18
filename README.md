# LCD Math v0.0.3 for Graph Math+

A real-valued scientific calculator add-in with a natural textbook display and an olive LCD appearance based on Chris’s physical fx-85GT CW. It draws stacked fractions, mixed fractions, superscript powers and their empty boxes, scalable brackets and radicals. Contrast has eleven adjustable levels. Fixed glass/pixel grain and lightly shaded characters preserve an LCD appearance. The calculation screen contains only the expression, answer and normal status indicators, now with slightly darker backgrounds behind active icons.

[Download the latest release](https://github.com/cis5123467-sudo/lcd-math-casio/releases/latest) · [Complete controls and worked examples](KEYBINDS.md) · [Release notes](https://github.com/cis5123467-sudo/lcd-math-casio/releases/tag/v0.0.3)

## Install

Download `lcd-math-v0.0.3.g3a`, copy it to the root of USB storage, eject and launch **LCD Math** from **TOOLS** on the existing MPM installation. No companion assets are required. Target: Chris’s working **Graph Math+, OS 02.00.2202**. Physical display revision and exact installed MPM build are not established.

## Changes in v0.0.3

- Central mapping for all 50 Graph Math+ keys and their primary, SHIFT and ALPHA layers, audited against Casio’s [official keypad photograph](https://www.casio-education.fr/wp-content/uploads/2024/03/graph-math-produit.png) and [OS 2.00 manual](https://www.casio.com/content/dam/casio/global/support/manuals/calculators/pdf/2022/g/GRAPH_MATH%2B_Soft_v200_FR.pdf). Full source references and the complete physical table are in [KEYBINDS.md](KEYBINDS.md).
- All A–Z letters, r and θ; ALPHA lock; literal spaces/quotes; variable assignment using the printed → shortcut. The x,θ,T,Y key enters x and reads stored X in Calculate.
- Correct SHIFT actions for nth roots, arbitrary-base logarithms, log, ln, inverse sin/cos/tan, π, unary (-), Ans and approximate evaluation. Root index/log base entry precedes the main argument. Factorial is in CATALOG.
- Mixed-fraction entry, FORMAT choices for decimal/exact, mixed/improper, °′″ and engineering notation; explicit radian and sexagesimal entry.
- Numbered shortcuts on every selectable menu: **1–9, then A–Z**, proceeding down the list. Use the alphabet-labelled keys directly for A–L and ALPHA for M–Z. CATALOG’s More submenu keeps all entries accessible. SETTINGS, VARIABLE, FORMAT, TOOLS and Copy/Cut use the same convention.
- App-local selection/copy/cut/paste, operand insertion capture and BMP screen capture on their printed SHIFT keys. Correct named-key help inside the app.
- Slightly darker LCD backing behind each active status icon. The natural mathematical layout, adjustable contrast and clean calculation screen are retained.
- Versioned saves now retain all 28 variables and migrate earlier nine-variable saves without losing history or contrast.

## Quick use

Enter a calculation and press **OK/EXE**. Operators after an answer continue with Ans; digits start fresh input. **←/→** moves the cursor, **⌫ (DEL)** deletes, **AC/↩** clears, and **↑/↓** selects template fields or history. **HOME** saves and exits.

**□/□ → 1 → ↓ → 3 → → → EXE** gives a stacked 1/3. **2 → □^□ → 3 → → → EXE** gives 8; the exponent starts as a raised empty box. **√□ → 8 → EXE** gives 2√2. **SHIFT → √□ → 5 → → → 3 → 2 → EXE** gives the fifth root of 32. Long expressions scroll; nested fractions and powers retain their structure.

Open **CATALOG** for other functions. **SETTINGS → 5** opens Contrast: use **←/→**, then **OK/EXE** to return. **FORMAT** opens result presentation choices. **SHIFT → FORMAT** inserts Ans; **SHIFT → EXE (≈)** requests an approximate result. **TOOLS → 1** opens the full key guide.

For memory, enter **value → variable → EXE**. The arrow is **SHIFT → x,θ,T,Y**; use ALPHA plus the printed letter key or select a variable from **VARIABLE**. There are 28 variables: A–Z, r, θ. The complete table, menu shortcuts and examples are in [KEYBINDS.md](KEYBINDS.md).

## Calculation scope

Arithmetic, parentheses, implicit multiplication, powers, square/cube/nth roots, reciprocal, absolute value, percentages, factorials; trigonometric and inverse trigonometric functions in degrees/radians/gradians; logarithms and exponentials; hyperbolic functions and their inverses; combinations/permutations, gcd/lcm, remainder, min/max, rounding, normal CDF and binomial probability. Calculations use double precision, with reduced fractions, single surds and rational multiples of π as supported exact forms. Expressions hold up to 111 internal characters; history holds 20 results. Invalid domains and non-finite results report errors.

There are no separate complex, matrix, vector, graphing, spreadsheet, equation or statistics-editor applications. Their printed characters can be entered; unsupported complex, matrix and quoted-text evaluations report explicit errors. Clipboard and capture are this app’s implementations, described in the key guide.

## Saved files

HOME saves history, memories, contrast and standard output preferences to `lcdmath.dat`. A previous valid save is retained as `lcdmath.bak`; `lcdmath.tmp` is used during replacement. Version and checksum validation permit backup recovery. Earlier saves migrate automatically. Unevaluated input, the clipboard and temporary engineering/sexagesimal presentation are not saved. **SHIFT → ×10^□** writes/replaces `lcdshot.bmp` in USB storage.

## Build

Clone the repository and build from its folder:

```sh
git clone https://github.com/cis5123467-sudo/lcd-math-casio.git
cd lcd-math-casio
python3 tools/build.py
```

The existing fxSDK/gint toolchain is required. Set `CASIO_SDK` to the fxSDK installation prefix if needed; `CMAKE` can select the CMake executable. This project uses fxSDK's colour CMake toolchain, the `Gint::Gint` target, and `GenerateG3A`. The small local `FindGint.cmake` resolves this installation's libraries through `fxsdk path` because its sysroot is outside GCC's default library path. The build creates the local `release/` directory. Icons and font data are already generated; `tools/assets.py` regenerates them with Pillow.

Known build: SH GCC **15.2.0**, fxSDK dev **a6e46eb6**, gint dev **463f180f** (reported version 2.11.0), fxlibc dev **b96335c2**, OpenLibm **9828d2e3**. These are the Math+-aware toolchain revisions used by the existing local NumPlay and NumAI ports. The calculation engine and textbook layout started from Chris's local NumAI-MPM project. The display, editor integration, catalog, controls, contrast and save format are specific to this add-in. The fxlibc printf adaptation and linked libraries' notices are under `vendor/`.

## Verification

```sh
python3 tools/test.py
# Runtime suites require Pillow and the local emuCasioCalc build:
python3 tools/smoke.py
python3 tools/test_rendering.py
python3 tools/test_keyboard_runtime.py
python3 tools/test_lcd_texture.py
python3 tools/check_mpm.py /path/to/mpm release/lcd-math-v0.0.3.g3a
```

Host checks use AddressSanitizer and UndefinedBehaviorSanitizer: 80 numerical/domain cases plus editing, exact forms, history, formats and save recovery; 32 layout bounds/caret/symbol cases; and 50 physical keys across three modifier layers, letters, printed SHIFT actions, ALPHA lock, option shortcuts and Unicode glyphs.

The actual release binary runs in **emuCasioCalc’s fx-CG50 model**, driving corresponding physical matrix positions through the Graph Math+ mapping. Runtime suites cover arithmetic, persistence, editing/error recovery, menus, contrast limits, all A–Z assignments, x/X alias, r/θ, clipboard, capture, save migration and 29 dedicated mathematical rendering cases. Texture checks cover stable grain, character shading, readable contrast and active status backplates. MPM v1.1’s discovery/metadata C routines recognise the name, version and icon blocks; G3A header/checksum inspection is also performed.

These checks do not emulate Graph Math+ hardware or establish physical compatibility. Startup, physical named-key operation, OFF/resume, readability and save/relaunch remain to be checked on Chris’s target calculator. OFF uses gint’s power-off API on the target; the emulator path saves and exits.

The current release is `lcd-math-v0.0.3.g3a`; the next is `lcd-math-v0.0.4.g3a`. Only `.g3a` builds go in local `release/`. Builds are published through GitHub Releases; the source tree contains application code, assets, build/test tools, dependency notices and user documentation. Local assistant instructions, queues, research, release drafts, previews and generated verification output remain ignored by Git.
