# Debugging the AI node graph

Pathos uses a node graph system very similar to Half-Life 1's, only
with major improvements. One of these improvements is that the code
allows you to view and debug the node graph visually. To do this, you
can enable node graph debugging with the "r_nodes_debug" in the console.
This will make the game render info_node debug lines, which can come
from impulse commands issued by the player, or by NPCs marking their
paths and/or navigation failures.

Values:
 - 0 - Disabled, node graph debug lines will not be shown
 - 1 - Enabled, the node graph debug lines will render
 - 2 - Enabled, but without depth culling

Notes:
In the node log file under maps/graphs, nodes will contain two sets of
indexes. One of them is the original index before optimization is done,
this will be the index referenced while the graph is being built. The
second index is the one it is given after nodes are optimized out. The
one relevant to you if the first index. 
 

Impulse commands for debugging the node graph:
 - impulse 185 - Gives the nearest info_node's indexes.
 - impulse 186 to 189 - Shows the bounding boxes of the nearest info_node 
						for the small, fly, large and human hulls each
						respectively.
 - impulse 190 to 193 - Shows the bounding boxes of all info_nodes in the
						level for the small, fly, large and human hulls
						each respectively.
 - impulse 194 to 197 - Show small, fly, large and human hull paths each
						respectively for the nearest node.
 - impulse 198 - Shows all paths connected between the info_nodes in the
						entire level.

These commands will help you debug your levels when it comes to info_nodes
and how to lay them out. Generally you want the info_nodes to mark a the
path an NPC can take. They don't have to be able to see eachother to be
linked, but the path inbetween has to be a path the NPC can traverse, so
pay attention to the sizes of stair steps, and their heights. You want to
apply a 1:2 ratio on the length and the height of the stair step. You also
want to use heights of either 8 or 16, as anything else the NPC will not be
able to traverse.
As for sloped terrain, try to avoid going crazy with the slope. NPCs really
hate non-flat surfaces, but they will traverse them to some degree. This is
really a limit of the movement code.

When placing them on either side of a func_door_rotating, make sure that
the info_node will not be inside the func_door_rotating's path as it will
rotate, because this will lead to issues.
Also make sure that you take the "Lip" value of your func_door into account
when positioning the info_node, as that will also cause your info_nodes to
fail linking.

