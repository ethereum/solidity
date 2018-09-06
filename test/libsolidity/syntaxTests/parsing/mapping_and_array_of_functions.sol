contract test {
    mapping (address => function() internal returns (uint)) a;
    mapping (address => function() external) b;
    mapping (address => function() external[]) c;
    function() external[] d;
}
