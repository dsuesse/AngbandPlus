;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: org.langband.vanilla -*-

#|

DESC: variants/vanilla/config/weapons.lisp - weapons for vanilla variant
Copyright (c) 2000-2002 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :org.langband.vanilla)

(define-object-kind "broken-dagger" "& broken dagger~"
  :numeric-id 30
  :x-attr #\D
  :x-char #\|
  :depth 0
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(0 0 0 0)
  :weight 5
  :cost 1
  :obj-type '(<broken-dagger> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3201
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 1 :num-dice 1 :tohit-modifier -2 :dmg-modifier -4)) 

(define-object-kind "bastard-sword" "& bastard sword~"
  :numeric-id 31
  :x-attr #\W
  :x-char #\|
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 140
  :cost 350
  :obj-type '(<bastard> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3221
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 3)) 

(define-object-kind "scimitar" "& scimitar~"
  :numeric-id 32
  :x-attr #\W
  :x-char #\|
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 130
  :cost 250
  :obj-type '(<scimitar> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3218
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 2)) 

(define-object-kind "tulwar" "& tulwar~"
  :numeric-id 33
  :x-attr #\W
  :x-char #\|
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 100
  :cost 200
  :obj-type '(<tulwar> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3215
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 2)) 

(define-object-kind "broad-sword" "& broad sword~"
  :numeric-id 34
  :x-attr #\W
  :x-char #\|
  :depth 10
  :rarity 0
  :chance #(1 1 0 0)
  :locale #(10 15 0 0)
  :weight 150
  :cost 255
  :obj-type '(<broad-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3216
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 2)) 

(define-object-kind "short-sword" "& short sword~"
  :numeric-id 35
  :x-attr #\W
  :x-char #\|
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 80
  :cost 90
  :obj-type '(<short-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3210
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 7 :num-dice 1)) 

(define-object-kind "chaos-blade" "& blade~ of chaos"
  :numeric-id 36
  :x-attr #\v
  :x-char #\|
  :depth 70
  :rarity 0
  :chance #(8 0 0 0)
  :locale #(70 0 0 0)
  :weight 180
  :cost 4000
  :obj-type '(<blade-of-chaos> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3230
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 6 :resists '(<chaos> <confusion>))) 

(define-object-kind "two-h-sword" "& two-handed sword~"
  :numeric-id 37
  :x-attr #\W
  :x-char #\|
  :depth 30
  :rarity 0
  :chance #(1 1 0 0)
  :locale #(30 40 0 0)
  :weight 200
  :cost 775
  :obj-type '(<two-handed-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3225
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 3)) 

(define-object-kind "dirk" "& dirk~"
  :numeric-id 38
  :x-attr #\W
  :x-char #\|
  :depth 3
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(3 0 0 0)
  :weight 30
  :cost 25
  :obj-type '(<main-gauche> <dirk> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3205
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 1)) 

(define-object-kind "cutlass" "& cutlass~"
  :numeric-id 39
  :x-attr #\W
  :x-char #\|
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 110
  :cost 85
  :obj-type '(<cutlass> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3212
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 7 :num-dice 1)) 

(define-object-kind "exec-sword" "& executioner's sword~"
  :numeric-id 40
  :x-attr #\r
  :x-char #\|
  :depth 40
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(40 0 0 0)
  :weight 260
  :cost 850
  :obj-type '(<exec-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3228
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 4)) 

(define-object-kind "katana" "& katana~"
  :numeric-id 41
  :x-attr #\W
  :x-char #\|
  :depth 20
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(20 0 0 0)
  :weight 120
  :cost 400
  :obj-type '(<katana> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3220
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 3)) 

(define-object-kind "long-sword" "& long sword~"
  :numeric-id 42
  :x-attr #\W
  :x-char #\|
  :depth 10
  :rarity 0
  :chance #(1 1 0 0)
  :locale #(10 20 0 0)
  :weight 130
  :cost 300
  :obj-type '(<long-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3217
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 2)) 

(define-object-kind "dagger" "& dagger~"
  :numeric-id 43
  :x-attr #\W
  :x-char #\|
  :depth 0
  :rarity 0
  :chance #(1 1 1 1)
  :locale #(0 5 10 20)
  :weight 12
  :cost 10
  :obj-type '(<dagger> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3204
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 1)) 

(define-object-kind "rapier" "& rapier~"
  :numeric-id 44
  :x-attr #\W
  :x-char #\|
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 40
  :cost 42
  :obj-type '(<rapier> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3207
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 1)) 

