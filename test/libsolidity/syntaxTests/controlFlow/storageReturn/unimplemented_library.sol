// This used to work pre-0.6.0.
library L {
    function f() public returns(uint[] storage);
    function g() public returns(uint[] storage s);
}
abstract library T {
    function f() public returns(uint[] storage);
    function g() public returns(uint[] storage s);
}
// ----
// TypeError: (146-268): Libraries cannot be abstract.
// TypeError: (48-92): Library functions must be implemented if declared.
// TypeError: (97-143): Library functions must be implemented if declared.
// TypeError: (171-215): Library functions must be implemented if declared.
// TypeError: (220-266): Library functions must be implemented if declared.
