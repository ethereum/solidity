contract ERC20 {
	/// @notice This event is emitted when a transfer occurs.
	/// @param from The source account.
	/// @param to The destination account.
	/// @param amount The amount.
	/// @dev A test case!
	event Transfer(address indexed from, address indexed to, uint amount);
}

contract A is ERC20 {
	/// @inheritdoc ERC20
	event Transfer();
}


// ----
// DocstringParsingError 6546: (305-326): Documentation tag @inheritdoc not valid for events.