(define-object-kind "sabre" "& sabre~"
  :numeric-id 45
  :x-attr #\W
  :x-char #\|
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 50
  :cost 50
  :obj-type '(<sabre> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3211
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 7 :num-dice 1)) 

(define-object-kind "small-sword" "& small sword~"
  :numeric-id 46
  :x-attr #\W
  :x-char #\|
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 75
  :cost 48
  :obj-type '(<small-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3208
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 1)) 

(define-object-kind "broken-sword" "& broken sword~"
  :numeric-id 47
  :x-attr #\D
  :x-char #\|
  :depth 0
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(0 0 0 0)
  :weight 30
  :cost 2
  :obj-type '(<broken-sword> <weapon> <sword>)
  :flags '(<show-modififers>)
  :sort-value 3202
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 2 :num-dice 1 :tohit-modifier -2 :dmg-modifier -4)) 

(define-object-kind "ball-and-chain" "& ball-and-chain~"
  :numeric-id 48
  :x-attr #\D
  :x-char #\\
  :depth 20
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(20 0 0 0)
  :weight 150
  :cost 200
  :obj-type '(<ball-and-chain> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3006
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 2)) 

(define-object-kind "whip" "& whip~"
  :numeric-id 49
  :x-attr #\D
  :x-char #\\
  :depth 3
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(3 0 0 0)
  :weight 30
  :cost 30
  :obj-type '(<whip> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3002
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 1)) 

(define-object-kind "flail" "& flail~"
  :numeric-id 50
  :x-attr #\D
  :x-char #\\
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 150
  :cost 353
  :obj-type '(<flail> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3013
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 2)) 

(define-object-kind "two-h-flail" "& two-handed flail~"
  :numeric-id 51
  :x-attr #\y
  :x-char #\\
  :depth 45
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(45 0 0 0)
  :weight 280
  :cost 590
  :obj-type '(<two-handed-flail> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3018
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 3)) 

(define-object-kind "morning-star" "& morning star~"
  :numeric-id 52
  :x-attr #\D
  :x-char #\\
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 150
  :cost 396
  :obj-type '(<morning-star> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3012
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 2)) 

(define-object-kind "mace" "& mace~"
  :numeric-id 53
  :x-attr #\D
  :x-char #\\
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 120
  :cost 130
  :obj-type '(<mace> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3005
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 2)) 

(define-object-kind "quarterstaff" "& quarterstaff~"
  :numeric-id 54
  :x-attr #\U
  :x-char #\\
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 150
  :cost 200
  :obj-type '(<quarterstaff> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3003
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 9 :num-dice 1)) 

(define-object-kind "war-hammer" "& war hammer~"
  :numeric-id 55
  :x-attr #\D
  :x-char #\\
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 120
  :cost 225
  :obj-type '(<war-hammer> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3008
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 3 :num-dice 3)) 

(define-object-kind "lead-mace" "& lead-filled mace~"
  :numeric-id 56
  :x-attr #\D
  :x-char #\\
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 180
  :cost 502
  :obj-type '(<lead-filled-mace> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3015
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 3)) 

(define-object-kind "disruption-mace" "& mace~ of disruption"
  :numeric-id 57
  :x-attr #\v
  :x-char #\\
  :depth 80
  :rarity 0
  :chance #(8 0 0 0)
  :locale #(80 0 0 0)
  :weight 400
  :cost 4300
  :obj-type '(<mace-of-disruption> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3020
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 8 :num-dice 5 :slays '(<undead>))) 

(define-object-kind "lucerne-hammer" "& lucerne hammer~"
  :numeric-id 58
  :x-attr #\B
  :x-char #\\
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 120
  :cost 376
  :obj-type '(<lucern-hammer> <weapon> <hafted>)
  :flags '(<show-modififers>)
  :sort-value 3010
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 2)) 

(define-object-kind "beaked-axe" "& beaked axe~"
  :numeric-id 59
  :x-attr #\s
  :x-char #\/
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 180
  :cost 408
  :obj-type '(<beaked-axe> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3110
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 2)) 

(define-object-kind "glaive" "& glaive~"
  :numeric-id 60
  :x-attr #\s
  :x-char #\/
  :depth 20
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(20 0 0 0)
  :weight 190
  :cost 363
  :obj-type '(<glaive> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3113
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 2)) 

