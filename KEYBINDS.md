# LCD Math controls and usage

This guide describes the v0.0.1 build. Press and release SHIFT or ALPHA, then press the next key; you do not need to hold them. Pressing the same modifier twice cancels it. Modifiers reset after the next non-modifier key.

## Getting started

Copy the G3A to the root of the calculator's USB storage, eject, and launch LCD Math from TOOLS in the existing MPM installation. Type an expression and press OK or EXE. A number starts a new expression after a result; an operator continues using Ans. Final missing closing parentheses are completed at evaluation. HOME saves and exits from any screen.

## Main calculation screen

| Key | Action |
| --- | --- |
| 0–9, dot | Enter digits and decimal point |
| +, −, ×, ÷ | Arithmetic |
| Fraction key | Insert a numerator/denominator template; capture a preceding value as numerator |
| ( and ) | Group expressions |
| Comma | Separate function arguments |
| Negative-sign key (−) | Insert a minus sign |
| Square key | Insert `^2` |
| Power key | Insert a superscript placeholder; enter the exponent and press RIGHT or DOWN to leave |
| Square-root key | Insert a radical with an empty radicand placeholder |
| SIN, COS, TAN | Insert the corresponding trig function |
| LN / LOG, where present | Natural / base-10 logarithm |
| Exponential-function key, where present | Insert `exp(` |
| EXP / ×10^x key | Insert `*10^(` for scientific notation |
| ANS/store key | Insert `Ans`, the most recent answer |
| x / X, where present | Recall memory X; it must have a stored value |
| OK / EXE | Evaluate |
| Left / right | Move cursor; also re-enter editing after a result or error |
| SHIFT then left / right | Jump to start / end |
| DEL | Delete the character immediately before the cursor |
| BACK or AC | Clear the current expression |
| SHIFT then DEL | Clear the current expression |
| Up / down | Move between fraction fields; otherwise recall older / newer expressions |
| FORMAT | Toggle exact / decimal output |
| SHIFT then FORMAT | Cycle degrees → radians → gradians |
| CATALOG | Open function catalog |
| SETTINGS or TOOLS | Open settings |
| VARS | Open memories |
| SHIFT then ANS/store | Open memories |
| HOME | Save and exit from any screen |

After recalling a history expression, press EXE to evaluate it again. Ans always refers to the latest evaluated answer, not the history position. Clearing an expression leaves history, Ans and memories intact.

## Natural fraction and power entry

- `fraction`, `1`, `DOWN`, `3`, `RIGHT`, `EXE` gives 1/3.
- `1`, `fraction`, `3`, `RIGHT`, `EXE` also gives 1/3, capturing the existing numerator.
- While inside a fraction, DOWN selects the denominator and UP selects the numerator. RIGHT at the end of the numerator enters the denominator; RIGHT at the end of the denominator exits the fraction.
- Press fraction inside a field to nest another fraction.
- `2`, `power`, `3`, `RIGHT`, `EXE` gives 8. The empty exponent is a raised hollow box. DOWN exits a filled exponent; UP immediately after a completed power re-enters it.
- For a fractional exponent, insert a fraction template inside the exponent. A power or factorial entered after leaving a fraction applies to that whole fraction.
- Function and grouping brackets close automatically. A closing-parenthesis key steps over an existing closing parenthesis. LEFT from the beginning of a function argument moves before that function; RIGHT moves back into it.
- Ordinary ÷ stays a division sign, separate from the stacked-fraction key.

## Scientific shortcuts

