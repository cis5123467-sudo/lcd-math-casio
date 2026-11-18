# Dedicated fraction-key test — existing 1.0.1 release

Executed the unchanged `release/lcd-math-1.0.1.g3a` in emuCasioCalc's fx-CG50 model, using **FRAC (keycode 0x51)**, not DIV. This is the same logical fraction key handled by the Math+-aware gint build. Physical Math+ testing is separate.

| Input | Outcome |
| --- | --- |
| 1, FRAC, 3, EXE | Stacked input and exact stacked 1/3 answer; FORMAT displays decimal |
| 1, FRAC, 3, +, 1, FRAC, 6, EXE | Correct stacked 1/2 answer |
| (1+2), FRAC, 4, EXE | Correct stacked 3/4 answer |
| 1, FRAC, (2+3), EXE | Correct stacked 1/5 answer |
| 1, FRAC, 3, LEFT, LEFT, 2, EXE | Numerator edits to 12; answer 4 |
| FRAC, 1, DOWN, 3, EXE | Fails: fraction-first does not create an editable template; Syntax ERROR |

Input, result and decimal screenshots were inspected. Numeric results were asserted against the saved SuperH session. Logs and screenshots are in the named subdirectories; raw decoded results are in `results.json`. No add-in source or release binary was changed for this test.

The current fraction button is a division-insertion key whose expression is typeset as a fraction. It does not yet implement Casio-style numerator/denominator template entry or vertical navigation between fraction fields.
