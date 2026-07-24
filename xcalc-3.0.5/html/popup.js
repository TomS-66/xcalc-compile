/*
 * Hacked from an Apple example by BR Jan 08
 */

// Popup position relative to cursor
var xOffset = 15;
var yOffset = 10;

function showPopup (targetObjectId, eventObj) {
    if (eventObj) {
		// get popup width (to position within page)
		var wd = document.getElementById(targetObjectId).clientWidth;
		// hide any currently-visible popups
		hideCurrentPopup();
		// stop event from bubbling up any farther
		eventObj.cancelBubble = true;
		// move popup div to current cursor position 
		// (add scroll elements to account for scrolling for IE - doesn't support .pageX/.pageY)
		var newXCoordinate = (eventObj.pageX)?eventObj.pageX + xOffset:eventObj.x + xOffset + ((document.body.scrollLeft)?document.body.scrollLeft:0);
		var newYCoordinate = (eventObj.pageY)?eventObj.pageY + yOffset:eventObj.y + yOffset + ((document.body.scrollTop)?document.body.scrollTop:0);
		if (newXCoordinate+wd+4 > document.body.clientWidth) newXCoordinate = document.body.clientWidth-wd-4;
		moveObject(targetObjectId, newXCoordinate, newYCoordinate);
		// and make it visible
		if( changeObjectVisibility(targetObjectId, 'visible') ) {
		    // if we successfully showed the popup
		    // store its Id on a globally-accessible object
		    window.currentlyVisiblePopup = targetObjectId;
		    return true;
		} else {
		    // couldn't show the popup
		    return false;
		}
    } else {
		// there was no event object, so we won't be able to position anything, so give up
		return false;
    }
} // showPopup

function hideCurrentPopup() {
    // note: we've stored the currently-visible popup on the global object window.currentlyVisiblePopup
    if(window.currentlyVisiblePopup) {
		changeObjectVisibility(window.currentlyVisiblePopup, 'hidden');
		window.currentlyVisiblePopup = false;
    }
} // hideCurrentPopup

// setup an event handler to hide popups for generic clicks on the document
document.onclick = hideCurrentPopup;
