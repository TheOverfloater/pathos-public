/*
==============================================================================

QC script generated by Half-Life MDL Decompiler 1.2 
2003, Kratisto. Based on code from Valve's HL SDK.

shellcasings.mdl

Original internal name:
"bullets.mdl"

==============================================================================
*/

$modelname "bullets.mdl"
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
studio "pellet_reference"
studio "railgunslug_reference"
}

// 1 hit box(es)
$hbox 0 "Bone01" -1.090000 -2.280000 -1.700000 1.090000 2.780000 3.970000

// 1 animation sequence(s)
$sequence "casing" "casing" fps 10 

// End of QC script.
