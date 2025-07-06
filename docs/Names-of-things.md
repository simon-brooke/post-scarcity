* **assoc list** An assoc list is any list all of whose elements are cons-cells.
* **association** Anything which associates names with values. An *assoc list* is an association, but so it a *map*, a *namespace*, a *regularity* and a *homogeneity*.
* **homogeneity** A [[homogeneity]] is a *regularity* which has a validation funtion associated with each key.
* **keyword** A [[keyword]] is a token whose denotation starts with a colon and which has a limited range of allowed characters, not including punctuation or spaces, which evaluates to itself irrespective of the current binding environment.
* **map** A map in the sense of a Clojure map; immutable, adding a name/value results in a new map being created. A map may be treated as a function on *keywords*, exactly as in Clojure.
* **namespace** A namespace is a mutable map. Generally, if a namespace is shared, there will be a path from the oblist to that namespace.
* **oblist** The oblist is a privileged namespace which forms the root of all canonical paths. It is accessed at present by the function `(oblist)`, but it can be denoted in paths by the empty keyword.
* **path** A [[path]] is a list of keywords, with special notation and semantics. 
* **regularity** A [[regularity]] is a map whose values are maps, all of whose members share the same keys. A map may be added to a regularity only if it has all the keys the regularity expects, although it may optionally have more. It is legitimate for the same map to be a member of two different regularities, if it has a union of their keys. Keys in a regularity must be keywords. Regularities are roughly the same sort of thing as objects in object oriented programming or tables in databases, but the values of the keys are not policed (see `homogeneity`).
 