pragma solidity >=0.5.0;
pragma experimental SMTChecker;

/**
 * @title SafeMath
 * @dev Math operations with safety checks that revert on error
 */
library SafeMath {

  /**
  * @dev Multiplies two numbers, reverts on overflow.
  */
  function mul(uint256 a, uint256 b) internal pure returns (uint256) {
    // Gas optimization: this is cheaper than requiring 'a' not being zero, but the
    // benefit is lost if 'b' is also tested.
    // See: https://github.com/OpenZeppelin/openzeppelin-solidity/pull/522
    if (a == 0) {
      return 0;
    }

    uint256 c = a * b;
    require(c / a == b);

    return c;
  }

  /**
  * @dev Integer division of two numbers truncating the quotient, reverts on division by zero.
  */
  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b > 0); // Solidity only automatically asserts when dividing by 0
    uint256 c = a / b;
    // assert(a == b * c + a % b); // There is no case in which this doesn't hold

    return c;
  }

  /**
  * @dev Subtracts two numbers, reverts on overflow (i.e. if subtrahend is greater than minuend).
  */
  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b <= a);
    uint256 c = a - b;

    return c;
  }

  /**
  * @dev Adds two numbers, reverts on overflow.
  */
  function add(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 c = a + b;
    require(c >= a);

    return c;
  }

  /**
  * @dev Divides two numbers and returns the remainder (unsigned integer modulo),
  * reverts when dividing by zero.
  */
  function mod(uint256 a, uint256 b) internal pure returns (uint256) {
    require(b != 0);
    return a % b;
  }
}

interface IERC20 {
  function totalSupply() external view returns (uint256);

  function balanceOf(address who) external view returns (uint256);

  function transfer(address to, uint256 value) external returns (bool);

  event Transfer(
    address indexed from,
    address indexed to,
    uint256 value
  );

  event Approval(
    address indexed owner,
    address indexed spender,
    uint256 value
  );
}

contract ERC20 is IERC20 {
  using SafeMath for uint256;

  mapping (address => uint256) private _balances;

  uint256 private _totalSupply;

  function totalSupply() public view returns (uint256) {
    return _totalSupply;
  }

  function balanceOf(address owner) public view returns (uint256) {
    return _balances[owner];
  }

  function transfer(address to, uint256 value) public returns (bool) {
    _transfer(msg.sender, to, value);
    return true;
  }

  function _transfer(address from, address to, uint256 value) internal {
    require(value <= _balances[from]);
    require(to != address(0));

    _balances[from] = _balances[from].sub(value);
    _balances[to] = _balances[to].add(value);
    emit Transfer(from, to, value);
  }

  function _mint(address account, uint256 value) internal {
    require(account != address(0));
    _totalSupply = _totalSupply.add(value);
    _balances[account] = _balances[account].add(value);
    emit Transfer(address(0), account, value);
  }

  function _burn(address account, uint256 value) internal {
    require(account != address(0));
    require(value <= _balances[account]);

    _totalSupply = _totalSupply.sub(value);
    _balances[account] = _balances[account].sub(value);
    emit Transfer(account, address(0), value);
  }

  function _burnFrom(address account, uint256 value) internal {
    _burn(account, value);
  }
}
// ----
// Warning: (567-572): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (555-572): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (586-591): Division by zero happens here
// Warning: (586-596): Condition is always true.
// Warning: (1389-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1377-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1408-1414): Condition is always true.
// Warning: (1668-1673): Assertion checker does not yet implement this operator.
// Warning: (2649-2664): Internal error: Expression undefined for SMT solver.
// Warning: (2685-2695): Assertion checker does not yet implement this expression.
// Warning: (2685-2695): Internal error: Expression undefined for SMT solver.
// Warning: (2721-2736): Internal error: Expression undefined for SMT solver.
// Warning: (1216-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1204-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (2703-2747): Assertion checker does not yet implement such assignments.
// Warning: (2769-2782): Internal error: Expression undefined for SMT solver.
// Warning: (1389-1394): Underflow (resulting value less than 0) happens here
// Warning: (1389-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1377-1394): Underflow (resulting value less than 0) happens here
// Warning: (1377-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (2753-2793): Assertion checker does not yet implement such assignments.
// Warning: (2804-2829): Assertion checker does not yet implement this type of function call.
// Warning: (2649-2664): Internal error: Expression undefined for SMT solver.
// Warning: (2685-2695): Assertion checker does not yet implement this expression.
// Warning: (2685-2695): Internal error: Expression undefined for SMT solver.
// Warning: (2721-2736): Internal error: Expression undefined for SMT solver.
// Warning: (1216-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1204-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (2703-2747): Assertion checker does not yet implement such assignments.
// Warning: (2769-2782): Internal error: Expression undefined for SMT solver.
// Warning: (1389-1394): Underflow (resulting value less than 0) happens here
// Warning: (1389-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1377-1394): Underflow (resulting value less than 0) happens here
// Warning: (1377-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (2753-2793): Assertion checker does not yet implement such assignments.
// Warning: (2804-2829): Assertion checker does not yet implement this type of function call.
// Warning: (2919-2929): Assertion checker does not yet implement this expression.
// Warning: (2919-2929): Internal error: Expression undefined for SMT solver.
// Warning: (1389-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1377-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (2936-2974): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (3001-3019): Internal error: Expression undefined for SMT solver.
// Warning: (1389-1394): Underflow (resulting value less than 0) happens here
// Warning: (1389-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1377-1394): Underflow (resulting value less than 0) happens here
// Warning: (1377-1394): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (2980-3030): Assertion checker does not yet implement such assignments.
// Warning: (3050-3060): Assertion checker does not yet implement this expression.
// Warning: (3041-3077): Assertion checker does not yet implement this type of function call.
// Warning: (3167-3177): Assertion checker does not yet implement this expression.
// Warning: (3167-3177): Internal error: Expression undefined for SMT solver.
// Warning: (3201-3219): Internal error: Expression undefined for SMT solver.
// Warning: (3292-3310): Internal error: Expression undefined for SMT solver.
// Warning: (1216-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1204-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (3271-3321): Assertion checker does not yet implement such assignments.
// Warning: (3350-3360): Assertion checker does not yet implement this expression.
// Warning: (3332-3368): Assertion checker does not yet implement this type of function call.
// Warning: (3167-3177): Assertion checker does not yet implement this expression.
// Warning: (3167-3177): Internal error: Expression undefined for SMT solver.
// Warning: (3201-3219): Internal error: Expression undefined for SMT solver.
// Warning: (3292-3310): Internal error: Expression undefined for SMT solver.
// Warning: (1216-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (1204-1221): Overflow (resulting value larger than 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) happens here
// Warning: (3271-3321): Assertion checker does not yet implement such assignments.
// Warning: (3350-3360): Assertion checker does not yet implement this expression.
// Warning: (3332-3368): Assertion checker does not yet implement this type of function call.
