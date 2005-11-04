// Globals
// you can change this to whatever style you want for each outline item
var outlineStart = "";
var outlineEnd = "";

// This holds the outline list
var theOutline = null;

// **functions that establish objects and environment**
// basic array maker
function makeArray(n) {
  this.length = n;
    return this;
}

// object constructor for each outline entry
// (see object-building calls, below)
function OutlineNode(parent,imageURL,display,URL,targetframe,indent){
  this.parent = parent;   // is this item a parent?
  this.imageURL= imageURL; // image to display
  this.display = display; // text to display
  this.URL = URL;         // link tied to text; if empty string, item appears as straight text
  this.targetframe = targetframe;  // target frame
  this.indent = indent;   // how deeply nested?
  return this;
}

// The all important cookie that holds the curent state
var mycookie = "";
//var mycookie = document.cookie;
//alert(mycookie);

// ** functions that get and set persistent cookie data **
// set cookie data
function setCurrState(setting) {
  SetCookie('currState', setting);
  mycookie = setting;
  //mycookie = document.cookie = "currState=" + escape(setting);
}



// retrieve cookie data
function getCurrState() {
  if (mycookie == '')
  {
    mycookie=GetCookie('currState');
    //alert('just set mycookie from currState='+mycookie);
    if (mycookie == null || mycookie == "" || mycookie.length != theOutline.length) {
      //alert('will do init');
      initializeState2();
      //alert('had to init mycookie='+mycookie);
    }
  }
  return mycookie;
}

function getCurrStateold() {
    var label = "currState=";
    var labelLen = label.length;
    var cLen = mycookie.length;
    var i = 0;
    while (i < cLen) {
      var j = i + labelLen;
	if (mycookie.substring(i,j) == label) {
	  var cEnd = mycookie.indexOf(";",j);
	    if (cEnd ==     -1) {
	      cEnd = mycookie.length;
	    }
	    return unescape(mycookie.substring(j,cEnd));
	}
	i++;
    }
    return "";
}

function getCookieVal (offset) {
 var endstr = document.cookie.indexOf (";", offset);
 if (endstr == -1)
  endstr = document.cookie.length;
 return unescape(document.cookie.substring(offset, endstr));
}
function GetCookie (name) {
 var arg = name + "=";
 var alen = arg.length;
 var clen = document.cookie.length;
 var i = 0;
 while (i < clen) {
  var j = i + alen;
  if (document.cookie.substring(i, j) == arg)
   return getCookieVal (j);
  i = document.cookie.indexOf(" ", i) + 1;
  if (i == 0)
   break;
 }
 return null;
}

function SetCookie (name, value) {
var argv = SetCookie.arguments;
var argc = SetCookie.arguments.length;
var expires = (argc > 2) ? argv[2] : null;
var path = (argc > 3) ? argv[3] : null;
var domain = (argc > 4) ? argv[4] : null;
var secure = (argc > 5) ? argv[5] : false;
document.cookie = name + "=" + escape (value) +
((expires == null) ? "" : ("; expires=" +
expires.toGMTString())) +
((path == null) ? "" : ("; path=" + path)) +
((domain == null) ? "" : ("; domain=" + domain)) +
((secure == true) ? "; secure" : "");
}

// **function that updates persistent storage of state**
// toggles an outline parent entry, storing new value in the cookie
function toggle(n) {
  if (n != 0) {
    var newString = "";
      var currState = getCurrState(); // of whole outline
      var expanded = currState.substring(n-1,n); // of clicked item
      newString += currState.substring(0,n-1);
      newString += expanded ^ 1; // Bitwise XOR clicked item
      newString += currState.substring(n,currState.length);
      setCurrState(newString); // write new state back to cookie
  }
}

// **functions used in assembling updated outline**
// returns the proper GIF file name for each entry's control
function getGIF(n) {
  var myparent = theOutline[n].parent;  // is entry a parent?
    var expanded = getCurrState().substring(n-1,n); // of clicked item
    if (!myparent) {
      return imgroot + "/item.gif";
    } 
    else {
      if (expanded == 1) {
	return imgroot + "/menu2.gif";
      }
    }
    return imgroot + "/menu.gif";
}

// returns the proper status line text based on the icon style
function getGIFStatus(n) {
  var myparent = theOutline[n].parent;  // is entry a parent
    var expanded = getCurrState().substring(n-1,n); // of rolled item
    if (!myparent) {
      return "";
    } 
    else {
      if (expanded == 1) {
	return "Click to close chapter or section";
      }
    }
    return "Click to open chapter or section";
}