(define-object-kind "halberd" "& halberd~"
  :numeric-id 61
  :x-attr #\s
  :x-char #\/
  :depth 25
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(25 0 0 0)
  :weight 190
  :cost 430
  :obj-type '(<halberd> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3115
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 3)) 

(define-object-kind "awl-pike" "& awl-pike~"
  :numeric-id 62
  :x-attr #\s
  :x-char #\/
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 160
  :cost 340
  :obj-type '(<awl-pike> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3104
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 8 :num-dice 1)) 

(define-object-kind "pike" "& pike~"
  :numeric-id 63
  :x-attr #\s
  :x-char #\/
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 160
  :cost 358
  :obj-type '(<pike> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3108
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 5 :num-dice 2)) 

(define-object-kind "spear" "& spear~"
  :numeric-id 64
  :x-attr #\s
  :x-char #\/
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 50
  :cost 36
  :obj-type '(<spear> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3102
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 1)) 

(define-object-kind "trident" "& trident~"
  :numeric-id 65
  :x-attr #\y
  :x-char #\/
  :depth 5
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(5 0 0 0)
  :weight 70
  :cost 120
  :obj-type '(<trident> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3105
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 8 :num-dice 1)) 

(define-object-kind "lance" "& lance~"
  :numeric-id 66
  :x-attr #\s
  :x-char #\/
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 300
  :cost 230
  :obj-type '(<lance> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3120
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 8 :num-dice 2)) 

(define-object-kind "great-axe" "& great axe~"
  :numeric-id 67
  :x-attr #\s
  :x-char #\/
  :depth 40
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(40 0 0 0)
  :weight 230
  :cost 500
  :obj-type '(<great-axe> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3125
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 4)) 

(define-object-kind "battle-axe" "& battle axe~"
  :numeric-id 68
  :x-attr #\s
  :x-char #\/
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 170
  :cost 334
  :obj-type '(<battle-axe> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3122
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 8 :num-dice 2)) 

(define-object-kind "lochaber-axe" "& lochaber axe~"
  :numeric-id 69
  :x-attr #\D
  :x-char #\/
  :depth 45
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(45 0 0 0)
  :weight 250
  :cost 750
  :obj-type '(<lochaber-axe> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3128
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 8 :num-dice 3)) 

(define-object-kind "broad-axe" "& broad axe~"
  :numeric-id 70
  :x-attr #\s
  :x-char #\/
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 160
  :cost 304
  :obj-type '(<broad-axe> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3111
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 6 :num-dice 2)) 

(define-object-kind "scythe" "& scythe~"
  :numeric-id 71
  :x-attr #\s
  :x-char #\/
  :depth 45
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(45 0 0 0)
  :weight 250
  :cost 800
  :obj-type '(<scythe> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3117
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 3 :num-dice 5)) 

(define-object-kind "scythe-slicing" "& scythe~ of slicing"
  :numeric-id 72
  :x-attr #\r
  :x-char #\/
  :depth 60
  :rarity 0
  :chance #(4 0 0 0)
  :locale #(60 0 0 0)
  :weight 250
  :cost 3500
  :obj-type '(<scythe-slicing> <weapon> <polearm>)
  :flags '(<show-modififers>)
  :sort-value 3130
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 4 :num-dice 8)) 

(define-object-kind "short-bow" "& short bow~"
  :numeric-id 73
  :x-attr #\U
  :x-char #\}
  :depth 3
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(3 0 0 0)
  :weight 30
  :cost 50
  :multiplier 2
  :obj-type '(<short> <bow>)
  :flags '(<show-modififers>)
  :sort-value 2812
  :the-kind '<bow>
  :game-values (make-game-values)) 

(define-object-kind "long-bow" "& long bow~"
  :numeric-id 74
  :x-attr #\U
  :x-char #\}
  :depth 10
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(10 0 0 0)
  :weight 40
  :cost 120
  :multiplier 2
  :obj-type '(<long> <bow>)
  :flags '(<show-modififers>)
  :sort-value 2813
  :the-kind '<bow>
  :game-values (make-game-values)) 

(define-object-kind "light-xbow" "& light crossbow~"
  :numeric-id 75
  :x-attr #\s
  :x-char #\}
  :depth 15
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(15 0 0 0)
  :weight 110
  :cost 140
  :multiplier 3
  :obj-type '(<bow> <light> <xbow>)
  :flags '(<show-modififers>)
  :sort-value 2823
  :the-kind '<bow>
  :game-values (make-game-values)) 

