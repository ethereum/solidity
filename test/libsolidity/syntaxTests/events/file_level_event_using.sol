event E();

type T is uint;
using {E} for T;
// ----
// TypeError 8187: (35-36): Expected function name.
