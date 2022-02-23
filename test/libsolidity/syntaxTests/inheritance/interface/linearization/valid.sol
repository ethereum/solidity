interface ParentA {}
interface ParentB {}
interface Sub is ParentA, ParentB {}

contract ListsA is ParentA, Sub {}
contract ListsB is ParentB, Sub {}
contract ListsBoth is ParentA, ParentB, Sub {}
// ----
