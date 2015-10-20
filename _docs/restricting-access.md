---
layout: docs
title: Restrict Access
permalink: /docs/restricting-access/
---

Restricting access is a common pattern for contracts.
Note that you can never restrict any human or computer
from reading the content of your transactions or
your contract's state. You can make it a bit harder
by using encryption, but if your contract is supposed
to read the data, so will everyone else.

You can restrict read access to your contract's state
by **other contracts**. That is actually the default
unless you declare make your state variables `public`.

Furthermore, you can restrict who can make modifications
to your contract's state or call your contract's
functions and this is what this page is about.

The use of **function modifiers** makes these
restrictions highly readable.

{% include open_link gist="fe4ef267cbdeac151b98" %}
{% highlight javascript %}
contract AccessRestriction {
    // These will be assigned at the construction
    // phase, where `msg.sender` is the account
    // creating this contract.
    address public owner = msg.sender;
    uint public creationTime = now;

    // Modifiers can be used to change
    // the body of a function.
    // If this modifier is used, it will
    // prepend a check that only passes
    // if the function is called from
    // a certain address.
    modifier onlyBy(address _account)
    {
        if (msg.sender != _account)
            throw;
        // Do not forget the "_"! It will
        // be replaced by the actual function
        // body when the modifier is invoked.
        _
    }

    /// Make `_newOwner` the new owner of this
    /// contract.
    function changeOwner(address _newOwner)
        onlyBy(owner)
    {
        owner = _newOwner;
    }

    modifier onlyAfter(uint _time) {
        if (now < _time) throw;
        _
    }

    /// Erase ownership information.
    /// May only be called 6 weeks after
    /// the contract has been created.
    function disown()
        onlyBy(owner)
        onlyAfter(creationTime + 6 weeks)
    {
        delete owner;
    }

    // This modifier requires a certain
    // fee being associated with a function call.
    // If the caller sent too much, he or she is
    // refunded, but only after the function body.
    // This is dangerous, because if the function
    // uses `return` explicitly, this will not be
    // done!
    modifier costs(uint _amount) {
        if (msg.value < _amount)
            throw;
        _
        if (msg.value > _amount)
            msg.sender.send(_amount - msg.value);
    }

    function forceOwnerChange(address _newOwner)
        costs(200 ether)
    {
        owner = _newOwner;
        // just some example condition
        if (uint(owner) & 0 == 1)
            // in this case, overpaid fees will not
            // be refunded
            return;
        // otherwise, refund overpaid fees
    }
}
{% endhighlight %}

A more specialised way in which access to function
calls can be restricted will be discussed
in the next example.
