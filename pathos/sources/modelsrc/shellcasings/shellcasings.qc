/*
==============================================================================

QC script generated by Half-Life MDL Decompiler 1.2 
2003, Kratisto. Based on code from Valve's HL SDK.

shellcasings.mdl

Original internal name:
"shellcasings.mdl"

==============================================================================
*/

$modelname "shellcasings.mdl"
$cd ".\"
$cdtexture ".\"
$scale 1.0
$cliptotextures


$bbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$cbox 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000
$eyeposition 0.000000 0.000000 0.000000


//reference mesh(es)
$bodygroup "belt"
{
studio "22lr_reference"
studio "9mm_reference"
studio "40sw_reference"
studio "50ae_reference"
studio "556_reference"
studio "762_reference"
studio "buckshot_reference"
studio "link_reference"
studio "spoon_reference"
}

$texrendermode "pistolca.bmp" "masked" 
$texrendermode "762x54mm.bmp" "masked" 
$texrendermode "50bmg.bmp" "masked" 
$texrendermode "50ae.bmp" "masked" 
$texrendermode "shell_remington.bmp" "masked" 
$texrendermode "link.bmp" "masked" 

// 1 hit box(es)
$hbox 0 "Bone01" -1.090000 -2.280000 -1.700000 1.090000 2.780000 3.970000

// 1 animation sequence(s)
$sequence "idle" "idle" fps 10 

// End of QC script.
