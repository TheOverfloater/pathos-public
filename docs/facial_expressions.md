# Facial Animation System

The Pathos Engine sports a facial animation system heavily inspired by
how Source did facial animations, but the system in Pathos is far simpler
in design and is more basic. Facial animations are created with the model
viewer. See "model_viewer.txt" for more information.

Facial flex scripts exist in two forms: Generic ones used by NPCs for
normal AI states like being scared, idle, in combat, or dead. These are
defined in the "expressions_human.txt" file under pathos/scripts/expressions.

Here is an example of a generic AI-state definition:
IDLE_EXPRESSIONS npc_human_idle1 npc_human_idle2 npc_human_idle3 

Here, idle state expressions have the scripts npc_human_idle1,
npc_human_idle2 and npc_human_idle3 tied to them. These will be picked
randomly.

However for facial expression scripts that are used for scripted sentences,
those are defined in the following manner:
SCRIPT_EXPRESSION HARRISON_SENT1 harrison/harrison_sent1

Here, the sentence entry from sentences.txt, HARRISON_SENT1 will be tied
to the facial flex script file harrison_sent1.ccs located under the
"harrison" folder.

The way facial animations tie into the game is very simple an straightforward,
as it was used less ingame than I expected it to be. Hopefully this will help
you make use of it - if you want.