| Sequence | Action |
| --- | --- |
| SHIFT then SIN / COS / TAN | Inverse sine / cosine / tangent |
| SHIFT then square | Square root |
| SHIFT then square root | Cube root |
| SHIFT then power | Reciprocal, `^(-1)` |
| SHIFT then LN | Exponential, `exp(` |
| SHIFT then LOG | Power of ten, `10^(` |
| SHIFT then exponential-function key | Natural log |
| SHIFT then dot | Factorial, `!` |
| SHIFT then division or fraction | Percentage, `%` |
| SHIFT then EXP, or SHIFT then 7 | Pi |
| SHIFT then negative-sign key, or SHIFT then 8 | Euler's number e |
| SHIFT then right parenthesis | Comma |
| ALPHA then 1–9 | Recall A, B, C, D, E, F, X, Y, Z respectively |

These are this add-in's bindings. For functions without a dedicated physical key, use CATALOG.

## Menus

CATALOG, settings and memory share these controls:

| Key | Action |
| --- | --- |
| Up / down | Previous / next item; wraps at the ends |
| Left / right | Jump backward / forward five items |
| OK / EXE | Select |
| BACK or AC | Return to the calculation screen |
| HOME | Save and exit the add-in |

In settings, OK/EXE cycles a selected option forward; SHIFT then OK/EXE cycles angle, number format or precision backward. Exact/decimal simply toggles. Contrast, memories and help open their own views. Clear history immediately clears history and resets Ans to zero; stored variables remain.

## Contrast and output

SETTINGS or TOOLS → Contrast → OK/EXE opens the contrast panel. Left makes the LCD lighter, right makes it darker. Levels range from 0 to 10 with a live preview. OK/EXE, BACK or AC returns to settings. HOME saves and exits.

Angle selects degrees, radians or gradians. Number format selects Norm, Fix or Sci. Norm uses supported exact forms or up to ten significant decimal digits. Fix and Sci use 0–9 decimal places and override exact output. To see fractions or surds again, select Norm and Exact.

## Memory

Calculate the value first. Open VARS, select a variable, then press SHIFT followed by OK/EXE to store Ans. Press OK/EXE without SHIFT to insert that variable into the expression and return to calculating. Alternatively, ALPHA then 1–9 recalls the matching variable. An empty memory reports an error.

Example: evaluate `42`, open VARS, select A, and press SHIFT then EXE. Return with BACK, then press ALPHA, 1, +, 8, EXE to get 50.

## Catalog and examples

Select a function with the arrows and press EXE to insert it. Most entries insert an opening parenthesis. Separate arguments with comma; the shifted right-parenthesis key also inserts a comma.

| Calculation | Entry | Result |
| --- | --- | --- |
| Basic arithmetic | `2*(3+4)` | 14 |
| Fractions | `1/3+1/6` | 1/2 |
| Surds | `sqrt(8)` | 2√2 |
| Trigonometry in degrees | `sin(30)` | 1/2 |
| Factorial | `5!` | 120 |
| Percentage | `200*15%` | 30 |
| Arbitrary-base log | `logb(8,2)` | 3 |
| nth root | `root(-32,5)` | −2 |
| Combinations | `ncr(10,3)` | 120 |
| Scientific notation | `6.02*10^(23)` | 6.02 × 10²³ |

The catalog also provides absolute value, cube root, reciprocal, exponentials, permutations, gcd/lcm, remainder, min/max, floor/ceil/round, hyperbolic and inverse hyperbolic functions, normal CDF and binomial probability. Expressions can contain up to 111 characters. Long expressions scroll horizontally. Tall formulas retain their mathematical layout and scroll to the active field. Empty template fields must be filled before evaluating.

## Help and saving

Open SETTINGS/TOOLS → Help / key guide. Left/right changes help pages; BACK or AC returns to calculating.

Exit with HOME to save contrast, settings, the latest 20 calculations and stored variables in `lcdmath.dat`. A previous valid save is retained as `lcdmath.bak`. Unevaluated input is not saved.

The app evaluates real-valued scientific expressions; it does not include separate graphing, complex, matrix, vector or spreadsheet applications.

## Emulator aliases

F1 = Catalog, F2 = Settings, F3 = Memories, F4 = Help. These are compatibility aliases for fx-CG50 testing; use the named keys on Graph Math+.
