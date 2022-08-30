contract C {
    function foo() public {
        super << this;
        super >> this;
        super ^ this;
        super | this;
        super & this;

        super * this;
        super / this;
        super % this;
        super - this;
        super + this;
        super ** this;

        super == this;
        super != this;
        super >= this;
        super <= this;
        super < this;
        super > this;

        super || this;
        super && this;

        super -= this;
        super += this;

        true ? super : this;
    }
}
// ----
// TypeError 2271: (49-62): Binary operator << not compatible with types type(contract super C) and contract C.
// TypeError 2271: (72-85): Binary operator >> not compatible with types type(contract super C) and contract C.
// TypeError 2271: (95-107): Binary operator ^ not compatible with types type(contract super C) and contract C.
// TypeError 2271: (117-129): Binary operator | not compatible with types type(contract super C) and contract C.
// TypeError 2271: (139-151): Binary operator & not compatible with types type(contract super C) and contract C.
// TypeError 2271: (162-174): Binary operator * not compatible with types type(contract super C) and contract C.
// TypeError 2271: (184-196): Binary operator / not compatible with types type(contract super C) and contract C.
// TypeError 2271: (206-218): Binary operator % not compatible with types type(contract super C) and contract C.
// TypeError 2271: (228-240): Binary operator - not compatible with types type(contract super C) and contract C.
// TypeError 2271: (250-262): Binary operator + not compatible with types type(contract super C) and contract C.
// TypeError 2271: (272-285): Binary operator ** not compatible with types type(contract super C) and contract C.
// TypeError 2271: (296-309): Binary operator == not compatible with types type(contract super C) and contract C.
// TypeError 2271: (319-332): Binary operator != not compatible with types type(contract super C) and contract C.
// TypeError 2271: (342-355): Binary operator >= not compatible with types type(contract super C) and contract C.
// TypeError 2271: (365-378): Binary operator <= not compatible with types type(contract super C) and contract C.
// TypeError 2271: (388-400): Binary operator < not compatible with types type(contract super C) and contract C.
// TypeError 2271: (410-422): Binary operator > not compatible with types type(contract super C) and contract C.
// TypeError 2271: (433-446): Binary operator || not compatible with types type(contract super C) and contract C.
// TypeError 2271: (456-469): Binary operator && not compatible with types type(contract super C) and contract C.
// TypeError 4247: (480-485): Expression has to be an lvalue.
// TypeError 7366: (480-493): Operator -= not compatible with types type(contract super C) and contract C.
// TypeError 4247: (503-508): Expression has to be an lvalue.
// TypeError 7366: (503-516): Operator += not compatible with types type(contract super C) and contract C.
// TypeError 1080: (527-546): True expression's type type(contract super C) does not match false expression's type contract C.
