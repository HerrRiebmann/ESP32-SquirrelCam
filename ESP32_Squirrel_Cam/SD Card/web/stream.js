function Toggle(){
  var stream = document.getElementById("ImgStream");
  var picture = document.getElementById("ImgStatic");
  if (stream.hidden){
	stream.style.display = "inline";
	stream.hidden = false;
	picture.style.display = "none";
	picture.hidden = true;
	document.getElementById("StreamBtn").style.display = "none";
	document.getElementById("PictureBtn").style.display = "inline";
	document.getElementById("RefreshBtn").style.display = "none";
	document.getElementById("SaveBtn").style.display = "inline";
	stream.src = "/mjpeg/1";
  }
  else {
	stream.src = "/web/favicon.ico";
	stream.style.display = "none";
	stream.hidden = true;
	picture.style.display = "inline";
	picture.hidden = false;	
	document.getElementById("StreamBtn").style.display = "inline";
	document.getElementById("PictureBtn").style.display = "none";
	document.getElementById("RefreshBtn").style.display = "inline";
	document.getElementById("SaveBtn").style.display = "inline";
	setTimeout(Refresh,500);
  }	  
} 
function Refresh() {
	var picture = document.getElementById("ImgStatic");
	picture.src = "/jpg?" + new Date().getTime();
}
var colorToReset;
function Flash() {
	var oRequest = new XMLHttpRequest();
	var sURL  = '/flash';	
	if(colorToReset == null)
		colorToReset = document.getElementById("FlashBtn").style.backgroundColor; 
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			document.getElementById("FlashBtn").style.backgroundColor = "#25bb0d";
			setTimeout(ResetControl,3000);
		}
	};
	oRequest.onerror = function (e) {
		document.getElementById("FlashBtn").style.backgroundColor = "#ea401b";
		setTimeout(ResetControl,3000);
	};
	oRequest.send(null);
}
function Save() {
	var oRequest = new XMLHttpRequest();
	var sURL  = '/photo';	
	if(colorToReset == null)
		colorToReset = document.getElementById("SaveBtn").style.backgroundColor; 
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			document.getElementById("SaveBtn").style.backgroundColor = "#25bb0d";
			setTimeout(ResetControl,3000);
			if(document.getElementById("ImgStream").hidden)
				Refresh();
		}
	};
	oRequest.onerror = function (e) {
		document.getElementById("SaveBtn").style.backgroundColor = "#ea401b";
		setTimeout(ResetControl,3000);
	};
	oRequest.send(null);
}
function ResetControl(){
	document.getElementById("FlashBtn").style.backgroundColor = colorToReset;
	document.getElementById("SaveBtn").style.backgroundColor = colorToReset;
}