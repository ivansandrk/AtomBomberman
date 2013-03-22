; empty lines are ignored
; also comment lines are ignored (like this one)

; actual array data (# is solid, : is brick, . is empty)
; basic with empty fields where level specials are
::::::::::::::: ; row 10
:#:#:#:#:#:#:#:
.::::::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
.:::.::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
....::::::::::: ; row 0

; number of additional stuff on level (arrow, carry, trampoline, warp)
; make sure this number matches the actual amount of stuff
15
A N 0 0 ; arrow, heading (N, S, W, E), col, row
A S 1 0
A W 2 0
A E 3 0
C S 9 7 ; carry, heading, col, row
C S 9 6
C E 9 5
C E 10 5
C N 11 5
C N 11 6
C W 11 7
C W 10 7
T 4 4 ; trampoline, col, row
; first warp gate is 0, second 1, ...
W 1 0 4 ; warp gate, target gate, col, row
W 0 0 8

; player starting locations
10 ; maximum number of players on this map/level
; (col row team - team not used)
 0  0  0
14 10  1
 0 10  0
14  0  1
 6  4  0
 8  0  1
12  4  0
 2  6  1
10  8  0
 6 10  1


; original Atomic Bomberman .sch file values
; powerup information; the fields are:
;   bornwith - how many has at start (doesnt include the default 1 bomb & 2 flames)
;   has_override - if should read the next value
;   override_value - this many hidden in level
;   forbidden - zero
;   (note the last text field has no effect; it is only a comment)

; JDV Bomberman fields
; starting level_additional
; negative number - how many times doing 1-in-10 chance of dropping that powerup
 2 10  bombs
 2 10  flame
 0  3  disease
 0  4  kicker
 0  8  skates
 0  2  punch
 0  2  grab
 0  1  spooger
 0 -2  goldflame
 0 -4  trigger
 0  1  jelly
 0 -4  superdisease
 0 -2  random



;; stuff below ignored --

; normal basic
::::::::::::::: ; row 10
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
::::::::::::::: ; row 0

; basic with empty fields where level specials are
::::::::::::::: ; row 10
:#:#:#:#:#:#:#:
.::::::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
.:::.::::::::::
:#:#:#:#:#:#:#:
:::::::::::::::
:#:#:#:#:#:#:#:
....::::::::::: ; row 0

; custom pimped version
::::::::::::::: ; row 10
..............:
.....::.......:
......#.......:
.....#........:
:.............:
........#...:::
.#:#:#..:#:#:.:
..............:
:#:#:....#:#:#:
..............: ; row 0

; game defaults
 1 10  bombs
 2 10  flame
 0  3  disease
 0  4  kicker
 0  8  skates
 0  2  punch
 0  2  grab
 0  1  spooger
 0 -2  goldflame
 0 -4  trigger
 0  1  jelly
 0 -4  superdisease
 0 -2  random
