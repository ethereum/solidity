contract C {
    function f0() public { (()) = 2; }

    function f1() public pure { (()) = (); }

    //#8711
    function f2() internal pure returns (uint, uint) { return () = f2(); }

    //#8277
    function f3()public{return()=();}

    //#8277
    function f4 ( bytes32 hash , uint8 v , bytes32 r , bytes32 s , uint blockExpired , bytes32 salt ) public returns ( address ) {
        require ( ( ( ) ) |= keccak256 ( abi . encodePacked ( blockExpired , salt ) ) ) ;
        return ecrecover ( hash , v , r , s ) ;
    }
}
// ----
// TypeError 5547: (41-43): Empty tuple on the left hand side.
// TypeError 7407: (47-48): Type int_const 2 is not implicitly convertible to expected type tuple().
// TypeError 5547: (86-88): Empty tuple on the left hand side.
// TypeError 5547: (173-175): Empty tuple on the left hand side.
// TypeError 7407: (178-182): Type tuple(uint256,uint256) is not implicitly convertible to expected type tuple().
// TypeError 5132: (166-182): Different number of arguments in return statement than in returns declaration.
// TypeError 5547: (229-231): Empty tuple on the left hand side.
// TypeError 5547: (401-404): Empty tuple on the left hand side.
// TypeError 4289: (399-466): Compound assignment is not allowed for tuple types.
// TypeError 7407: (410-466): Type bytes32 is not implicitly convertible to expected type tuple().
// TypeError 9322: (389-396): No matching declaration found after argument-dependent lookup.
