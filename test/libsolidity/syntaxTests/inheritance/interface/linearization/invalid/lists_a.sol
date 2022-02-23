interface ParentA {}
interface ParentB {}
interface Sub is ParentA, ParentB {}

contract ListsA is Sub, ParentA {}

// ----
// TypeError 5005: (80-114): Linearization of inheritance graph impossible
