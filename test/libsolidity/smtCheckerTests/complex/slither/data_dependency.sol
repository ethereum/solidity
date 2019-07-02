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

    constructor() public{
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
// Warning: (1886-1954): Function state mutability can be restricted to view
// Warning: (318-332): Assertion checker does not yet support the type of this variable.
// Warning: (338-347): Assertion checker does not yet support the type of this variable.
// Warning: (353-378): Assertion checker does not yet support the type of this variable.
// Warning: (384-409): Assertion checker does not yet support the type of this variable.
// Warning: (464-479): Assertion checker does not yet support this expression.
// Warning: (464-475): Assertion checker does not yet implement type struct Reference.St storage ref
// Warning: (464-494): Assertion checker does not yet implement such assignments.
// Warning: (539-554): Assertion checker does not yet support this expression.
// Warning: (539-550): Assertion checker does not yet implement type struct Reference.St storage ref
// Warning: (557-567): Assertion checker does not yet support this expression.
// Warning: (557-563): Assertion checker does not yet implement type struct Reference.St storage ref
// Warning: (539-567): Assertion checker does not yet implement such assignments.
// Warning: (629-643): Assertion checker does not yet support the type of this variable.
// Warning: (646-668): Assertion checker does not yet implement type struct Reference.St storage ref
// Warning: (706-728): Assertion checker does not yet implement type struct Reference.St storage ref
// Warning: (700-728): Assertion checker does not yet implement type struct Reference.St storage pointer
// Warning: (748-755): Assertion checker does not yet support this expression.
// Warning: (748-751): Assertion checker does not yet implement type struct Reference.St storage pointer
// Warning: (748-770): Assertion checker does not yet implement such assignments.
// Warning: (849-905): Assertion checker does not yet support constructors.
