var FakeHttpProvider = require('./FakeHttpProvider');

var FakeHttpProvider2 = function () {
    this.counter = 0;
    this.resultList = [];
};

FakeHttpProvider2.prototype = new FakeHttpProvider();
FakeHttpProvider2.prototype.constructor = FakeHttpProvider2;

FakeHttpProvider2.prototype.injectResultList = function (list) {
    this.resultList = list;
};

FakeHttpProvider2.prototype.getResponse = function () {
    var result = this.resultList[this.counter];
    this.counter++;

    // add fallback result value
    if(!result)
        result = {
            result: undefined
        };

    if (result.type === 'batch') {
        this.injectBatchResults(result.result);
    } else {
        this.injectResult(result.result);
    }

    this.counter = 0;

    return this.response;
};

module.exports = FakeHttpProvider2;

