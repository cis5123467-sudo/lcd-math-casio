# Display research — 18 November 2026

Official Casio sources were consulted before designing the interface:

1. [fx-991CW product page](https://www.casio.com/us/scientific-calculators/product.FX-991CW/) describes the dot-matrix, four-gradation display and Natural Textbook Display. Its actual product photograph was inspected (`fx991cw-front.png`). The screen has a pale green/grey background with dark pixel glyphs, rather than seven-segment numerals.
2. The product page's Natural Textbook Display example (`fx991cw-natural.png`) was inspected. Expressions start at the upper left and answers appear towards the lower right. Fractions have horizontal bars; roots have overbars; multiplication next to roots is compact. Small mode and angle indicators occupy a separate top strip.
3. [Casio's Input/Output settings](https://support.casio.com/global/en/calc/manual/fx-991CW%2BUK_en/calculator_apps_and_menus/using_the_settings_menu.html) describes MathI/MathO, MathI/DecimalO and linear entry modes, and exact/decimal conversion. This informed the textbook rendering and answer toggle.
4. [Original fx-991CW Getting Started / Contrast / Indicators](https://support.casio.com/global/en/calc/manual/fx-570CW_991CW_en/before_using_the_calculator/getting_started.html) documents selecting System Settings > Contrast and adjusting it with left/right. The same page documents the natural-input, boxed D/R/G, FIX/SCI, shift and calculation-history indicators.

Design decisions: a dedicated LCD panel fills the Math+ screen; no fake keypad; dark bitmap glyphs on a green/grey background; natural fractions, radicals and powers; an upper-left expression and lower-right answer; a small status strip; no branding, shortcuts, tooltips, history counts or help text in the calculation view. Catalog, settings and help are separate views. The contrast panel uses a left/right scale, four shade samples and a formula preview, with eleven readable levels saved between launches.

The reference images are Casio's official images, retained as research references. They are not embedded in the add-in. The app's bitmap typography and menu icons are separate assets. This is an inspired add-in, not Casio firmware or an exact clone of its application suite.

## Physical fx-85GT CW reference

Chris supplied a photograph of his actual fx-85GT CW displaying `sqrt(8)` and `2*sqrt(2)`. This became the primary appearance reference for 1.0.1: a warmer olive-green LCD, taller dot-matrix numerals, an expression nearer the upper-left edge, an answer nearer the lower-right edge, a solid history triangle and a simpler recessed screen edge. Lighting, reflections and scratches in the photograph are not simulated. Saved contrast and the calculation engine are unchanged.

## Natural templates and symbol audit — v0.0.1

The [official fx-83GT CW / fx-85GT CW manual](https://support.casio.com/storage/en/manual/pdf/EN/004/fx-83GT%20CW_85GT%20CW_EN.pdf) was consulted for natural textbook template entry. The app now separates fraction templates from ordinary division, supplies hollow field placeholders and vertical field movement, and renders indexed roots, inverse trig superscripts, true exponent fields and base-log subscripts. The renderer's bracket geometry follows the enclosed expression height; root stems connect continuously to the overbar. Fraction numerator and denominator alignment uses measured content bounds. Layout tests check that rendered primitives remain within those bounds.

The audit also covered expression grouping: implicit multiplication after a fraction, and applying a square/power/factorial to an entire completed fraction. The app remains an approximation of the visual style; physical Graph Math+ verification remains separate from emulator testing.
