# LCD Math v0.0.1 — natural LCD scientific calculator for Casio Graph Math+

LCD Math is a standalone, real-valued scientific calculator add-in for the Casio Graph Math+ running Math+ Mod (MPM). Its green LCD appearance was developed using official Casio display references and a photograph of a physical fx-85GT CW. This is the first GitHub release, using the requested `lcd-math-v0.0.1.g3a` filename convention. Future builds increment that version; earlier local 1.0.x prototypes are preserved separately.

## Download and install

1. Download **lcd-math-v0.0.1.g3a** from this release. The accompanying `.sha256` file can verify the download.
2. Copy the G3A to the root of the calculator's USB storage on an existing working MPM installation.
3. Remove older LCD Math G3A files from the calculator to avoid duplicate launcher entries. Keep `lcdmath.dat` and `lcdmath.bak` to retain saved settings, memories and history.
4. Eject the USB storage safely, then launch **LCD Math** from **TOOLS**.

No companion asset pack is needed. The target baseline is Graph Math+ OS **02.00.2202** with MPM already installed. This release does not install or update MPM or calculator firmware.

## Scientific calculations

- Arithmetic with normal precedence, parentheses, implicit multiplication, percentages, factorials, reciprocals, powers and square/cube/nth roots.
- Trigonometric and inverse trigonometric functions in degrees, radians or gradians; hyperbolic and inverse hyperbolic functions.
- Natural, common and arbitrary-base logarithms; exponentials, pi and Euler's number.
- Combinations, permutations, gcd/lcm, remainder, min/max, floor/ceiling/rounding, the standard normal CDF and binomial probability mass function.
- Supported exact answers are reduced fractions, single surds and rational multiples of pi. FORMAT toggles exact/decimal output; other results use decimals.
- Double precision internally, up to 10 significant decimal digits in normal output, and FIX/SCI display settings with 0–9 decimal places.
- Ans continuation, nine numerical memories (A–F, X, Y, Z), a function catalog, and a 20-calculation editable history. Expressions support up to 111 characters.

## Natural fraction entry — new in this build

The dedicated fraction key previously inserted a division operator. It now creates a real numerator/denominator template.

- Empty fields display hollow placeholder boxes.
- Enter the numerator, press **DOWN** (or **RIGHT** at its end), enter the denominator, then press **RIGHT** to leave the fraction.
- **UP** returns to the numerator. Field navigation retains the horizontal position where possible.
- Pressing the fraction key after a value captures that value as the numerator.
- Fractions can be nested inside either field and inside roots, powers and function arguments.
- Memory recalls and Ans can be inserted into template fields without leaving an invalid placeholder behind.
- The ordinary division key now displays **÷**, independently of stacked fraction entry.

Example: **fraction → 1 → DOWN → 3 → RIGHT → EXE** gives an exact stacked **1/3**.

## Powers and exponent placeholders — new in this build

- The power key creates a raised hollow exponent box instead of showing caret syntax.
- Exponents render as true superscripts, including negative and fractional exponents and powers nested inside other powers.
- **RIGHT** or **DOWN** leaves the exponent. **UP**, immediately after a completed power, re-enters it.
- Empty exponent templates can be removed with DEL.
- A square, power, percentage or factorial applied after leaving a fraction now applies to the **entire fraction**. Previously it could bind only to the denominator. For example, squaring 1/2 now displays `(1/2)²` and evaluates to 1/4.

## Brackets, roots and other mathematical symbols — improved

- Brackets use narrow curved strokes and grow to enclose tall expressions, avoiding full-character-width gaps around fractions.
- Fraction bars and centered numerators/denominators use measured layout bounds and consistent spacing.
- Square-root ticks, stems and overbars join continuously and grow with their radicand, including fractions and nested roots.
- Cube and nth roots display their root index.
- Inverse trig and inverse hyperbolic functions use a raised **−1**.
- Exponentials display as **e** with a superscript; arbitrary-base logarithms display a subscript base.
- Pi uses a curved lowercase symbol. Multiplication, division, plus, minus and equality signs have dedicated mathematical strokes.
- Implicit multiplication after a fraction preserves the grouping used by the numerical evaluator.
- Tall expressions keep their natural structure, shrink where appropriate, and scroll to the active field instead of falling back to a flat text representation.
- Grouping and function keys create closing brackets automatically; entering a closing bracket steps over an existing one. Cursor movement respects function and exponent boundaries.

