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
// TypeError 2271: (49-62='super << this'): Operator << not compatible with types type(contract super C) and contract C
// TypeError 2271: (72-85='super >> this'): Operator >> not compatible with types type(contract super C) and contract C
// TypeError 2271: (95-107='super ^ this'): Operator ^ not compatible with types type(contract super C) and contract C
// TypeError 2271: (117-129='super | this'): Operator | not compatible with types type(contract super C) and contract C
// TypeError 2271: (139-151='super & this'): Operator & not compatible with types type(contract super C) and contract C
// TypeError 2271: (162-174='super * this'): Operator * not compatible with types type(contract super C) and contract C
// TypeError 2271: (184-196='super / this'): Operator / not compatible with types type(contract super C) and contract C
// TypeError 2271: (206-218='super % this'): Operator % not compatible with types type(contract super C) and contract C
// TypeError 2271: (228-240='super - this'): Operator - not compatible with types type(contract super C) and contract C
// TypeError 2271: (250-262='super + this'): Operator + not compatible with types type(contract super C) and contract C
// TypeError 2271: (272-285='super ** this'): Operator ** not compatible with types type(contract super C) and contract C
// TypeError 2271: (296-309='super == this'): Operator == not compatible with types type(contract super C) and contract C
// TypeError 2271: (319-332='super != this'): Operator != not compatible with types type(contract super C) and contract C
// TypeError 2271: (342-355='super >= this'): Operator >= not compatible with types type(contract super C) and contract C
// TypeError 2271: (365-378='super <= this'): Operator <= not compatible with types type(contract super C) and contract C
// TypeError 2271: (388-400='super < this'): Operator < not compatible with types type(contract super C) and contract C
// TypeError 2271: (410-422='super > this'): Operator > not compatible with types type(contract super C) and contract C
// TypeError 2271: (433-446='super || this'): Operator || not compatible with types type(contract super C) and contract C
// TypeError 2271: (456-469='super && this'): Operator && not compatible with types type(contract super C) and contract C
// TypeError 4247: (480-485='super'): Expression has to be an lvalue.
// TypeError 7366: (480-493='super -= this'): Operator -= not compatible with types type(contract super C) and contract C
// TypeError 4247: (503-508='super'): Expression has to be an lvalue.
// TypeError 7366: (503-516='super += this'): Operator += not compatible with types type(contract super C) and contract C
// TypeError 1080: (527-546='true ? super : this'): True expression's type type(contract super C) does not match false expression's type contract C.
