/*
==============================================================================
==============================================================================
*/

$modelname "blackout.mdl"
$cd ".\"
$cdtexture ".\"
$scale 1.0
$cliptotextures
$origin 0.000000 0.000000 -4.000000

//reference mesh(es)
$body "studio" "blackout_ref"

// 2 attachment(s)
$attachment 0 "Blackout.view" 0.000000 0.000000 0.000000
$attachment 1 "Blackout.view" 5.000000 0.000000 0.000000
$attachment 2 "Blackout.view" 0.000000 5.000000 0.000000

// 3 animation sequence(s)
$sequence "idle" "idle" fps 30
$sequence "enter" "enter" fps 30
$sequence "exit" "exit" fps 30
$sequence "turn" "turn" fps 15
$sequence "turnw" "turnw" fps 10
$sequence "knockw" "knockw" fps 15
$sequence "knockout" "knockout" fps 30
