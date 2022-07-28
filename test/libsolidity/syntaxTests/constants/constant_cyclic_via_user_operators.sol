type Type is uint;
using {f as +} for Type;
function f(Type, Type) pure returns (Type) {}

Type constant t = Type.wrap(1);
Type constant u = v + t;
Type constant v = u + t;
// ----
// TypeError 8349: (141-146): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (166-171): Initial value for constant variable has to be compile-time constant.
