A homogeneity is a [[regularity]] which has a validation funtion associated with each key. A member can only be added to a homogeneity if not only does it have all the required keys, but the value of each key in the candidate member satisfies the validation function for that key. For example, the validation function for the age of a person might be something like

```
(fn [value]
  (and (integer? value) (positive? value) (< value 140)))
```