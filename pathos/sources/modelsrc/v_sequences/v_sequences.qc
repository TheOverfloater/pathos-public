/*
==============================================================================
==============================================================================
*/

$modelname "v_sequences.mdl"
$cd ".\"
$cdtexture ".\"
$scale 1.0
$cliptotextures
$origin 0.000000 0.000000 -4.000000

$texrendermode "null.bmp" "masked"

//reference mesh(es)
$body "studio" "v_hands_reference"

$bodygroup "studio"
{
blank
blank
studio "v_hands_book1_reference"
studio "v_hands_book2_reference"
}

// 2 attachment(s)
$attachment 0 "V_Bip01 Head" 0.000000 0.000000 0.000000
$attachment 1 "V_Bip01 Head" 5.000000 0.000000 0.000000
$attachment 2 "V_Bip01 Head" 0.000000 5.000000 0.000000
$attachment 3 "V_Base" 0.000000 0.000000 0.000000

// 3 animation sequence(s)
$sequence "idle" "idle" fps 30 loop
$sequence "ladder_down_lup" "ladder/ladder_down_lup" fps 45 { event 9001 22 }
$sequence "ladder_down_rup" "ladder/ladder_down_rup" fps 45 { event 9001 22 }
$sequence "ladder_up_lup" "ladder/ladder_up_lup" fps 45 { event 9001 22 }
$sequence "ladder_up_rup" "ladder/ladder_up_rup" fps 45 { event 9001 22 }
$sequence "ladder_idle_lup" "ladder/ladder_idle_lup" fps 15 loop
$sequence "ladder_idle_rup" "ladder/ladder_idle_rup" fps 15 loop
$sequence "ladder_enter_bottom" "ladder/ladder_enter_bottom" fps 30  { event 9001 31 }  { event 9001 48 }
$sequence "ladder_enter_left" "ladder/ladder_enter_left" fps 30  { event 9001 21 }  { event 9001 51 }  { event 9001 61 }
$sequence "ladder_enter_right" "ladder/ladder_enter_right" fps 30  { event 9001 32 }  { event 9001 55 }  { event 9001 65 }
$sequence "ladder_enter_top" "ladder/ladder_enter_top" fps 30  { event 9001 15 }  { event 9001 33 }  { event 9001 43 }  { event 9001 70 }  { event 9001 110 }  { event 9001 132 }
$sequence "ladder_leave_bottom_lup" "ladder/ladder_leave_bottom_lup" fps 30 { event 9001 15 } { event 5004 21 "player/pl_step2.wav" } { event 5004 28 "player/pl_step4.wav" }
$sequence "ladder_leave_bottom_rup" "ladder/ladder_leave_bottom_rup" fps 30 { event 9001 22 }
$sequence "ladder_leave_left_lup" "ladder/ladder_leave_left_lup" fps 30 { event 9001 18 } { event 5004 35 "player/pl_step2.wav" } { event 5004 46 "player/pl_step4.wav" }
$sequence "ladder_leave_left_rup" "ladder/ladder_leave_left_rup" fps 30 { event 9001 18 } { event 5004 35 "player/pl_step3.wav" } { event 5004 46 "player/pl_step1.wav" }
$sequence "ladder_leave_right_lup" "ladder/ladder_leave_right_lup" fps 30 { event 9001 16 } { event 5004 26 "player/pl_step1.wav" } { event 5004 39 "player/pl_step2.wav" }
$sequence "ladder_leave_right_rup" "ladder/ladder_leave_right_rup" fps 30 { event 9001 16 } { event 5004 26 "player/pl_step3.wav" } { event 5004 39 "player/pl_step1.wav" }
$sequence "ladder_leave_top_lup" "ladder/ladder_leave_top_lup" fps 30 { event 9001 22 } { event 9001 32 } { event 9001 24 } { event 5004 57 "player/pl_step1.wav" } { event 5004 66 "player/pl_step3.wav" }
$sequence "ladder_leave_top_rup" "ladder/ladder_leave_top_rup" fps 30 { event 9001 22 } { event 9001 32 } { event 9001 24 } { event 5004 57 "player/pl_step4.wav" } { event 5004 66 "player/pl_step2.wav" }
$sequence "book_enter" "book_enter" fps 15
$sequence "book_loop" "book_loop" fps 15 loop
$sequence "book_exit" "book_exit" fps 15
$sequence "sleep_enter" "sleep_enter" fps 30 { event 5004 64 "misc/get_in_bed.wav" } { event 5004 167 "misc/pull_sheets.wav" }
$sequence "sleep_loop" "sleep_loop" fps 30 loop
$sequence "die_standing_forward1" "death_anims/die_standing_forward1" fps 48 { event 5005 28 "player/death_bodydrop1.wav" }
$sequence "die_standing_forward2" "death_anims/die_standing_forward2" fps 48 { event 5005 23 "player/death_bodydrop2.wav" }
$sequence "die_standing_backwards1" "death_anims/die_standing_backwards1" fps 48 { event 5005 32 "player/death_bodydrop1.wav" }
$sequence "die_standing_backwards2" "death_anims/die_standing_backwards2" fps 48 { event 5005 29 "player/death_bodydrop2.wav" }
$sequence "die_ducking_forward1" "death_anims/die_ducking_forward1" fps 38 { event 5005 21 "player/death_bodydrop1.wav" }
$sequence "die_ducking_forward2" "death_anims/die_ducking_forward2" fps 36 { event 5005 25 "player/death_bodydrop2.wav" }
$sequence "die_ducking_forward3" "death_anims/die_ducking_forward3" fps 39 { event 5005 19 "player/death_bodydrop1.wav" }
$sequence "die_ducking_backwards1" "death_anims/die_ducking_backwards1" fps 33 { event 5005 14 "player/death_bodydrop1.wav" }
$sequence "die_ducking_backwards2" "death_anims/die_ducking_backwards2" fps 35 { event 5005 16 "player/death_bodydrop2.wav" }
$sequence "die_gassing" "death_anims/die_gassing" fps 35 { event 5005 35 "player/death_bodydrop2.wav" }
