type Type is uint;
using {f as +} for Type global;
function f(Type, Type) pure returns (Type) {}

Type constant t = Type.wrap(1);
Type constant u = v + t;
Type constant v = u + t;
// ----
// TypeError 8349: (148-153): Initial value for constant variable has to be compile-time constant.
// TypeError 8349: (173-178): Initial value for constant variable has to be compile-time constant.