(define-object-kind "heavy-xbow" "& heavy crossbow~"
  :numeric-id 76
  :x-attr #\s
  :x-char #\}
  :depth 30
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(30 0 0 0)
  :weight 200
  :cost 300
  :multiplier 4
  :obj-type '(<bow> <heavy> <xbow>)
  :flags '(<show-modififers>)
  :sort-value 2824
  :the-kind '<bow>
  :game-values (make-game-values)) 

(define-object-kind "sling" "& sling~"
  :numeric-id 77
  :x-attr #\u
  :x-char #\}
  :depth 1
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(1 0 0 0)
  :weight 5
  :cost 5
  :multiplier 2
  :obj-type '(<sling> <bow>)
  :flags '(<show-modififers>)
  :sort-value 2802
  :the-kind '<bow>
  :game-values (make-game-values)) 

(define-object-kind "arrow" "& arrow~"
  :numeric-id 78
  :x-attr #\U
  :x-char #\{
  :depth 3
  :rarity 0
  :chance #(1 1 0 0)
  :locale #(3 15 0 0)
  :weight 2
  :cost 1
  :obj-type '(<normal> <ammo> <arrow>)
  :flags '(<show-modififers>)
  :sort-value 2601
  :the-kind '<ammo>
  :game-values (make-game-values :base-dice 4 :num-dice 1)) 

(define-object-kind "seeker-arrow" "& seeker arrow~"
  :numeric-id 79
  :x-attr #\G
  :x-char #\{
  :depth 55
  :rarity 0
  :chance #(2 0 0 0)
  :locale #(55 0 0 0)
  :weight 2
  :cost 20
  :obj-type '(<heavy> <ammo> <arrow>)
  :flags '(<show-modififers>)
  :sort-value 2602
  :the-kind '<ammo>
  :game-values (make-game-values :base-dice 4 :num-dice 4)) 

(define-object-kind "bolt" "& bolt~"
  :numeric-id 80
  :x-attr #\s
  :x-char #\{
  :depth 3
  :rarity 0
  :chance #(1 1 0 0)
  :locale #(3 25 0 0)
  :weight 3
  :cost 2
  :obj-type '(<normal> <ammo> <bolt>)
  :flags '(<show-modififers>)
  :sort-value 2701
  :the-kind '<ammo>
  :game-values (make-game-values :base-dice 5 :num-dice 1)) 

(define-object-kind "seeker-bolt" "& seeker bolt~"
  :numeric-id 81
  :x-attr #\B
  :x-char #\{
  :depth 65
  :rarity 0
  :chance #(4 0 0 0)
  :locale #(65 0 0 0)
  :weight 3
  :cost 25
  :obj-type '(<heavy> <ammo> <bolt>)
  :flags '(<show-modififers>)
  :sort-value 2702
  :the-kind '<ammo>
  :game-values (make-game-values :base-dice 5 :num-dice 4)) 

(define-object-kind "round-pebble" "& rounded pebble~"
  :numeric-id 82
  :x-attr #\s
  :x-char #\{
  :depth 0
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(0 0 0 0)
  :weight 4
  :cost 1
  :obj-type '(<light> <ammo> <shot>)
  :flags '(<show-modififers>)
  :sort-value 2500
  :the-kind '<ammo>
  :game-values (make-game-values :base-dice 2 :num-dice 1)) 

(define-object-kind "iron-shot" "& iron shot~"
  :numeric-id 83
  :x-attr #\s
  :x-char #\{
  :depth 3
  :rarity 0
  :chance #(1 0 0 0)
  :locale #(3 0 0 0)
  :weight 5
  :cost 2
  :obj-type '(<normal> <ammo> <shot>)
  :flags '(<show-modififers>)
  :sort-value 2501
  :the-kind '<ammo>
  :game-values (make-game-values :base-dice 3 :num-dice 1)) 

(define-object-kind "mighty-hammer" "& mighty hammer~"
  :numeric-id 498
  :x-attr #\D
  :x-char #\\
  :depth 15
  :rarity 0
  :chance #(0 0 0 0)
  :locale #(0 0 0 0)
  :weight 200
  :cost 1000
  :obj-type '(<grond> <weapon> <hafted>)
  :flags '(<instant-artifact> <show-modififers>)
  :sort-value 3050
  :the-kind '<weapon>
  :game-values (make-game-values :base-dice 9 :num-dice 3)) 
