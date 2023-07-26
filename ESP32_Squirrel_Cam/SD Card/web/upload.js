"use strict";
function Upload(){
	document.body.style.cursor = 'progress';
	FileChange();
	document.getElementsByClassName('dimmer')[0].style.display = 'block';
	document.getElementById("UploadForm").submit();
}
function FileChange(){
    var fileList = document.getElementById("uploadFile").files;
    var file = fileList[0];
    if(!file)
        return; 
    document.getElementById("fileName").innerHTML = 'Name: ' + file.name;
    document.getElementById("fileSize").innerHTML = 'Size: ' + humanFileSize(file.size);
    document.getElementById("fileType").innerHTML = 'Type: ' + file.type;      
}
function humanFileSize(size) {
    var i = Math.floor( Math.log(size) / Math.log(1024) );
    return ( size / Math.pow(1024, i) ).toFixed(2) * 1 + ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i];
}