type AP is address payable;

function sub(AP, AP) pure returns (AP) {}
function unsub(AP) pure returns (AP) {}

function attachedPure(AP, uint, address) pure {}
function attachedView(AP) view {}
function attached(AP, function(AP)) {}

using {sub as -, attachedPure, attachedView, unsub as -, attached} for AP global;
