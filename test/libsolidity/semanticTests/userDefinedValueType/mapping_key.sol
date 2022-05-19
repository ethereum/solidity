type MyInt is int;
contract C {
    mapping(MyInt => int) public m;
    function set(MyInt key, int value) external {
        m[key] = value;
    }
    function set_unwrapped(int key, int value) external {
        m[MyInt.wrap(key)] = value;
    }
}
// ----
// set(int256,int256): 1, 1 ->
// m(int256): 1 -> 1
// set_unwrapped(int256,int256): 1, 2 ->
// m(int256): 1 -> 2
// m(int256): 2 -> 0
