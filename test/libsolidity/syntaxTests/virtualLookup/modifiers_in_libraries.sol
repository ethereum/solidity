library WithModifier {
    modifier mod() { require(msg.value > 10 ether); _; }
    function withMod(uint self) mod() internal view { require(self > 0); }
}

contract Test {
    using WithModifier for *;

    function f(uint _value) public payable {
        _value.withMod();
        WithModifier.withMod(_value);
    }
}
// ----
