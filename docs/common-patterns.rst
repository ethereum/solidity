###############
Common Patterns
###############

.. index:: access;restricting

******************
Restricting Access
******************

Restricting access is a common pattern for contracts.
Note that you can never restrict any human or computer
from reading the content of your transactions or
your contract's state. You can make it a bit harder
by using encryption, but if your contract is supposed
to read the data, so will everyone else.

You can restrict read access to your contract's state
by **other contracts**. That is actually the default
unless you declare make your state variables ``public``.

Furthermore, you can restrict who can make modifications
to your contract's state or call your contract's
functions and this is what this page is about.

.. index:: function;modifier

The use of **function modifiers** makes these
restrictions highly readable.

::

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

A more specialised way in which access to function
calls can be restricted will be discussed
in the next example.

.. index:: state machine

*************
State Machine
*************

Contracts often act as a state machine, which means
that they have certain **stages** in which they behave
differently or in which different functions can
be called. A function call often ends a stage
and transitions the contract into the next stage
(especially if the contract models **interaction**).
It is also common that some stages are automatically
reached at a certain point in **time**.

An example for this is a blind auction contract which
starts in the stage "accepting blinded bids", then
transitions to "revealing bids" which is ended by
"determine auction autcome".

.. index:: function;modifier

Function modifiers can be used in this situation
to model the states and guard against
incorrect usage of the contract.

Example
=======

In the following example,
the modifier ``atStage`` ensures that the function can
only be called at a certain stage.

Automatic timed transitions
are handled by the modifier ``timeTransitions``, which
should be used for all functions.

.. note::
    **Modifier Order Matters**.
    If atStage is combined
    with timedTransitions, make sure that you mention
    it after the latter, so that the new stage is
    taken into account.

Finally, the modifier ``transitionNext`` can be used
to automatically go to the next stage when the
function finishes.

.. note::
    **Modifier May be Skipped**.
    Since modifiers are applied by simply replacing
    code and not by using a function call,
    the code in the transitionNext modifier
    can be skipped if the function itself uses
    return. If you want to do that, make sure
    to call nextStage manually from those functions.

::

    contract StateMachine {
        enum Stages {
            AcceptingBlindedBids,
            RevealBids,
            AnotherStage,
            AreWeDoneYet,
            Finished
        }

        // This is the current stage.
        Stages public stage = Stages.AcceptingBlindedBids;

        uint public creationTime = now;

        modifier atStage(Stages _stage) {
            if (stage != _stage) throw;
            _
        }

        function nextStage() internal {
            stage = Stages(uint(stage) + 1);
        }

        // Perform timed transitions. Be sure to mention
        // this modifier first, otherwise the guards
        // will not take the new stage into account.
        modifier timedTransitions() {
            if (stage == Stages.AcceptingBlindedBids &&
                        now >= creationTime + 10 days)
                nextStage();
            if (stage == Stages.RevealBids &&
                    now >= creationTime + 12 days)
                nextStage();
            // The other stages transition by transaction
            _
        }

        // Order of the modifiers matters here!
        function bid()
            timedTransitions
            atStage(Stages.AcceptingBlindedBids)
        {
            // We will not implement that here
        }

        function reveal()
            timedTransitions
            atStage(Stages.RevealBids)
        {
        }

        // This modifier goes to the next stage
        // after the function is done.
        // If you use `return` in the function,
        // `nextStage` will not be called
        // automatically.
        modifier transitionNext()
        {
            _
            nextStage();
        }

        function g()
            timedTransitions
            atStage(Stages.AnotherStage)
            transitionNext
        {
            // If you want to use `return` here,
            // you have to call `nextStage()` manually.
        }

        function h()
            timedTransitions
            atStage(Stages.AreWeDoneYet)
            transitionNext
        {
        }

        function i()
            timedTransitions
            atStage(Stages.Finished)
        {
        }
    }
