# LCD Math v0.0.3: Graph Math+ controls

Target: **Graph Math+, OS 02.00.2202**. Key legends were checked against [Casio’s official Graph Math+ keypad photograph](https://www.casio-education.fr/wp-content/uploads/2024/03/graph-math-produit.png), the [OS 2.00 software manual](https://www.casio.com/content/dam/casio/global/support/manuals/calculators/pdf/2022/g/GRAPH_MATH%2B_Soft_v200_FR.pdf) and the [hardware manual](https://www.casio.com/content/dam/casio/global/support/manuals/calculators/pdf/2022/g/GRAPH_MATH%2B_Hard_FR.pdf). Relevant printed software pages: 5 (key layers), 9 (ALPHA lock and clipboard), 11–12 (menu option shortcuts), 18 (28 variables and x/X), 19–20 (scientific entry), 217 (factorial through CATALOG).

This is an audit of the official physical keypad and manuals, with software and emulator regression checks. Physical keypresses on a Graph Math+ have not been tested here. The calculator engine supports real-valued scientific calculation; mapping a key does not add the operating system’s other applications.

## Start and modifiers

Copy `lcd-math-v0.0.3.g3a` to the root of the calculator’s USB storage, eject it and launch LCD Math from **TOOLS** in the existing MPM installation. Type an expression, then press **OK** or **EXE**. After an answer, a number starts a new expression and an operator continues using **Ans**. **HOME** saves and exits from every view.

Press and release **SHIFT** or **ALPHA**, then the desired key. Each normally applies once; pressing it again cancels. **SHIFT → ALPHA** enables **A-LOCK** for repeated letters. **ALPHA** cancels the lock. The status row shows the active modifier. A dash in the table means that the keypad has no printed secondary legend: this app retains the primary action on that layer. The two bracket-like tab keys are shown as ⇤ and ⇥, and the double page arrows as ⇈ and ⇊.

## Complete physical keypad

Rows below follow the physical keypad, left to right, top to bottom. □ denotes a template field, with exponents rendered raised in calculations; □/□ denotes the stacked fraction key. The mixed-fraction legend is a whole-number field beside a fraction.

| Primary key | SHIFT legend/action | ALPHA character |
| --- | --- | --- |
| ON | — | — |
| HOME | — | — |
| ⇤ (previous tab) | — | — |
| ↑ | — | — |
| ⇥ (next tab) | — | — |
| ⇈ | — | — |
| SETTINGS | — | — |
| ↩ | — | — |
| ← | — | — |
| OK | — | — |
| → | — | — |
| ⇊ | — | — |
| SHIFT | — | — |
| ALPHA | A-LOCK | — |
| VARIABLE | — | — |
| ↓ | — | — |
| CATALOG | — | — |
| TOOLS | — | — |
| x,θ,T,Y | → | A |
| □/□ | ■ □/□ | B |
| √□ | ⁿ√□ | C |
| □^□ | logₐ□ | D |
| □² | log | E |
| e^□ | ln | F |
| , | : | G |
| sin | sin⁻¹ | H |
| cos | cos⁻¹ | I |
| tan | tan⁻¹ | J |
| ( | = | K |
| ) | — | L |
| 7 | π | M |
| 8 | ∠ | N |
| 9 | i | O |
| ⌫ (DEL) | INS | — |
| AC | OFF | — |
| 4 | [ | P |
| 5 | ] | Q |
| 6 | — | R |
| × | — | S |
| ÷ | — | T |
| 1 | ʳ | U |
| 2 | θ | V |
| 3 | — | W |
| + | °′″ | X |
| − | (-) | Y |
| 0 | Select | Z |
| . | Paste | Space |
| ×10^□ | Capture | " |
| FORMAT | Ans | — |
| EXE | ≈ | — |

The key **x,θ,T,Y** is the context-dependent variable key. In this app’s calculation screen it enters **x**, which reads stored **X**, matching the manual’s Calculate convention. Graph Math+ has no separate dedicated x key. ALPHA + that key enters **A**. **sin**, **cos**, and **tan** are separate keys with their own inverse functions on SHIFT.

## Editing and natural templates

- **←/→** moves the cursor. **⌫ (DEL)** deletes the preceding character. **AC** or **↩** clears the current expression without clearing memories/history. ↩ cancels an active selection first.
- **↑/↓** moves between template fields; otherwise it recalls older/newer history. **⇈/⇊** also moves through history. Up to 20 results are retained.
- **□/□ → 1 → ↓ → 3 → → → EXE** produces a stacked 1/3. Pressing □/□ after a value captures it as the numerator. Fractions can nest. UP returns to the numerator; RIGHT from the completed denominator leaves the fraction.
- **SHIFT → □/□ → 2 → → → 1 → ↓ → 3 → → → EXE** enters mixed 2⅓. RIGHT moves from the whole-number field to the numerator, then to the denominator and out.
- **2 → □^□ → 3 → → → EXE** produces 8. Empty exponents show raised hollow boxes. RIGHT or DOWN leaves the exponent; UP immediately after it re-enters it. Fractional and nested powers are supported. □² appends a squared exponent.
- **√□ → 8 → EXE** produces 2√2. **SHIFT → √□ → 5 → → → 3 → 2 → EXE** enters the fifth root of 32 and produces 2. Enter the root index first, then RIGHT for the radicand.
- **SHIFT → □^□ → 2 → → → 8 → EXE** produces log₂8 = 3. Enter the base first, then RIGHT for the argument. **SHIFT → □²** inserts log; **SHIFT → e^□** inserts ln. The unshifted e^□ inserts an exponential template.
- **sin**, **cos**, and **tan** insert their own functions. **SHIFT → sin**, **SHIFT → cos**, and **SHIFT → tan** insert sin⁻¹, cos⁻¹, and tan⁻¹ respectively.
- **(** inserts a group with an automatic closing bracket. **)** moves over an existing closing bracket. Final missing parentheses are completed on evaluation. Brackets grow with tall fractions and roots.
- **×10^□** enters scientific notation with a raised exponent placeholder. **SHIFT → 7** inserts π. **SHIFT → FORMAT** inserts Ans.
- **−** is binary subtraction and continues Ans after a result. **SHIFT → −**, labelled **(-)**, enters a unary negative and starts a new negative value after a result.
- **SHIFT → 1** adds an explicit radian marker ʳ. Put it after the argument or grouped angle, for example sin((π÷2)ʳ). **SHIFT → +** enters the °′″ separator; repeated separators enter degrees, minutes and seconds.

Long expressions scroll to the active field; the expression limit is 111 internal characters. Empty template boxes must be filled before evaluation. Ordinary **÷** remains an inline division sign, separate from a stacked fraction.

## Numbered menu shortcuts

**CATALOG**, **SETTINGS**, **VARIABLE**, **FORMAT**, **TOOLS**, and the **Copy/Cut** selection menu show an option badge beside each row. Options run **1–9, then A–Z**, from top to bottom. Shortcuts activate the option immediately, without OK. Option labels continue across scrolling pages; they do not restart at 1 on each visible page.

For **A–L**, press the physical key carrying that red letter directly, without ALPHA. For **M–Z**, press **ALPHA** then that letter’s key. Examples: CATALOG → **1** inserts √□; CATALOG → **7** inserts !; CATALOG → **x,θ,T,Y** selects option A (sin); CATALOG → **ALPHA → 0** selects option Z (More). More opens the remaining catalogue entries with their own numbering from 1. This keeps every catalogue entry selectable using a native option shortcut.

**↑/↓** selects a row, wrapping at the ends. **←/→** or **⇈/⇊** moves five rows. **OK/EXE** chooses it. **↩/AC** returns to calculation. **HOME** saves and exits. The contrast slider and read-only key guide use arrows rather than numbered options.

## Menus and output

**SETTINGS**: 1 angle (Degrees → Radians → Gradians); 2 standard/decimal output; 3 Norm/Fix/Sci; 4 Fix/Sci precision (0–9); 5 Contrast; 6 VARIABLE; 7 key guide; 8 Clear history. A setting shortcut cycles it immediately; choose another shortcut or return with ↩. Clear history resets Ans and history but preserves variables.

**SETTINGS → 5** opens Contrast. **←** makes the LCD lighter and **→** darker, across 11 levels (0–10). The preview and all LCD shades update live. **OK/EXE**, **↩**, or **AC** returns to SETTINGS. Contrast is saved on HOME exit.

**FORMAT**: 1 Standard ↔ Decimal; 2 Mixed ↔ Improper; 3 °′″; 4 Engineering; 5 Engineering reverse. Choose an option to return to calculation. **SHIFT → EXE (≈)** evaluates just that result approximately; plain EXE restores the normal exact-result preference on the next calculation. FIX/SCI settings override exact forms. Supported exact results include reduced fractions, single surds and rational multiples of π. Engineering and °′″ are presentation options for the current session.

**TOOLS**: 1 Graph Math+ key guide; 2 Copy expression; 3 Paste; 4 Clear history. The key guide covers all 50 physical keys and their SHIFT/ALPHA legends. **←/→**, **⇤/⇥**, or **⇈/⇊** changes its ten pages. Tab keys have no calculation-screen action.

**CATALOG** contains cube root, reciprocal, absolute value, %, !, π, e, sin/cos/tan and each inverse, ln/log/base logarithm, exponentials, nth roots, combinations/permutations, gcd/lcm, remainder, min/max, rounding, hyperbolic/inverse hyperbolic functions, normal CDF and binomial probability, plus punctuation and Ans. **Factorial is CATALOG → 7**; it is not SHIFT + dot. Comma separates multi-argument function fields.

## All 28 variables

A–Z, r and θ retain numerical double precision. Use **value → variable → EXE** to store. The assignment arrow is **SHIFT → x,θ,T,Y**. Type A–Z with the physical red ALPHA legends, θ with **SHIFT → 2**, and r through **VARIABLE**. The VARIABLE list inserts the selected variable; its option labels identify rows, not the variable’s name. For example, option 1 is variable A, option A is variable J, option R is r, and option S is θ.

Example: **4 → 2 → SHIFT → x,θ,T,Y → ALPHA → x,θ,T,Y → EXE** stores 42 in A. Then **AC → ALPHA → x,θ,T,Y → + → 8 → EXE** gives 50. Stored X is available as **ALPHA → +** or the unshifted **x,θ,T,Y** key. Exact symbolic information is retained by Ans for supported results; ordinary variables hold numerical values.

## Selection, insertion and capture

**SHIFT → 0 (Select)** anchors a selection at the current cursor. Move with arrows to underline a range, then press **OK** for **1 Copy / 2 Cut**. **↩** cancels. **SHIFT → . (Paste)** pastes from this add-in’s own clipboard. TOOLS offers whole-expression copy/paste. Clipboard contents are local to the running app and are not exchanged with the OS clipboard or saved.

**SHIFT → ⌫ (INS)** arms insertion capture: position the cursor before an existing operand, arm INS, then press a function key to wrap that operand in the function. Example: enter 8, move LEFT, press SHIFT → ⌫ → √□ to obtain √8. This operates with the app’s natural-template editor.

**SHIFT → ×10^□ (Capture)** writes a 396×224 colour `lcdshot.bmp` in USB storage. It captures the calculation screen with the transient SHIFT indicator cleared. A later capture replaces that screenshot.

**ON** leaves input intact. **SHIFT → AC (OFF)** saves and uses gint’s power-off/resume API on the target hardware; this hardware action still needs a physical Graph Math+ check. In emulator runs it saves and exits. **HOME** is the usual save-and-exit key.

## Literal characters and scope

ALPHA + **.** enters a space; ALPHA + **×10^□** enters a double quote. The colon, equality sign, square brackets, ∠ and i are mapped to their printed SHIFT keys. These characters display instead of being replaced by unrelated functions. Quoted text, complex input and matrix brackets report explicit unsupported-input errors when evaluated. Equality and colon are not a solver or a programming statement separator in this app. There are no separate complex, matrix, graphing, vector, spreadsheet or equation applications.

## Saving and verification

HOME saves contrast, output preferences, the last 20 calculations and all 28 variables to `lcdmath.dat`. A valid previous save is retained as `lcdmath.bak`; `lcdmath.tmp` is used during replacement. Unevaluated input is not saved. The version-2 format migrates the earlier nine-variable format, preserving its A–F/X/Y/Z values, history and contrast.

The release is exercised in emuCasioCalc’s **fx-CG50 model** using the corresponding physical matrix positions translated through gint’s Graph Math+ mapping. This is not a Graph Math+ hardware emulator and uses no F1–F4 catalogue/settings aliases. Host tests check all 50 keys across primary/SHIFT/ALPHA layers; runtime tests cover A–Z storage, templates, output, menu shortcuts, clipboard, capture and save migration. Actual Graph Math+ display revision and installed MPM build are not established.
