interface ParentA {}
interface ParentB {}
interface Sub is ParentA, ParentB {}

contract ListsBoth is Sub, ParentA, ParentB {}

// ----
// TypeError 5005: (80-126): Linearization of inheritance graph impossible
