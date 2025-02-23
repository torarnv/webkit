/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

(function () {

module("iu.results");

var kExampleResultsByTest = {
    "scrollbars/custom-scrollbar-with-incomplete-style.html": {
        "Mock Builder": {
            "expected": "IMAGE",
            "actual": "CRASH"
        },
        "Mock Linux": {
            "expected": "TEXT",
            "actual": "CRASH"
        }
    },
    "userscripts/another-test.html": {
        "Mock Builder": {
            "expected": "PASS",
            "actual": "TEXT"
        }
    }
};

var kExampleReftestResults = {
    "reftest.html": {
        "Mock Builder": {
            "expected": "PASS",
            "actual": "IMAGE",
            "is_reftest": true
        }
    },
    "mismatch-reftest.html": {
        "Mock Builder": {
            "expected": "PASS",
            "actual": "IMAGE",
            "is_mismatch_reftest": true
        }
    }
};

var kExampleResultsWithTimeoutByTest = {
    "fast/not-fast-test.html": {
        "Mock Builder": {
            "expected": "PASS",
            "actual": "TIMEOUT"
        }
    }
};

test('View', 16, function() {
    var delegate = {
        fetchResultsURLs: function(failureInfo, callback) { return; }
    };

    var view = new ui.results.View(delegate);
    view.setResultsByTest(kExampleResultsByTest);

    view.firstResult();
    var testSelector = view.querySelector('.test-selector');
    var topPanel = testSelector.querySelector('.top-panel');
    equals(topPanel.childNodes[0], topPanel.querySelector('.active'));;
    equals($($('.builder-selector', view)[0]).tabs('option', 'selected'), 0);

    view.nextResult();
    equals(topPanel.childNodes[0], topPanel.querySelector('.active'));;
    equals($($('.builder-selector', view)[0]).tabs('option', 'selected'), 1);
    equals(view.currentTestName(), 'scrollbars/custom-scrollbar-with-incomplete-style.html');

    view.nextResult();
    equals(topPanel.childNodes[1], topPanel.querySelector('.active'));;
    equals($($('.builder-selector', view)[0]).tabs('option', 'selected'), 0);
    equals(view.currentTestName(), 'userscripts/another-test.html');

    view.previousResult();
    equals(topPanel.childNodes[0], topPanel.querySelector('.active'));;
    equals($($('.builder-selector', view)[0]).tabs('option', 'selected'), 1);

    view.nextTest();
    equals(topPanel.childNodes[1], topPanel.querySelector('.active'));;
    equals($($('.builder-selector', view)[0]).tabs('option', 'selected'), 0);
    equals(view.currentTestName(), 'userscripts/another-test.html');

    view.previousTest();
    equals(topPanel.childNodes[0], topPanel.querySelector('.active'));;
    equals($($('.builder-selector', view)[0]).tabs('option', 'selected'), 1);
    equals(view.currentTestName(), 'scrollbars/custom-scrollbar-with-incomplete-style.html');
});

test('View', 2, function() {
    var delegate = {
        fetchResultsURLs: function(failureInfo, callback) { return; }
    };

    var view = new ui.results.View(delegate);
    view.setResultsByTest(kExampleReftestResults);
    view.firstResult();

    equals($('.non-action-button', view).length, 1);
    equals($('.action', view).length, 0);
});

test('View of timeouts', 1, function() {
    var delegate = {
        fetchResultsURLs: function(failureInfo, callback) { callback([]); }
    };

    var view = new ui.results.View(delegate);
    view.setResultsByTest(kExampleResultsWithTimeoutByTest);
    view.firstResult();

    equals($('.results-grid', view).html(), 'No results to display.');
});

})();
