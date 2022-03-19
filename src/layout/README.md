# Layout
This is the default layout engine. It is a layout policy used to transform
the semantic text model into a format that can be displayed. Custom layouts
can be implemented (not now, in the future) by subclassing `TextLayout`
and overriding select virtual methods.

The 'layout manager' takes in a 'semantic tree' and produces a 'layout tree',
which is passed on the toolkit-specific drawing widget. It is responsible for
caching the layout tree in between redraws and minimising the recomputation
needed.

The architecture of the layout tree is loosely inspired by Matt Brubeck's
[excellent series](https://limpet.net/mbrubeck/2014/09/08/toy-layout-engine-5-boxes.html)
on building a browser engine, itself a simplification of modern web engines.