## LCD appearance and persistent settings

- Warm olive-green LCD palette, taller dot-matrix numerals, an expression near the upper left and a right-aligned answer near the lower right.
- Main calculation screen contains only the expression, result and normal calculator status indicators: no branding, shortcut bar, tooltips or help text.
- **SETTINGS/TOOLS → Contrast** provides 11 levels (0–10), a live formula preview and four LCD shade samples. Left makes it lighter; right makes it darker.
- HOME saves contrast, angle/output settings, history and memories to `lcdmath.dat`, retaining a previous valid save as `lcdmath.bak`. Version/checksum validation supports recovery from a damaged primary save. Unevaluated input is not saved.

Full controls and examples: [KEYBINDS.md](https://github.com/cis5123467-sudo/lcd-math-casio/blob/main/KEYBINDS.md).

## Verification performed before publication

The exact attached G3A was executed in **emuCasioCalc's fx-CG50 model** after rebuilding from the renamed `lcd-math-casio` folder.

- **66 numerical/domain cases**, plus editor, template, memory, history, output-format, persistence and corrupt-save recovery assertions, passed under host AddressSanitizer and UndefinedBehaviorSanitizer.
- **27 layout cases** passed bounds, superscript-caret, numerator/denominator-caret, symbolic-function and implicit-grouping checks under the same sanitizers.
- **29 released-binary scenarios** passed with saved numerical values asserted and screenshots captured: fraction templates/capture/sums, nested fractions, denominator fractions, tall brackets, square/nested/indexed roots, roots containing fractions, arbitrary-base logs, exponent placeholders, negative/fractional/nested powers, bracketed and fraction bases, inverse trig, pi, ordinary division, scientific notation, negative fractions, decimal denominators, fraction editing, empty-template deletion, exponentials, memory values in fractions and trig arguments containing fractions.
- General SuperH regression checks passed for arithmetic, exact/decimal conversion, automatic brackets, factorials, Ans, editing/history reuse, error correction, menus, angle modes, memory, help, scrolling, HOME exit and identical restored calculation screens.
- Contrast changed live at both endpoints, persisted across independent launches, and the main screen remained free of shortcut/help text.
- MPM v1.1's actual discovery/metadata functions recognised the add-in name, version and icon blocks. G3A signatures, lengths and checksums were verified.

Logs, screenshots, decoded results and reproducible test scripts are included in the source repository. The toolchain is SH GCC 15.2.0, fxSDK dev a6e46eb6, Math+-aware gint dev 463f180f, fxlibc dev b96335c2 and OpenLibm 9828d2e3.

**Physical hardware status:** Graph Math+ startup, named-key behaviour, display readability and save/exit/relaunch testing remain pending on a real unit. The emulator tests establish SuperH execution and the tested fx-CG50 environment; they do not establish physical Graph Math+ compatibility by themselves. Display revision and exact installed MPM build are not established.

## Scope and source

This app evaluates real-valued scientific expressions. It does not reproduce separate graphing, complex-number, matrix, vector, spreadsheet or statistics-editor applications, and is not Casio firmware or an exact clone of the fx-85GT CW.

The repository contains all application source, font/icon assets, build scripts, tests, documentation and bundled dependency notices. The local folder and project references now use **lcd-math-casio**. The calculation engine and initial textbook renderer were adapted from the user's existing NumAI-MPM project; dependency notices are retained under `vendor/`. Research images are linked in the research notes; downloaded reference photographs are not embedded in the add-in.