// returns padded spaces (in multiples of 10px) for indenting
function pad(n) {
	var padw=n*10;
    if (n==0) return "";
    return "<TD NOWRAP><IMG SRC=\""+ imgroot + "/1x1.gif\" ALT=\"\" HEIGHT=\"1\" WIDTH=\""+padw+"\" /></TD>";
}

// see if user is running a 2.01 Navigator with the reentrant window bug
function isReentrantBuggy() {
  return (navigator.userAgent.indexOf("2.01") >= 0 && navigator.userAgent.indexOf("(Win") == -1) ? true :false
}

function initializeState() {
  // initialize 'current state' storage field
  getCurrState();
}
function initializeState2() {
    initState = ""
    for (i = 1; i <= theOutline.length; i++) {
	initState += "0";
    }
    setCurrState(initState);
}

var prevIndentDisplayed = 0;
var showMyChild = 0;

var newOutline = "" + outlineStart;  // let padded spaces make indents

// Do it!
function doOutline() {
  var isbuggy = isReentrantBuggy();
  // cycle through each entry in the outline array
  for (var i = 1; i < theOutline.length; i++) {
      var theGIF = getGIF(i);				// get the image
      var theGIFStatus = getGIFStatus(i);  // get the status message
      var currIndent = theOutline[i].indent;		// get the indent level
      var expanded = getCurrState().substring(i-1,i); // current state
      // display entry only if it meets one of three criteria
      if (currIndent == 0 || currIndent <= prevIndentDisplayed || (showMyChild == 1 && (currIndent - prevIndentDisplayed == 1))) {
	newOutline += "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=\"100%\"><TR>";
	newOutline += pad(currIndent);
	newOutline += "<TD WIDTH=\"14\" VALIGN=\"top\">";
	  var activeWidget = (theGIF == imgroot + "item.gif") ? false : true;
	  if (isbuggy) {
	    newOutline += "<A CLASS=\"cf-toc1\" HREF=# onMouseOver=\"window.parent.status=\'" + theGIFStatus + "\';return true;\" onClick=\"toggle(" + i + ");var timeoutID = setTimeout('history.go(0)',300);return " + activeWidget + "\"><IMG SRC=\"" + theGIF + "\" BORDER=0></A></TD>";
	  } 
	  else {
	    newOutline += "<A CLASS=\"cf-toc1\" HREF=\"javascript:history.go(0)\" onMouseOver=\"window.parent.status=\'" + theGIFStatus + "\';return true;\" onClick=\"toggle(" + i + ");return " + activeWidget + "\"><IMG SRC=\"" + theGIF + "\" BORDER=0></A></TD>";
	  }

	  if (theOutline[i].URL == "" || theOutline[i].URL == null) {
	    if (theOutline[i].imageURL == "" || theOutline[i].imageURL  == null) {
	      newOutline += "<TD COLSPAN=2 WIDTH=\"100%\">" + theOutline[i].display + "</TD>";	// no link, just a listed item	
	    } 
	    else {
	      newOutline += "<TD><img src='"  + theOutline[i].imageURL + "' border='0'></TD><TD>" + theOutline[i].display + "</TD>";
	    }
	  } 
	  else {
	    if (theOutline[i].imageURL == "" || theOutline[i].imageURL  == null) {
	      newOutline += "<TD COLSPAN=2 WIDTH=\"100%\"><A CLASS=\"cf-toc" + (currIndent +1) + "\" HREF=\"" + theOutline[i].URL + "\" TARGET='"  + theOutline[i].targetframe + "' onMouseOver=\"window.parent.status=\'View " + theOutline[i].display + "...\';return true;\">" + theOutline[i].display + "</A></TD>";
	    } 
	    else {
	      newOutline += "<TD><img src='"  + theOutline[i].imageURL + "' border='0'></TD><TD WIDTH=\"100%\"><A CLASS=\"cf-toc" + (currIndent +1) + "\" HREF=\"" + theOutline[i].URL + "\" TARGET='"  + theOutline[i].targetframe + "' onMouseOver=\"window.parent.status=\'View " + theOutline[i].display + "...\';return true;\">" + theOutline[i].display + "</A></TD>";
	    }
	  }

	  newOutline += "</TR></TABLE>"; 
	  prevIndentDisplayed = currIndent;
	  showMyChild = expanded;
	  if (newOutline.length > 2000) {
	      document.writeln(newOutline);
	      newOutline = "";
	  }
      }
  }
  newOutline += outlineEnd + "";
  document.writeln(newOutline); 
  document.close();
}

// create object containing outline content and attributes
// To adapt outline for your use, modify this table.  Make sure
// that the size of the array (theOutline[i]) is reflected in the call
// to makeArray(i).  See the OutlineNode() function, above, for the
// meaning of the parameters in each array entry.

// OutlineNode(isparent,imageURL,display,URL,targetframe,indent)

