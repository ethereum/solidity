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
// TypeError 2271: (49-62): Built-in binary operator << cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (72-85): Built-in binary operator >> cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (95-107): Built-in binary operator ^ cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (117-129): Built-in binary operator | cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (139-151): Built-in binary operator & cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (162-174): Built-in binary operator * cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (184-196): Built-in binary operator / cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (206-218): Built-in binary operator % cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (228-240): Built-in binary operator - cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (250-262): Built-in binary operator + cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (272-285): Built-in binary operator ** cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (296-309): Built-in binary operator == cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (319-332): Built-in binary operator != cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (342-355): Built-in binary operator >= cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (365-378): Built-in binary operator <= cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (388-400): Built-in binary operator < cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (410-422): Built-in binary operator > cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (433-446): Built-in binary operator || cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (456-469): Built-in binary operator && cannot be applied to types type(contract super C) and contract C.
// TypeError 4247: (480-485): Expression has to be an lvalue.
// TypeError 7366: (480-493): Operator -= not compatible with types type(contract super C) and contract C.
// TypeError 4247: (503-508): Expression has to be an lvalue.
// TypeError 7366: (503-516): Operator += not compatible with types type(contract super C) and contract C.
// TypeError 9717: (534-539): Invalid mobile type in true expression.
