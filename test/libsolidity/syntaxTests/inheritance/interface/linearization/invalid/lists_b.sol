interface ParentA {}
interface ParentB {}
interface Sub is ParentA, ParentB {}

contract ListsB is Sub, ParentB {}

// ----
// TypeError 5005: (80-114): Linearization of inheritance graph impossible
