# Regularity

A regularity is a map whose values are maps, all of whose members share the same keys. A map may be added to a regularity only if it has all the keys the regularity expects, although it may optionally have more. It is legitimate for the same map to be a member of two different regularities, if it has a union of their keys. Keys in a regularity must be keywords. Regularities are roughly the same sort of thing as classes in object oriented programming or tables in databases, but the values of the keys are not policed (see homogeneity).

A regularity may also have an association of methods, that is, functions which accept a member of the regularity as their first argument; this set of methods forms an API to the regularity. Of course a full hierarchical object oriented model can be layered on top of this, but a regularity does not in itself have any concept of class inheritance.

But, for example, if we have a regularity whose members represent companies, and those companies each have employees, then there might be a method :payroll of companies which might internally look like:

(lambda (company)
    (reduce + (map do-something-to-get-salary (:employees company))))

which would be accessed

(with ((companies . ::shared:pool:companies)
       (acme . companies:acme-widgets))
    (companies:methods:payroll acme))

But salary is not a property of a company, it's a property of an employee; so what is this thing called do-something-to-get-salary? It's a method on the regularity of employees, so in this example, it is ::shared:pool:employees:methods:salary.

There are issues that I haven't resolved yet about the mutability of regularities and homogeneities; obviously, in order to provide multi-user visibility of current values of shared data, some regularities must be mutable. But mutability has potentially very serious perfomance issues for the hypercube architecture, so I think that in general they should not be.