# Paths

*See also [How do we notate paths?](How do we notate paths?.html), which in part supercedes this.*

A path is essentially a list of keywords. 

However, the path `(:aardvark :beluga :coeleocanth :dodo)` may be denoted `:aardvark:beluga:coeleocanth:dodo` and will be expanded into a list by the reader. If the value of `:ardvark` in the current environment is a map, and the value of `:beluga` in that map is a map, and the value of `:coeleocanth` in that map is a map, then the value of the path `:aardvark:beluga:coeleocanth:dodo` is whatever the value of `:dodo` is in the map indicated by `:aardvark:beluga:coeleocanth`; however if the path cannot be fully satisfied the value is `nil`. 

The notation `::aardvark:beluga:coeleocanth:dodo` is expanded by the reader into `((oblist) :aardvark :beluga :coeleocanth :dodo)`, or, in other words, is a path from the root namespace.