pragma experimental SMTChecker;
contract Simple{

    address destination;
    address source;

    function set(address source_taint) public{
        destination = source_taint;
    }

    function set2() public{
        destination = source;
    }
}

contract Reference{

    struct St{
        uint val;
    }

    St destination;
    St source;
    St destination_indirect_1;
    St destination_indirect_2;

    function set(uint source_taint) public{
        destination.val = source_taint;
    }

    function set2() public{
        destination.val = source.val;
    }

    function set3(uint source_taint) public{
        St storage ref = destination_indirect_1;
        if(true){
            ref = destination_indirect_2;
        }
        ref.val = source_taint;
    }
}

contract SolidityVar{

    address addr_1;
    address addr_2;

    constructor(){
        addr_1 = msg.sender;
    }

}

contract Intermediate{

    uint destination;
    uint source_intermediate;
    uint source;

    function f() public{
        destination = source_intermediate;
    }
    function f2() public{
        source_intermediate = source;
    }

}


contract Base{

    uint destination;
    uint source_intermediate;
    uint source;

    function f() public{
        destination = source_intermediate;
    }
}
contract Derived is Base{

    function f2() public{
        source_intermediate = source;
    }


}


contract PropagateThroughArguments {
    uint var_tainted;
    uint var_not_tainted;
    uint var_dependant;

    function f(uint user_input) public {
        f2(user_input, 4);
        var_dependant = var_tainted;
    }

    function f2(uint x, uint y) internal {
        var_tainted = x;
        var_not_tainted = y;
    }
}

contract PropagateThroughReturnValue {
  uint var_dependant;
  uint var_state;

  function foo() public {
    var_dependant = bar();
  }

  function bar() internal returns (uint) {
    return (var_state);
  }
}
// ----
// Warning 2018: (1879-1947): Function state mutability can be restricted to view
