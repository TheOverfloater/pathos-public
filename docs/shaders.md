# The GLSL shader format

Pathos uses GLSL for rendering, and these are defined in BSS
scripts located under "moddir/scripts/shaders". Pathos uses the
method of cutting shader scripts up into several permutations,
and then switching between them based on which oen is needed. It
will then compile these on loading, and generate both a cut-up
version in CSD files, and the raw binary data returned by OpenGL
in BSD files.

The basic shader format is rather simple. How the scripts are cut
up are defined by what are called determinators. These specify the
types of shader variations.

Here is an example of a determinator:
>$global_determinator texture 0 1 <br />

Global determinators affect both the vertex and the fragment shader,
and are placed at the beginning of the file.

The definition of each individual shader is defined by the tokens
below:
>$shader_vertex <br />
>$shader_fragment <br />

These are closed with a $begin and $end token. If you have determinators
that affect only one of the shaders, they are defined as such:
>$shader_fragment <br />
>$determinator rectangle 0 1 <br />
>$begin <br />

# Invalid states:
This option allows you to cut down on shader variations if there's specific
combinations of shader determinators that will not be used. You can define
the ranges of values that the compiler will skip. 

For example in bsprenderer.bss:
>$invalid_state 2 fogtype 1 2 shadertype 5 19 <br />

Here, fog is not going to be used with specific shader variations, so it is
not compiled by the compiler code. The first parameter is the number of
determinators to check, and then the determinator name, followed by the
range of values deemed to be invalid. Then the next determinator follows
with the same schema.

Here is the basic breakdown:
>$invalid_state *nb of determinators* *determinator 1 name* *range min* *range max* *determinator 2 name* *range min* *range max* <br />

# Branching:
Branching itself is defined with a $branch token, followed by $begin and
closed by an $end token. Here is an example in basicdraw.bss:

>$branch texture == 1 <br />
>$begin <br />
>	$branch rectangle == 0 <br />
>	$begin <br />
>		finalColor = texture(texture0, ps_texcoord); <br />
>	$end <br />
>	$branch rectangle == 1 <br />
>	$begin <br />
>		finalColor = texture(rectangle0, ps_texcoord); <br />
>	$end <br />
>$end <br />

There's no $else directive added, so you need to define each possible value.
Using C-like combinations of value checks is also supported, like here:

>$branch shadertype == 0 || shadertype == 17 || shadertype == 20 || shadertype == 21 <br />
>$begin <br />
>	ps_dtexcoord = in_dtexcoord; <br />
>$end <br />

I recommend keeping shader variations to a minimum, although with recent
advancements in Pathos, having lots of variations is managed better with some
optimizations.
If you are loading Pathos for the first time, it's likely that the engine will
recompile binary shader data for the game on the first launch, but subsequent
launches will be much faster. This might happen again if you've updated or
reinstalled your drivers, like if you changed your GPU.
