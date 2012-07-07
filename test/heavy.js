var path = require('path');
var fs = require('fs');
var wrench = require('wrench');
var async = require('async');

var cl = require('../clucene').CLucene;
var clucene = new cl.Lucene();

var indexPath = './heavy.index';

var testJson = JSON.parse(fs.readFileSync("./test/facebook.json"));

if (fs.existsSync(indexPath)) {
    wrench.rmdirSyncRecursive(indexPath);
}

var ctr = 0;
var MAX_COUNTER = 200000;
var pauseForRam = false;

function nextTest(nextFn) {
	if (pauseForRam) {
		console.log("All done check the ram");
		setTimeout(function() { nextFn.call(); }, 10000);
	} else {
		nextFn.call();
	}
}

function searchOnePass() {
	ctr = 0;
	console.log("Doing invdividual searches");
	function runStep() {
		if (ctr > MAX_COUNTER) {
			return nextTest(deletePass);
		}
		process.nextTick(function() {
			clucene.search(indexPath, '_id:"' + ctr + '"', function(err, results, searchTime) {
				++ctr;
				runStep();
			})
		});
	}
	runStep();
}

function deletePass() {
	/*
	ctr = 0;
	console.log("Doing deletes"); 
	function runStep() {
		if (ctr > MAX_COUNTER) {
			return nextTest(function() { console.log("All done"); });
		}
		process.nextTick(function() {
			clucene.search(indexPath, 'json:"Muldowney"', function(err, results, searchTime) {
				++ctr;
				runStep();
			})
		});
	}
	runStep();
	*/
	 console.log("done"); setTimeout(function() {}, 360000); 
}


function indexPass(updates) {
	console.log("Index docs" + (updates ? " with updates" : ""));
	var doc = new cl.Document();
	ctr = 0;
	function nextStep() { ctr++; runAgain(); }
	function runAgain() {
		if (ctr > MAX_COUNTER) {
			delete doc;
			doc = null;
			clucene.closeWriter();
			//nextTest(function() {console.log("Here"); if (!updates) indexPass(true); else searchOnePass(); });
			nextTest(deletePass);
			return;
		}
		process.nextTick(function() {
			doc.clear();
			if (ctr % 1000 == 0) console.log("Adding " + ctr);
	    	testJson.newField = ctr;
	    	var jsonStr = JSON.stringify(testJson);
	    	doc.addField("json", jsonStr, cl.STORE_NO|cl.INDEX_TOKENIZED);
	    	delete jsonStr;
	    	jsonStr = null;
	    	var ctrStr = String(ctr);
	    	doc.addField("baseId", ctrStr, cl.STORE_NO|cl.INDEX_UNTOKENIZED);
	    	delete ctrStr;
	    	ctrStr = null;
	    	var docId = "id" + ctr;
	    	clucene.addDocument(docId, doc, indexPath, nextStep);
		});
	}
	runAgain();
}

process.stdin.on("data", function(data) {
	if (data == "end") process.exit(0);
	console.log(data);
});

if (process.argv.length >= 3) {
	pauseForRam = true;
}

// Our startup ram check stop
console.log("Check start size"); 
nextTest(indexPass);
