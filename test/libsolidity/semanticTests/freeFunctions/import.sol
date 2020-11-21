==== Source: A ====
struct S { uint x; }
function set(S storage a, uint v) { a.x = v; }

==== Source: B ====
import "A";
import "A" as A;
contract C {
  A.S data;
  function f(uint v) public returns (uint one, uint two) {
    A.set(data, v);
    one = data.x;
    set(data, v + 1);
    two = data.x;
  }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256): 7 -> 7, 8
