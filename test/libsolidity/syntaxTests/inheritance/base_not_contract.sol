function fun() {}

contract C is fun {}
// ----
// TypeError 8758: (33-36): Contract expected.
